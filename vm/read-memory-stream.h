#ifndef CRANK_VM_READ_MEMORY_STREAM_H
#define CRANK_VM_READ_MEMORY_STREAM_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef struct crankvm_read_memory_stream_s
{
    const uint8_t *data;
    size_t position;
    size_t size;
} crankvm_read_memory_stream_t;

static inline crankvm_read_memory_stream_t
crankvm_read_memory_stream_create(const void *data, size_t position, size_t size)
{
    crankvm_read_memory_stream_t result;
    result.data = (const uint8_t*)data;
    result.position = position;
    result.size = size;
    return result;
}

static inline int
crankvm_read_memory_stream_hasNextSize(crankvm_read_memory_stream_t *stream, size_t size)
{
    return stream->size - stream->position >= size;
}

static inline int
crankvm_read_memory_stream_nextU32(crankvm_read_memory_stream_t *stream, uint32_t *result)
{
    if(!crankvm_read_memory_stream_hasNextSize(stream, 4))
        return 0;

    *result = * ((uint32_t*)(stream->data + stream->position));
    stream->position += 4;
    return 1;
}

static inline int
crankvm_read_memory_stream_nextU16(crankvm_read_memory_stream_t *stream, uint16_t *result)
{
    if(!crankvm_read_memory_stream_hasNextSize(stream, 2))
        return 0;

    *result = * ((uint16_t*)(stream->data + stream->position));
    stream->position += 2;
    return 1;
}

static inline int
crankvm_read_memory_stream_nextWord(crankvm_read_memory_stream_t *stream, uintptr_t *result)
{
    if(!crankvm_read_memory_stream_hasNextSize(stream, 4))
        return 0;

    *result = *((uintptr_t*)(stream->data + stream->position));
    stream->position += sizeof(uintptr_t);
    return 1;
}


static inline int
crankvm_read_memory_stream_nextU32BigEndian(crankvm_read_memory_stream_t *stream, uint32_t *result)
{
    if(!crankvm_read_memory_stream_hasNextSize(stream, 4))
        return 0;

    *result = stream->data[stream->position++];
    *result = (*result << 8) | stream->data[stream->position++];
    *result = (*result << 8) | stream->data[stream->position++];
    *result = (*result << 8) | stream->data[stream->position++];

    return 1;
}

static inline int
crankvm_read_memory_stream_nextI32(crankvm_read_memory_stream_t *stream, int32_t *result)
{
    if(!crankvm_read_memory_stream_hasNextSize(stream, 4))
        return 0;

    *result = *((int32_t*)(stream->data + stream->position));
    stream->position += 4;
    return 1;
}

static inline int
crankvm_read_memory_stream_nextBytesInto(crankvm_read_memory_stream_t *stream, size_t size, uint8_t *result)
{
    if(!crankvm_read_memory_stream_hasNextSize(stream, size))
        return 0;

    memcpy(result, stream->data + stream->position, size);
    stream->position += size;
    return 1;
}

#endif //CRANK_VM_READ_MEMORY_STREAM_H
