#include "heap.h"
#include "image.h"
#include "read-memory-stream.h"
#include "context-internal.h"
#include <assert.h>
#include <string.h>

#include <sys/mman.h>

static inline size_t
crankvm_heap_roundUpHeapSize(size_t size)
{
    return ((size + 4095) & (-4096));
}

static crankvm_error_t
crankvm_heap_segment_initializeWithCapacity(crankvm_heap_segment_t *segment, size_t capacity)
{
    memset(segment, 0, sizeof(*segment));

    // Allocate the address space
    // TODO: Use VirtualAlloc/virtualProtect on Windows
    segment->address = (uint8_t*)mmap(NULL, capacity, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(segment->address == MAP_FAILED)
        return CRANK_VM_ERROR_OUT_OF_MEMORY;

    segment->addressSpaceCapacity = capacity;
    return CRANK_VM_OK;
}

static void*
crankvm_heap_segment_allocate(crankvm_heap_segment_t *segment, size_t size)
{
    if(segment->size + size > segment->addressSpaceCapacity)
        return NULL;

    uint8_t *result = segment->address + segment->size;
    size_t newSegmentSize = segment->size + size;
    if(newSegmentSize > segment->allocatedCapacity)
    {
        size_t newAllocatedCapacity = crankvm_heap_roundUpHeapSize(newSegmentSize);
        int res = mprotect(segment->address + segment->allocatedCapacity, newAllocatedCapacity - segment->allocatedCapacity, PROT_READ | PROT_WRITE);
        if(res)
            return NULL;

        segment->allocatedCapacity = newAllocatedCapacity;
    }

    segment->size = newSegmentSize;
    return result;
}

static crankvm_object_header_t *
crankvm_heap_allocate(crankvm_heap_t *heap, size_t size)
{
    return crankvm_heap_segment_allocate(&heap->firstSegment, size);
}

static crankvm_object_header_t *
crankvm_heap_newObjectWithLogicalSize(crankvm_context_t *context, crankvm_object_format_t format, size_t logicalSize)
{
    size_t slotCount = 0;

    crankvm_object_format_t instanceFormat = format;
    if(format < CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
    {
        slotCount = logicalSize;
    }
    else if(format == CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
    {
#ifdef CRANK_VM_64_BITS
        slotCount = logicalSize;
#else
        slotCount = logicalSize * 2;
#endif
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1)
    {
        const size_t elementsPerSlot = sizeof(crankvm_oop_t) / 4;
        slotCount = (logicalSize + elementsPerSlot - 1) / elementsPerSlot;

        instanceFormat += slotCount * elementsPerSlot - logicalSize;
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3)
    {
        const size_t elementsPerSlot = sizeof(crankvm_oop_t) / 2;
        slotCount = (logicalSize + elementsPerSlot - 1) / elementsPerSlot;

        instanceFormat += slotCount * elementsPerSlot - logicalSize;
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_7)
    {
        const size_t elementsPerSlot = sizeof(crankvm_oop_t);
        slotCount = (logicalSize + elementsPerSlot - 1) / elementsPerSlot;

        instanceFormat += slotCount * elementsPerSlot - logicalSize;
    }
    else // if(format <= CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_7)
    {
        const size_t elementsPerSlot = sizeof(crankvm_oop_t);
        slotCount = (logicalSize + elementsPerSlot - 1) / elementsPerSlot;

        instanceFormat += slotCount * elementsPerSlot - logicalSize;
    }

    // Make sure there is at least one physical slot, for storing a forwarding pointer.
    size_t actualSlotCount = slotCount;
    if(actualSlotCount == 0)
        actualSlotCount = 1;

    size_t bodySize = actualSlotCount * sizeof(crankvm_oop_t);
    size_t headerSize = sizeof(crankvm_object_header_t);
    if(actualSlotCount >= 255)
        headerSize += sizeof(crankvm_object_header_t);

    size_t objectSize = headerSize + bodySize;
    crankvm_object_header_t *allocatedObject = (crankvm_object_header_t*)crankvm_heap_allocate(&context->heap, objectSize);

    // Clear the allocated object.
    memset(allocatedObject, 0, objectSize);
    if(actualSlotCount >= 255)
        ++allocatedObject;

    crankvm_object_header_setSlotCount(allocatedObject, slotCount);
    crankvm_object_header_setObjectFormat(allocatedObject, instanceFormat);

    // Clear the slots with nil for pointers objects.
    if(format < CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
    {
        crankvm_oop_t *slots = (crankvm_oop_t *)&allocatedObject[1];
        crankvm_oop_t nilOop = crankvm_specialObject_nil(context);
        for(size_t i = 0; i < slotCount; ++i)
            slots[i] = nilOop;
    }

    return allocatedObject;    
}

crankvm_object_header_t *
crankvm_heap_shallowCopy(crankvm_context_t *context, crankvm_object_header_t *sourceObject)
{
    size_t slotCount = crankvm_object_header_getSlotCount(sourceObject);
    crankvm_object_format_t format = crankvm_object_header_getObjectFormat(sourceObject);
    unsigned int classIndex = crankvm_object_header_getClassIndex(sourceObject);
    
    // Make sure there is at least one physical slot, for storing a forwarding pointer.
    size_t actualSlotCount = slotCount;
    if(actualSlotCount == 0)
        actualSlotCount = 1;

    size_t bodySize = actualSlotCount * sizeof(crankvm_oop_t);
    size_t headerSize = sizeof(crankvm_object_header_t);
    if(actualSlotCount >= 255)
        headerSize += sizeof(crankvm_object_header_t);

    size_t objectSize = headerSize + bodySize;
    crankvm_object_header_t *allocatedObject = (crankvm_object_header_t*)crankvm_heap_allocate(&context->heap, objectSize);
    memcpy(allocatedObject, sourceObject, objectSize);
    memset(allocatedObject, 0, sizeof(crankvm_object_header_t));
    
    crankvm_object_header_setSlotCount(allocatedObject, slotCount);
    crankvm_object_header_setObjectFormat(allocatedObject, format);
    crankvm_object_header_setClassIndex(allocatedObject, classIndex);

    return allocatedObject;
}

crankvm_object_header_t *
crankvm_heap_newObject(crankvm_context_t *context, crankvm_object_format_t format, size_t fixedSize, size_t variableSize)
{
    return crankvm_heap_newObjectWithLogicalSize(context, format, fixedSize + variableSize);
}

static crankvm_error_t
crankvm_heap_initializeFor(crankvm_heap_t *heap, size_t initialHeapSize)
{
    return crankvm_heap_segment_initializeWithCapacity(&heap->firstSegment, heap->maxCapacity);
}

crankvm_error_t
crankvm_heap_destroy(crankvm_heap_t *heap)
{
    return CRANK_VM_OK;
}

crankvm_spur_segment_info_t *
crankvm_heap_allocateSegmentInfo(crankvm_heap_t *heap)
{
    if(heap->segmentInfoSize >= heap->segmentInfoCapacity)
    {
        heap->segmentInfoCapacity *= 2;
        if(heap->segmentInfoCapacity == 0)
            heap->segmentInfoCapacity = 16;

        crankvm_spur_segment_info_t * newInfos = realloc(heap->segmentInfos, heap->segmentInfoCapacity*sizeof(crankvm_spur_segment_info_t));
        if(!newInfos)
            return NULL;
        heap->segmentInfos = newInfos;
    }

    crankvm_spur_segment_info_t *result = &heap->segmentInfos[heap->segmentInfoSize++];
    memset(result, 0, sizeof(crankvm_spur_segment_info_t));
    return result;
}


crankvm_heap_iterator_t
crankvm_heap_iterator_create(crankvm_heap_t *heap)
{
    crankvm_heap_iterator_t iterator;
    memset(&iterator, 0, sizeof(iterator));

    iterator.heap = heap;
    iterator.segment = &heap->firstSegment;

    crankvm_heap_iterator_advance(&iterator);

    return iterator;
}

void
crankvm_heap_iterator_advance(crankvm_heap_iterator_t *iterator)
{
    if(iterator->atEnd)
        return;

    if(iterator->currentObjectEnd + sizeof(crankvm_object_header_t) >= iterator->segment->size)
    {
        // TODO: Advance the segment
        iterator->atEnd = 1;
        return;
    }

    iterator->currentObjectStart = iterator->currentObjectEnd;

    crankvm_object_header_t *header = (crankvm_object_header_t *)(iterator->segment->address + iterator->currentObjectEnd);
    iterator->currentObjectEnd += sizeof(crankvm_object_header_t);
    if(crankvm_object_header_getRawSlotCount(header) == 255)
    {
        // This is an extension header
        assert(iterator->currentObjectEnd + sizeof(crankvm_object_header_t) <= iterator->segment->size);
        iterator->currentObjectEnd += sizeof(crankvm_object_header_t);
        ++header;
    }

    // We found the header.
    iterator->currentHeader = header;

    size_t slotCount = crankvm_object_header_getSlotCount(header);
    iterator->currentObjectSlots = (crankvm_oop_t*)&header[1];
    iterator->currentObjectSlotCount = slotCount;
    iterator->currentObjectEnd += slotCount * sizeof(crankvm_oop_t);
    if(slotCount == 0) // For forwarding pointer
        iterator->currentObjectEnd += sizeof(crankvm_oop_t);
}

crankvm_object_header_t *
crankvm_heap_objectPointerAfter(crankvm_heap_t *heap, crankvm_object_header_t *object)
{
    size_t objectSize = sizeof(crankvm_object_header_t);
    size_t slotCount = crankvm_object_header_getSlotCount(object);
    if(slotCount == 0)
        slotCount = 1;
    objectSize += slotCount * sizeof(crankvm_oop_t);

    crankvm_object_header_t *nextObject = (crankvm_object_header_t *) (((uint8_t*)object) + objectSize);
    if(crankvm_object_header_getRawSlotCount(nextObject) == 255)
        ++nextObject;
    return nextObject;
}

static crankvm_error_t
crankvm_heap_loadClassTable(crankvm_context_t *context)
{
    crankvm_HiddenRoots_t *hiddenRoots = context->roots.hiddenRootsObject;
    context->numberOfClassTablePages = CRANK_VM_CLASS_TABLE_ROOT_COUNT;
    context->roots.firstClassTablePage = hiddenRoots->classTablePages[0];
    if(crankvm_object_isNil(context, context->roots.firstClassTablePage))
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    // Check the class table.
    for(size_t i = 1; i < context->numberOfClassTablePages; ++i)
    {
        if(crankvm_object_isNil(context, hiddenRoots->classTablePages[i]))
        {
            context->numberOfClassTablePages = i;
            context->nextClassTableIndex = i;
        }
    }

    // There are not unused pages, so start wit the second one.
    context->nextClassTableIndex = 1;
    return CRANK_VM_OK;
}

crankvm_error_t
crankvm_heap_loadImageContent(crankvm_context_t *context, crankvm_read_memory_stream_t *stream, crankvm_image_header_t *header)
{
    crankvm_heap_t *heap = &context->heap;

    // Make sure the heap has the required capacity for the image.
    size_t initialHeapSize = crankvm_heap_roundUpHeapSize(header->imageBytes + header->extraVMMemory);
    if(initialHeapSize > heap->maxCapacity)
        return CRANK_VM_ERROR_OUT_OF_MEMORY;

    // Initialize the heap.
    crankvm_error_t error = crankvm_heap_initializeFor(heap, initialHeapSize);
    if(error)
        return error;

    // Load the image segments.
    crankvm_heap_segment_t *targetSegment = &heap->firstSegment;
    size_t nextSegmentSize = header->firstSegmentSize;
    uintptr_t oldBaseAddress = header->startOfMemory;

    do
    {
        printf("Load segment %zu\n", nextSegmentSize);
        crankvm_spur_segment_info_t *segmentInfo = crankvm_heap_allocateSegmentInfo(heap);
        if(!segmentInfo)
            return CRANK_VM_ERROR_OUT_OF_MEMORY;

        uint8_t *targetPointer = crankvm_heap_segment_allocate(targetSegment, nextSegmentSize);
        if(!targetPointer)
            return CRANK_VM_ERROR_OUT_OF_MEMORY;

        segmentInfo->startAddress = targetPointer;
        segmentInfo->size = nextSegmentSize;
        segmentInfo->swizzle = ((uintptr_t)segmentInfo->startAddress) - oldBaseAddress;

        //printf("Read segment of size %zu into %p\n", nextSegmentSize, targetPointer);
        if(!crankvm_read_memory_stream_nextBytesInto(stream, nextSegmentSize, targetPointer))
            return CRANK_VM_ERROR_BAD_IMAGE;

        uint64_t *bridge = (uint64_t*)(targetPointer + nextSegmentSize - 8);
        size_t bridgeSpan = 0;
        size_t bridgeSize = *bridge;
        printf("bridgeSize %zu\n", bridgeSize);
        crankvm_object_header_t *bridgeObject = (crankvm_object_header_t *)bridge;
        if(crankvm_object_header_getRawSlotCount((crankvm_object_header_t*)(bridge - 1)) == 0)
        {
            // Ignore the last element
            bridgeSpan = 0;
            //targetSegment->size -= 8;
            //printf("Last bridguintptr_te %p\n", bridgeObject);
        }
        else
        {
            bridgeSpan = crankvm_object_header_getRawSlotOverflowCount(bridgeObject);
            memset(bridgeObject, 0, sizeof(*bridgeObject));
            crankvm_object_header_setRawSlotOverflowCount(bridgeObject, 0);
            crankvm_object_header_setRawSlotCount(bridgeObject, 255);
        }

        oldBaseAddress += bridgeSpan;
        nextSegmentSize = bridgeSize;
        //printf("Bridge span %zu next %zu\n", bridgeSpan, bridgeSize);
    } while(nextSegmentSize != 0);

    // Time to fixup the image segments.
    crankvm_heap_iterator_t iterator = crankvm_heap_iterator_create(heap);
    crankvm_spur_segment_info_t *currentSegmentInfo = NULL;
    crankvm_spur_segment_info_t *nextSegmentInfo = &heap->segmentInfos[0];
    for(; !iterator.atEnd; crankvm_heap_iterator_advance(&iterator))
    {
        if(iterator.segment->address + iterator.currentObjectStart == nextSegmentInfo->startAddress)
        {
            currentSegmentInfo = nextSegmentInfo;
            ++nextSegmentInfo;
        }

        // Apply the swizzle to the slots
        crankvm_object_format_t format = crankvm_oop_getFormat((crankvm_oop_t)iterator.currentHeader);
        if(format < CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
        {
            for(size_t i = 0; i < iterator.currentObjectSlotCount; ++i)
            {
                if(crankvm_oop_isPointer(iterator.currentObjectSlots[i]))
                    iterator.currentObjectSlots[i] += currentSegmentInfo->swizzle;
            }
        }
        else if(format >= CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD)
        {
            crankvm_CompiledCode_t *compiledCode = (crankvm_CompiledCode_t *)iterator.currentHeader;
            size_t numberOfLiterals = crankvm_CompiledCode_getNumberOfLiterals(context, compiledCode);
            //printf("Swizzle compiled %p literal count %zu\n", compiledCode, numberOfLiterals);
            for(size_t i = 0; i < numberOfLiterals; ++i)
            {
                if(crankvm_oop_isPointer(compiledCode->literals[i]))
                    compiledCode->literals[i] += currentSegmentInfo->swizzle;
            }
        }
    }
    assert(iterator.currentObjectEnd == targetSegment->size);

    context->roots.specialObjectsArray = (crankvm_special_object_array_t*)(header->specialObjectsOop - header->startOfMemory + (uintptr_t)heap->firstSegment.address);
    // TODO: Validate the special objects array

    // Make sure nil, false, and true are in sequencial order.
    context->roots.nilOop = context->roots.specialObjectsArray->nilObject;
    context->roots.falseOop = context->roots.specialObjectsArray->falseObject;
    context->roots.trueOop = context->roots.specialObjectsArray->trueObject;

    // Initialize with nil some other non-mandatory objects.
    context->roots.byteSymbolClassOop = context->roots.nilOop;

    if(context->roots.nilOop != (crankvm_oop_t)heap->firstSegment.address)
        return CRANK_VM_ERROR_INVALID_PARAMETER;
    if(context->roots.falseOop != (crankvm_oop_t)crankvm_heap_objectPointerAfter(heap, (crankvm_object_header_t*)context->roots.nilOop))
        return CRANK_VM_ERROR_INVALID_PARAMETER;
    if(context->roots.trueOop != (crankvm_oop_t)crankvm_heap_objectPointerAfter(heap, (crankvm_object_header_t*)context->roots.falseOop))
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    context->roots.freeListObject = (crankvm_oop_t)crankvm_heap_objectPointerAfter(heap, (crankvm_object_header_t*)context->roots.trueOop);

    // Get the class table.
    context->roots.hiddenRootsObject = (crankvm_HiddenRoots_t*)crankvm_heap_objectPointerAfter(heap, (crankvm_object_header_t*)context->roots.freeListObject);
    error = crankvm_heap_loadClassTable(context);
    if(error)
        return error;

    // TODO: Swizzle the object stacks SpurMemoryManager>> #swizzleObjStackAt:

    free(heap->segmentInfos);
    heap->segmentInfoCapacity = 0;
    heap->segmentInfoSize = 0;

    // Infer some additional classes that are used for debugging purposes.
    context->roots.byteSymbolClassOop = crankvm_object_getClass(context, context->roots.specialObjectsArray->selectorDoesNotUnderstand);


    return CRANK_VM_OK;
}
