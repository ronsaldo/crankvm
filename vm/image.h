#ifndef CRANK_VM_IMAGE_H
#define CRANK_VM_IMAGE_H

#include <stdbool.h>
#include <stdint.h>

// Image format bits. Extracted from ImageFormat class >> initialize.
#define CRANK_VM_IMAGE_BIT_AT(x) (1<<((x)-1))
#define CRANK_VM_IMAGE_FORMAT_PLATFORM_BYTE_ORDER_BIT CRANK_VM_IMAGE_BIT_AT(1)
#define CRANK_VM_IMAGE_FORMAT_SPUR_OBJECT_BIT CRANK_VM_IMAGE_BIT_AT(5)
#define CRANK_VM_IMAGE_FORMAT_BASE_VERSION_MASK 72174
#define CRANK_VM_IMAGE_FORMAT_CAPABILITIES_BIT_MASK 17
#define CRANK_VM_IMAGE_FORMAT_RESERVED_BIT_MASK 2147411456

#define CRANK_VM_IMAGE_FORMAT_BASE_VERSION_NUMBERS 6502 6504 68000 68002
#define CRANK_VM_IMAGE_FORMAT_KNOWN_VERSION_NUMBERS 6502 6504 6505 6521 68000 68002 68003 68019 68021

typedef struct crankvm_image_format_s
{
    bool isLittleEndian;
    int wordSize;

    bool requiresClosureSupport;
    bool requiresNewSpur64TagAssignmentRequirement;
    bool requiresNativeFloatWordOrder;
    bool requiresSpurSupport;
} crankvm_image_format_t;

typedef struct crankvm_image_header_s
{
    crankvm_image_format_t format;

    uint32_t headerSize;
    uintptr_t imageBytes;
    uintptr_t startOfMemory;
    uintptr_t specialObjectsOop;
    uintptr_t lastHash;
    uintptr_t screenSizeWord;
    uintptr_t imageHeaderFlags;
    uint32_t extraVMMemory;

    // Cog specific header
    uint16_t desiredNumStackPages;
	uint16_t unknownShortOrCodeSizeInKs;
	uint32_t desiredEdenBytes;
	uint16_t maxExtSemTabSizeSet;
	uint16_t secondUnknownShort;

    uintptr_t firstSegmentSize;
    uintptr_t freeOldSpaceInImage;

} crankvm_image_header_t;

#endif //CRANK_VM_IMAGE_H
