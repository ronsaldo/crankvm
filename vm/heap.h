#ifndef CRANK_VM_HEAP_H
#define CRANK_VM_HEAP_H

#include <crank-vm/objectmodel.h>
#include <crank-vm/error.h>

typedef struct crankvm_spur_segment_info_s {
	uint8_t *startAddress;
	size_t size;
	intptr_t swizzle;
} crankvm_spur_segment_info_t;

typedef struct crankvm_heap_segment_s
{
    size_t addressSpaceCapacity;
    size_t allocatedCapacity;
    size_t size;
    uint8_t *address;
} crankvm_heap_segment_t;

typedef struct crankvm_heap_s {
    size_t maxCapacity;
    crankvm_heap_segment_t firstSegment;

    crankvm_spur_segment_info_t *segmentInfos;
    size_t segmentInfoCapacity;
    size_t segmentInfoSize;
} crankvm_heap_t;

typedef struct crankvm_heap_iterator_s {
    crankvm_heap_t *heap;
    crankvm_heap_segment_t *segment;

    crankvm_object_header_t *currentHeader;
    crankvm_oop_t *currentObjectSlots;

    size_t currentObjectStart;
    size_t currentObjectSlotCount;
    size_t currentObjectEnd;

    int atEnd;
} crankvm_heap_iterator_t;

typedef struct crankvm_image_header_s crankvm_image_header_t;
typedef struct crankvm_read_memory_stream_s crankvm_read_memory_stream_t;
typedef struct crankvm_context_s crankvm_context_t;

crankvm_error_t crankvm_heap_destroy(crankvm_heap_t *heap);
crankvm_error_t crankvm_heap_loadImageContent(crankvm_context_t *context, crankvm_read_memory_stream_t *stream, crankvm_image_header_t *header);

crankvm_heap_iterator_t crankvm_heap_iterator_create(crankvm_heap_t *heap);
void crankvm_heap_iterator_advance(crankvm_heap_iterator_t *iterator);

crankvm_object_header_t *crankvm_heap_objectPointerAfter(crankvm_heap_t *heap, crankvm_object_header_t *object);

crankvm_object_header_t *crankvm_heap_newObject(crankvm_context_t *context, crankvm_object_format_t format, size_t fixedSize, size_t variableSize);
crankvm_object_header_t *crankvm_heap_shallowCopy(crankvm_context_t *context, crankvm_object_header_t *sourceObject);

#endif //CRANK_VM_HEAP_H
