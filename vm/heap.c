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
        if(crankvm_object_header_getObjectFormat(iterator.currentHeader) < CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
        {
            for(size_t i = 0; i < iterator.currentObjectSlotCount; ++i)
            {
                if(crankvm_oop_isPointers(iterator.currentObjectSlots[i]))
                    iterator.currentObjectSlots[i] += currentSegmentInfo->swizzle;
            }
        }
    }
    assert(iterator.currentObjectEnd == targetSegment->size);

    context->specialObjectsArray = (crankvm_special_object_array_t*)(header->specialObjectsOop - header->startOfMemory + (uintptr_t)heap->firstSegment.address);
    // TODO: Validate the special objects array

    free(heap->segmentInfos);
    heap->segmentInfoCapacity = 0;
    heap->segmentInfoSize = 0;
    return CRANK_VM_OK;
}
