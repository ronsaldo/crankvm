#ifndef _CRANK_VM_OBJECT_MODEL_H_
#define _CRANK_VM_OBJECT_MODEL_H_

#include <crank-vm/assert.h>
#include <crank-vm/oop.h>
#include <crank-vm/common.h>

#if CRANK_VM_WORD_SIZE == 4

enum crankvm_oop_tag_e
{
    CRANK_VM_OOP_TAG_BITS = 2,

    CRANK_VM_OOP_TAG_POINTER_MASK = 3,
    CRANK_VM_OOP_TAG_POINTER_VALUE = 0,
    CRANK_VM_OOP_TAG_POINTER_SHIFT = 0,

    CRANK_VM_OOP_TAG_SMALL_INTEGER_MASK = 1,
    CRANK_VM_OOP_TAG_SMALL_INTEGER_VALUE = 1,
    CRANK_VM_OOP_TAG_SMALL_INTEGER_SHIFT = 1,

    CRANK_VM_OOP_TAG_CHARACTER_MASK = 3,
    CRANK_VM_OOP_TAG_CHARACTER_VALUE = 2,
    CRANK_VM_OOP_TAG_CHARACTER_SHIFT = 2,

    CRANK_VM_OOP_TAG_SMALL_FLOAT_MASK = 0,
    CRANK_VM_OOP_TAG_SMALL_FLOAT_VALUE = 42, // Anything but zero
    CRANK_VM_OOP_TAG_SMALL_FLOAT_SHIFT = 0,
};

#else
enum crankvm_oop_tag_e
{
    CRANK_VM_OOP_TAG_BITS = 3,

    CRANK_VM_OOP_TAG_POINTER_MASK = 7,
    CRANK_VM_OOP_TAG_POINTER_VALUE = 0,
    CRANK_VM_OOP_TAG_POINTER_SHIFT = 0,

    CRANK_VM_OOP_TAG_SMALL_INTEGER_MASK = 7,
    CRANK_VM_OOP_TAG_SMALL_INTEGER_VALUE = 1,
    CRANK_VM_OOP_TAG_SMALL_INTEGER_SHIFT = 3,

    CRANK_VM_OOP_TAG_CHARACTER_MASK = 7,
    CRANK_VM_OOP_TAG_CHARACTER_VALUE = 2,
    CRANK_VM_OOP_TAG_CHARACTER_SHIFT = 3,

    CRANK_VM_OOP_TAG_SMALL_FLOAT_MASK = 7,
    CRANK_VM_OOP_TAG_SMALL_FLOAT_VALUE = 4,
    CRANK_VM_OOP_TAG_SMALL_FLOAT_SHIFT = 3,
};
#endif

#define CRANK_VM_IDENTITY_HASH_MASK ((1<<22) - 1)
#define CRANK_VM_IDENTITY_OBJECT_ALIGNMENT 8

#define CRANK_VM_SMALL_FLOAT_EXPONENT_OFFSET (/* 1023 - 127 */ 896)
#define CRANK_VM_SMALL_FLOAT_EXPONENT_MIN (/* 1023 - 127 */ SmallFloatExponentOffset)
#define CRANK_VM_SMALL_FLOAT_EXPONENT_MAX (/* 1023 - 127 */ 1151)

inline int crankvm_oop_isPointers(crankvm_oop_t oop)
{
    return (oop & CRANK_VM_OOP_TAG_POINTER_MASK) == CRANK_VM_OOP_TAG_POINTER_VALUE;
}

inline int crankvm_oop_isSmallInteger(crankvm_oop_t oop)
{
    return (oop & CRANK_VM_OOP_TAG_SMALL_INTEGER_MASK) == CRANK_VM_OOP_TAG_SMALL_INTEGER_VALUE;
}

inline int crankvm_oop_isCharacter(crankvm_oop_t oop)
{
    return (oop & CRANK_VM_OOP_TAG_CHARACTER_MASK) == CRANK_VM_OOP_TAG_CHARACTER_VALUE;
}

inline int crankvm_oop_isSmallFloat(crankvm_oop_t oop)
{
    return (oop & CRANK_VM_OOP_TAG_SMALL_FLOAT_MASK) == CRANK_VM_OOP_TAG_SMALL_FLOAT_VALUE;
}

inline intptr_t crankvm_oop_decodeSmallInteger(crankvm_soop_t oop)
{
    crankvm_assertAlways(crankvm_oop_isSmallInteger(oop));
    return oop >> CRANK_VM_OOP_TAG_SMALL_INTEGER_SHIFT;
}

inline crankvm_oop_t crankvm_oop_encodeSmallInteger(intptr_t integer)
{
    // TODO: Check the integer range
    return (integer << CRANK_VM_OOP_TAG_SMALL_INTEGER_SHIFT) | CRANK_VM_OOP_TAG_SMALL_INTEGER_VALUE;
}

typedef enum crankvm_object_format_e
{
	CRANK_VM_OBJECT_FORMAT_EMPTY = 0,
	CRANK_VM_OBJECT_FORMAT_FIXED_SIZE = 1,
	CRANK_VM_OBJECT_FORMAT_VARIABLE_SIZE_NO_IVARS = 2,
	CRANK_VM_OBJECT_FORMAT_VARIABLE_SIZE_IVARS = 3,
	CRANK_VM_OBJECT_FORMAT_WEAK_VARIABLE_SIZE = 4,
	CRANK_VM_OBJECT_FORMAT_WEAK_FIXED_SIZE = 5,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_64 = 9,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_32 = 10,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_16 = 12,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_1,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_2,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_8 = 16,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_1,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_2,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_3,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_4,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_5,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_6,
	CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_7,
	CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD = 24,
	CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_1,
	CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_2,
	CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_3,
	CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_4,
	CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_5,
	CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_6,
	CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_7,

	CRANK_VM_OBJECT_FORMAT_INDEXABLE_NATIVE_FIRST = CRANK_VM_OBJECT_FORMAT_INDEXABLE_64,
}crankvm_object_format_t;

/***
 * Object header
 */
typedef struct crankvm_object_header_s
{
    struct __data
    {
        // TODO: Be able to remove these bit fields
        unsigned int slotCount : 8;
    	unsigned int isImmutable : 1;
    	unsigned int isPinned : 1;
    	unsigned int identityHash : 22;
    	unsigned int gcColor : 3;
    	unsigned int objectFormat : 5;
    	unsigned int reserved : 2;
    	unsigned int classIndex : 22;
    } hidden_data;
} crankvm_object_header_t;

inline unsigned int crankvm_object_header_getSlotCount(crankvm_object_header_t *header)
{
    return header->hidden_data.slotCount;
}

inline void crankvm_object_header_setSlotCount(crankvm_object_header_t *header, unsigned int value)
{
    header->hidden_data.slotCount = value;
}

inline unsigned int crankvm_object_header_isImmutable(crankvm_object_header_t *header)
{
    return header->hidden_data.isImmutable;
}

inline void crankvm_object_header_setIsImmutable(crankvm_object_header_t *header, unsigned int value)
{
    header->hidden_data.isImmutable = value;
}

inline unsigned int crankvm_object_header_getIsPinned(crankvm_object_header_t *header)
{
    return header->hidden_data.isPinned;
}

inline void crankvm_object_header_setIsPinned(crankvm_object_header_t *header, unsigned int value)
{
    header->hidden_data.isPinned = value;
}

inline unsigned int crankvm_object_header_getIdentityHash(crankvm_object_header_t *header)
{
    return header->hidden_data.identityHash;
}

inline void crankvm_object_header_setIdentityHash(crankvm_object_header_t *header, unsigned int value)
{
    header->hidden_data.identityHash = value;
}

inline crankvm_object_format_t crankvm_object_header_getObjectFormat(crankvm_object_header_t *header)
{
    return (crankvm_object_format_t)header->hidden_data.objectFormat;
}

inline void crankvm_object_header_setObjectFormat(crankvm_object_header_t *header, crankvm_object_format_t value)
{
    header->hidden_data.objectFormat = value;
}

inline unsigned int crankvm_object_header_getClassIndex(crankvm_object_header_t *header)
{
    return header->hidden_data.classIndex;
}

inline void crankvm_object_header_setClassIndex(crankvm_object_header_t *header, unsigned int value)
{
    header->hidden_data.classIndex = value;
}

#endif //_CRANK_VM_OBJECT_MODEL_H_
