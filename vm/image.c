#include "context-internal.h"
#include "image.h"
#include "read-memory-stream.h"
#include "heap.h"
#include <string.h>
#include <stdlib.h>

static crankvm_error_t
crankvm_image_readFormatFromIntegerAndLittleEndianness(crankvm_image_format_t *format, uint32_t integer, bool littleEndian)
{
    if(integer & CRANK_VM_IMAGE_FORMAT_RESERVED_BIT_MASK)
        return CRANK_VM_ERROR_BAD_IMAGE;

    memset(format, 0, sizeof(*format));
    format->isLittleEndian = littleEndian;

    uint32_t baseVersion = CRANK_VM_IMAGE_FORMAT_BASE_VERSION_MASK & integer;
    //printf("%08x Base version %d isLittleEndian %d\n", integer, baseVersion, littleEndian);
    if(baseVersion == 6504 || baseVersion == 68002 || baseVersion == 68004)
        format->requiresClosureSupport = true;

    if(baseVersion == 6502 || baseVersion == 6504)
    {
        format->wordSize = 4;
    }
    else if(baseVersion == 68000 || baseVersion == 68002 || baseVersion == 68004)
    {
        format->wordSize = 8;
        if(baseVersion == 68004)
            format->requiresNewSpur64TagAssignmentRequirement = true;
    }
    else
    {
        return CRANK_VM_ERROR_BAD_IMAGE;
    }

    uint32_t capabilitiesBits = integer & CRANK_VM_IMAGE_FORMAT_CAPABILITIES_BIT_MASK;
    if(capabilitiesBits & CRANK_VM_IMAGE_FORMAT_PLATFORM_BYTE_ORDER_BIT)
    {
        format->requiresNativeFloatWordOrder = true;
        if(!format->requiresClosureSupport)
            return CRANK_VM_ERROR_BAD_IMAGE;
        capabilitiesBits &= ~CRANK_VM_IMAGE_FORMAT_PLATFORM_BYTE_ORDER_BIT;
    }

    if(capabilitiesBits & CRANK_VM_IMAGE_FORMAT_SPUR_OBJECT_BIT)
    {
        format->requiresSpurSupport = true;
        if(!format->requiresClosureSupport)
            return CRANK_VM_ERROR_BAD_IMAGE;
        if(!format->requiresNativeFloatWordOrder)
            return CRANK_VM_ERROR_BAD_IMAGE;
        capabilitiesBits &= ~CRANK_VM_IMAGE_FORMAT_SPUR_OBJECT_BIT;
    }

    // TODO: add additional capability bit handling here
    if(capabilitiesBits != 0)
        return CRANK_VM_ERROR_BAD_IMAGE;

    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_image_readFormat(crankvm_read_memory_stream_t *stream, crankvm_image_format_t *format)
{
    uint32_t word;
    if(!crankvm_read_memory_stream_nextU32BigEndian(stream, &word))
        return CRANK_VM_ERROR_BAD_IMAGE;

    switch(word)
    {
    case 0x00001966: // "6502"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 6502, false);
    case 0x66190000: // "6502"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 6502, true);

    case 0x00001968: // "6504"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 6504, false);
    case 0x68190000: // "6504"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 6504, true);

    case 0x00001969: // "6505"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 6505, false);
    case 0x69190000: // "6505"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 6505, true);

    case 0x00001979: // "6521"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 6521, false);
    case 0x79190000: // "6521"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 6521, true);

    case 0xA0090100: // "68000"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 68000, true);
    case 0xA2090100: // "68002"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 68002, true);
    case 0xA3090100: // "68003"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 68003, true);

    case 0xB3090100: // "68019"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 68019, true);
    case 0x000109B3: // "68019"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 68019, false);

    case 0xB5090100: // "68021"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 68021, true);
    case 0x000109B5: // "68021"
        return crankvm_image_readFormatFromIntegerAndLittleEndianness(format, 68021, false);
    default:
        return CRANK_VM_ERROR_UNRECOGNIZED_IMAGE_FORMAT;
    }
}

static crankvm_error_t
crankvm_image_readHeader(crankvm_read_memory_stream_t *stream, crankvm_image_header_t *header)
{
    memset(header, 0, sizeof(crankvm_image_header_t));
    size_t startPosition = stream->position;

    crankvm_error_t error = crankvm_image_readFormat(stream, &header->format);
    if(error)
        return error;

    // Make sure the image has the correct capabilities.
    if(!header->format.requiresClosureSupport || !header->format.requiresNativeFloatWordOrder || ! header->format.requiresSpurSupport)
        return CRANK_VM_ERROR_BAD_IMAGE;

    // Check the word size.
    if(header->format.wordSize != sizeof(crankvm_oop_t))
        return CRANK_VM_ERROR_BAD_IMAGE_WORD_SIZE;

    // Check the endiannes
    if(header->format.isLittleEndian != CRANK_VM_LITTLE_ENDIAN)
        return CRANK_VM_ERROR_BAD_IMAGE_ENDIANNESS;

    // Standard headers
    if(!crankvm_read_memory_stream_nextU32(stream, &header->headerSize))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextWord(stream, &header->imageBytes))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextWord(stream, &header->startOfMemory))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextWord(stream, &header->specialObjectsOop))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextWord(stream, &header->lastHash))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextWord(stream, &header->screenSizeWord))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextWord(stream, &header->imageHeaderFlags))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextU32(stream, &header->extraVMMemory))
        return CRANK_VM_ERROR_BAD_IMAGE;

    // Cog headers
    if(!crankvm_read_memory_stream_nextU16(stream, &header->desiredNumStackPages))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextU16(stream, &header->unknownShortOrCodeSizeInKs))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextU32(stream, &header->desiredEdenBytes))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextU16(stream, &header->maxExtSemTabSizeSet))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextU16(stream, &header->secondUnknownShort))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextWord(stream, &header->firstSegmentSize))
        return CRANK_VM_ERROR_BAD_IMAGE;
    if(!crankvm_read_memory_stream_nextWord(stream, &header->freeOldSpaceInImage))
        return CRANK_VM_ERROR_BAD_IMAGE;

    // Check the readed size
    size_t readedSize = stream->position - startPosition;
    if(readedSize > header->headerSize)
        return CRANK_VM_ERROR_BAD_IMAGE;

    stream->position = startPosition + header->headerSize;

    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_context_loadImageFromMemory(crankvm_context_t *context, size_t imageSize, const void *imageData)
{
    if(!context || !imageData)
        return CRANK_VM_ERROR_NULL_POINTER;

    if(imageSize < 8)
        return CRANK_VM_ERROR_BAD_IMAGE;

    crankvm_read_memory_stream_t stream = crankvm_read_memory_stream_create(imageData, 0, imageSize);

    // Read the header
    crankvm_image_header_t header;
    crankvm_error_t error = crankvm_image_readHeader(&stream, &header);
    if(error)
        return error;

    context->lastIdentityHash = header.lastHash;

    return crankvm_heap_loadImageContent(context, &stream, &header);
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_context_loadImageFromFileNamed(crankvm_context_t *context, const char *fileName)
{
    if(!context)
        return CRANK_VM_ERROR_NULL_POINTER;

    // TODO: Use memory mapping, if possible
    FILE *file = fopen(fileName, "rb");
    if(!file)
        return CRANK_VM_ERROR_FAILED_TO_OPEN_FILE;

    // Get the image file size
    fseek(file, 0, SEEK_END);
    size_t imageSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate a buffer for reading the image.
    uint8_t *buffer = malloc(imageSize);
    if(!buffer)
    {
        fclose(file);
        return CRANK_VM_ERROR_OUT_OF_MEMORY;
    }

    // Read the full image content, and close the image file.
    size_t readedCount = fread(buffer, imageSize, 1, file);
    fclose(file);
    if(readedCount != 1)
    {
        free(buffer);
        return CRANK_VM_ERROR_FAILED_TO_READ_FILE;
    }

    // Load the image from the readed buffer.
    crankvm_error_t error = crankvm_context_loadImageFromMemory(context, imageSize, buffer);
    free(buffer);
    return error;
}
