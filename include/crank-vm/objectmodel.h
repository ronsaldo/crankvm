#ifndef _CRANK_VM_OBJECT_MODEL_H_
#define _CRANK_VM_OBJECT_MODEL_H_

#include <crank-vm/assert.h>
#include <crank-vm/oop.h>
#include <crank-vm/common.h>
#include <crank-vm/error.h>
#include <string.h>
#if CRANK_VM_WORD_SIZE == 4

enum crankvm_oop_tag_e
{
    CRANK_VM_OOP_TAG_BITS = 2,
    CRANK_VM_OOP_TAG_MASK = (1<<CRANK_VM_OOP_TAG_BITS) - 1,

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

    CRANK_VM_OOP_BITS = 32
};

#else
enum crankvm_oop_tag_e
{
    CRANK_VM_OOP_TAG_BITS = 3,
    CRANK_VM_OOP_TAG_MASK = (1<<CRANK_VM_OOP_TAG_BITS) - 1,

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

    CRANK_VM_OOP_BITS = 64,
};
#endif

#define CRANK_VM_IDENTITY_HASH_MASK ((1<<22) - 1)
#define CRANK_VM_IDENTITY_OBJECT_ALIGNMENT 8

#define CRANK_VM_SMALL_FLOAT_EXPONENT_OFFSET (/* 1023 - 127 */ 896)
#define CRANK_VM_SMALL_FLOAT_EXPONENT_MIN (/* 1023 - 127 */ CRANK_VM_SMALL_FLOAT_EXPONENT_OFFSET)
#define CRANK_VM_SMALL_FLOAT_EXPONENT_MAX (/* 1023 + 127 */ 1151)

#define CRANK_VM_SMALL_INTEGER_USABLE_BITS (CRANK_VM_OOP_BITS - CRANK_VM_OOP_TAG_SMALL_INTEGER_SHIFT)
#define CRANK_VM_SMALL_INTEGER_MIN_VALUE (-( ((intptr_t)1)<<(CRANK_VM_SMALL_INTEGER_USABLE_BITS - 1) ) )
#define CRANK_VM_SMALL_INTEGER_MAX_VALUE (+( ((intptr_t)1)<<(CRANK_VM_SMALL_INTEGER_USABLE_BITS - 1) ) - 1)

#define CRANK_VM_CHARACTER_USABLE_BITS (CRANK_VM_OOP_BITS - CRANK_VM_OOP_TAG_CHARACTER_SHIFT)
#define CRANK_VM_CHARACTER_MAX_VALUE (+( ((intptr_t)1)<< CRANK_VM_SMALL_INTEGER_USABLE_BITS) - 1)

CRANK_VM_INLINE int
crankvm_oop_isPointer(crankvm_oop_t oop)
{
    return (oop & CRANK_VM_OOP_TAG_POINTER_MASK) == CRANK_VM_OOP_TAG_POINTER_VALUE;
}

CRANK_VM_INLINE int
crankvm_oop_isSmallInteger(crankvm_oop_t oop)
{
    return (oop & CRANK_VM_OOP_TAG_SMALL_INTEGER_MASK) == CRANK_VM_OOP_TAG_SMALL_INTEGER_VALUE;
}

CRANK_VM_INLINE int
crankvm_oop_isIntegerInSmallIntegerRange(intptr_t integer)
{
    return CRANK_VM_SMALL_INTEGER_MIN_VALUE <= integer && integer <= CRANK_VM_SMALL_INTEGER_MAX_VALUE;
}

CRANK_VM_INLINE int
crankvm_oop_isIntegerInCharacterRange(intptr_t integer)
{
    return 0 <= integer && integer <= CRANK_VM_CHARACTER_MAX_VALUE;
}

CRANK_VM_INLINE intptr_t
crankvm_oop_decodeSmallInteger(crankvm_soop_t oop)
{
    crankvm_assertAlways(crankvm_oop_isSmallInteger(oop));
    return oop >> CRANK_VM_OOP_TAG_SMALL_INTEGER_SHIFT;
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_oop_encodeSmallInteger(intptr_t integer)
{
    crankvm_assertAlways(crankvm_oop_isIntegerInSmallIntegerRange(integer));
    return (integer << CRANK_VM_OOP_TAG_SMALL_INTEGER_SHIFT) | CRANK_VM_OOP_TAG_SMALL_INTEGER_VALUE;
}

CRANK_VM_INLINE int
crankvm_oop_isCharacter(crankvm_oop_t oop)
{
    return (oop & CRANK_VM_OOP_TAG_CHARACTER_MASK) == CRANK_VM_OOP_TAG_CHARACTER_VALUE;
}

CRANK_VM_INLINE uintptr_t
crankvm_oop_decodeCharacter(crankvm_soop_t oop)
{
    crankvm_assertAlways(crankvm_oop_isCharacter(oop));
    return oop >> CRANK_VM_OOP_TAG_CHARACTER_SHIFT;
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_oop_encodeCharacter(uintptr_t value)
{
    return (value << CRANK_VM_OOP_TAG_CHARACTER_SHIFT) | CRANK_VM_OOP_TAG_CHARACTER_VALUE;
}

CRANK_VM_INLINE int
crankvm_oop_isSmallFloat(crankvm_oop_t oop)
{
    return (oop & CRANK_VM_OOP_TAG_SMALL_FLOAT_MASK) == CRANK_VM_OOP_TAG_SMALL_FLOAT_VALUE;
}

#if CRANK_VM_WORD_SIZE == 4

CRANK_VM_INLINE int
crankvm_oop_isFloatInSmallFloatRange(double value)
{
    (void)value;
    return 0;
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_oop_encodeSmallFloat(crankvm_oop_t oop)
{
    abort();
}

CRANK_VM_INLINE double
crankvm_oop_decodeSmallFloat(crankvm_oop_t oop)
{
    abort();
}

#else

CRANK_VM_INLINE int
crankvm_oop_isFloatInSmallFloatRange(double value)
{
    uint64_t valueIEEE754 = 0;
    memcpy(&valueIEEE754, &value, 8);

    uint64_t exponent = (valueIEEE754 >> 52) & 2047;
    return CRANK_VM_SMALL_FLOAT_EXPONENT_MIN <= exponent && exponent <= CRANK_VM_SMALL_FLOAT_EXPONENT_MAX;
}

CRANK_VM_INLINE double
crankvm_oop_decodeSmallFloat(crankvm_oop_t oop)
{
    crankvm_assertAlways(crankvm_oop_isSmallFloat(oop));

    // Shift out the tag.
    uint64_t decodedOop = oop >> 3;

    // Offset the exponent
    decodedOop += ((uint64_t)CRANK_VM_SMALL_FLOAT_EXPONENT_OFFSET << 53);

    // Rotate right the sign bit.
    decodedOop = (decodedOop >> 1) | (decodedOop << 62);

    // Cast back to float.
    double floatValue = 0.0f;
    memcpy(&floatValue, &decodedOop, 8);
    printf("Decoded float value: %f\n", floatValue);
    return floatValue;
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_oop_encodeSmallFloat(double value)
{
    crankvm_assertAlways(crankvm_oop_isFloatInSmallFloatRange(value));

    uint64_t valueIEEE754 = 0;
    memcpy(&valueIEEE754, &value, 8);

    // Rotate left the sign.
    crankvm_oop_t encodedOop = (valueIEEE754 << 1) | (valueIEEE754 >> 62);

    // Offset the exponent
    encodedOop -= ((uint64_t)CRANK_VM_SMALL_FLOAT_EXPONENT_OFFSET << 53);

    // Add the tag.
    encodedOop = (encodedOop << CRANK_VM_OOP_TAG_SMALL_FLOAT_SHIFT) | CRANK_VM_OOP_TAG_SMALL_FLOAT_VALUE;

    printf("Encoded small float %f -> %f\n", value, crankvm_oop_decodeSmallFloat(encodedOop));

    return encodedOop;
}

#endif

typedef enum crankvm_object_format_e
{
	CRANK_VM_OBJECT_FORMAT_EMPTY = 0,
	CRANK_VM_OBJECT_FORMAT_FIXED_SIZE = 1,
	CRANK_VM_OBJECT_FORMAT_VARIABLE_SIZE_NO_IVARS = 2,
	CRANK_VM_OBJECT_FORMAT_VARIABLE_SIZE_IVARS = 3,
	CRANK_VM_OBJECT_FORMAT_WEAK_VARIABLE_SIZE = 4,
	CRANK_VM_OBJECT_FORMAT_WEAK_FIXED_SIZE = 5,
    CRANK_VM_OBJECT_FORMAT_IMMEDIATE = 7,
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
} crankvm_object_format_t;

/***
 * Object header
 */
typedef union crankvm_object_header_u
{
/**
(Bigendian serialization)
MSB:	| 8: numSlots		| (on a byte boundary)

       | 2 bits				|	(msb,lsb = {isMarked,?})
       | 22: identityHash	| (on a word boundary)

       | 3 bits				|	(msb <-> lsb = {isGrey,isPinned,isRemembered}
       | 5: format			| (on a byte boundary)

       | 2 bits				|	(msb,lsb = {isImmutable,?})
       | 22: classIndex		| (on a word boundary) : LSB
*/
    uint8_t bytes[8];
    uint32_t words[2];
    uint64_t allData;
} crankvm_object_header_t;

#define CRANK_VM_IDENTITY_HASH_MASK ((1<<22) - 1)
#define CRANK_VM_CLASS_INDEX_MASK CRANK_VM_IDENTITY_HASH_MASK

#define CRANK_VM_AT_GET_BITS(x, bitShift, bitCount) ((x) >> bitShift) & ((1<<bitCount) - 1)

CRANK_VM_INLINE uint32_t
crankvm_object_header_getBitAtWord(uint32_t *word, uint32_t bitIndex)
{
    return CRANK_VM_AT_GET_BITS(*word, bitIndex, 1);
}

CRANK_VM_INLINE void
crankvm_object_header_setBitAtWord(uint32_t *word, uint32_t bitIndex, uint32_t value)
{
    uint32_t bit = 1<<bitIndex;
    uint32_t bitToSetOrClear = value ? bit : 0;
    *word = (*word & (~bit)) | bitToSetOrClear;
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_getRawSlotCount(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return header->bytes[7];
}

CRANK_VM_INLINE void
crankvm_object_header_setRawSlotCount(crankvm_object_header_t *header, uint32_t value)
{
    // TODO: Check the endianness.
    header->bytes[7] = value;
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_getRawSlotOverflowCount(crankvm_object_header_t *header)
{
#ifdef CRANK_VM_64_BITS
    uint64_t count = ((uint64_t*)header)[-1];
    return count & 0x00FFFFFFFFFFFFFFul;
#else
    return ((uint32_t*)header)[-1];
#endif
}

CRANK_VM_INLINE void
crankvm_object_header_setRawSlotOverflowCount(crankvm_object_header_t *header, size_t count)
{
#ifdef CRANK_VM_64_BITS
    ((uint64_t*)header)[-1] = count | (0xFFul<<56ul);
#else
    ((uint32_t*)header)[-1] = count;
    ((uint32_t*)header)[-2] = 0xFF000000;
#endif
}

CRANK_VM_INLINE size_t
crankvm_object_header_getSlotCount(crankvm_object_header_t *header)
{
    size_t count = crankvm_object_header_getRawSlotCount(header);
    if(count == 255)
        return crankvm_object_header_getRawSlotOverflowCount(header);
    return count;
}

CRANK_VM_INLINE void
crankvm_object_header_setSlotCount(crankvm_object_header_t *header, size_t count)
{
    if(count >= 255)
    {
        crankvm_object_header_setRawSlotCount(header, 255);
        crankvm_object_header_setRawSlotOverflowCount(header, count);
    }
    else
    {
        crankvm_object_header_setRawSlotCount(header, count);
    }
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_isImmutable(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return crankvm_object_header_getBitAtWord(&header->words[0], 23);
}

CRANK_VM_INLINE void
crankvm_object_header_setIsImmutable(crankvm_object_header_t *header, uint32_t value)
{
    // TODO: Check the endianness.
    return crankvm_object_header_setBitAtWord(&header->words[0], 23, value);
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_getIsGray(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return crankvm_object_header_getBitAtWord(&header->words[0], 31);
}

CRANK_VM_INLINE void
crankvm_object_header_setIsGray(crankvm_object_header_t *header, uint32_t value)
{
    // TODO: Check the endianness.
    crankvm_object_header_setBitAtWord(&header->words[0], 31, value);
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_getIsMarked(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return crankvm_object_header_getBitAtWord(&header->words[1], 23);
}

CRANK_VM_INLINE void
crankvm_object_header_setIsMarked(crankvm_object_header_t *header, uint32_t value)
{
    // TODO: Check the endianness.
    crankvm_object_header_setBitAtWord(&header->words[1], 23, value);
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_getIsRemembered(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return crankvm_object_header_getBitAtWord(&header->words[0], 29);
}

CRANK_VM_INLINE void
crankvm_object_header_setIsRemembered(crankvm_object_header_t *header, uint32_t value)
{
    // TODO: Check the endianness.
    crankvm_object_header_setBitAtWord(&header->words[0], 29, value);
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_getIsPinned(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return crankvm_object_header_getBitAtWord(&header->words[0], 30);
}

CRANK_VM_INLINE void
crankvm_object_header_setIsPinned(crankvm_object_header_t *header, uint32_t value)
{
    // TODO: Check the endianness.
    crankvm_object_header_setBitAtWord(&header->words[0], 30, value);
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_getIdentityHash(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return header->words[1] & CRANK_VM_IDENTITY_HASH_MASK;
}

CRANK_VM_INLINE void
crankvm_object_header_setIdentityHash(crankvm_object_header_t *header, uint32_t value)
{
    // TODO: Check the endianness.
    header->words[1] = (header->words[1] & (~CRANK_VM_IDENTITY_HASH_MASK)) | (value & CRANK_VM_IDENTITY_HASH_MASK);
}

CRANK_VM_INLINE crankvm_object_format_t
crankvm_object_header_getObjectFormat(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return header->bytes[3] & 31;
}

CRANK_VM_INLINE void
crankvm_object_header_setObjectFormat(crankvm_object_header_t *header, crankvm_object_format_t value)
{
    // TODO: Check the endianness.
    header->bytes[3] = (header->bytes[3] & (-32)) | (value & 31);
}

CRANK_VM_INLINE uint32_t
crankvm_object_header_getClassIndex(crankvm_object_header_t *header)
{
    // TODO: Check the endianness.
    return header->words[0] & CRANK_VM_CLASS_INDEX_MASK;
}

CRANK_VM_INLINE void
crankvm_object_header_setClassIndex(crankvm_object_header_t *header, uint32_t value)
{
    // TODO: Check the endianness.
    header->words[0] = (header->words[0] & (~CRANK_VM_CLASS_INDEX_MASK)) | (value & CRANK_VM_CLASS_INDEX_MASK);
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_object_header_getSlot(crankvm_object_header_t *header, size_t index)
{
    if(index >= crankvm_object_header_getSlotCount(header))
        return 0;

    crankvm_oop_t *slots = (crankvm_oop_t *)&header[1];
    return slots[index];
}

CRANK_VM_INLINE crankvm_object_format_t
crankvm_oop_getFormat(crankvm_oop_t object)
{
    if(!crankvm_oop_isPointer(object))
        return CRANK_VM_OBJECT_FORMAT_IMMEDIATE;
    return crankvm_object_header_getObjectFormat((crankvm_object_header_t*)object);
}

CRANK_VM_INLINE int
crankvm_oop_isCompiledCode(crankvm_oop_t object)
{
    crankvm_object_format_t format = crankvm_oop_getFormat(object);
    return CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD <= format && format <= CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_7;
}


CRANK_VM_INLINE int
crankvm_oop_isBytes(crankvm_oop_t object)
{
    crankvm_object_format_t format = crankvm_oop_getFormat(object);
    return CRANK_VM_OBJECT_FORMAT_INDEXABLE_8 <= format && CRANK_VM_OBJECT_FORMAT_INDEXABLE_8 <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_7;
}
/**
 * The header for a compiled code. This is encoded as a SmallInteger
 */
typedef struct crankvm_compiled_code_header_s
{
    unsigned int isAlternateBytecode; // Sign bit - 1 bit
    unsigned int numberOfLiterals;  // At: 0 - 15 bits
    unsigned int hasPrimitive; // At: 16 - 1 bit
    unsigned int largeFrameRequired;  // At: 17 - 1 bit
    unsigned int numberOfTemporaries; // At: 18 - 6 bits
    unsigned int numberOfArguments; // At: 24 - 4 bits
    unsigned int requiresCounters;  // At: 28 - 1 bit FIXME: Is this one correct?
    unsigned int flag; // At: 29 - 1 bit. Ignored by the VM
} crankvm_compiled_code_header_t;

CRANK_VM_INLINE crankvm_error_t crankvm_object_decodeCompiledCodeHeader(crankvm_compiled_code_header_t *result, crankvm_oop_t oop)
{
    if(!crankvm_oop_isSmallInteger(oop))
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    intptr_t decodedInteger = crankvm_oop_decodeSmallInteger(oop);

    result->isAlternateBytecode = (decodedInteger < 0) ? 1 : 0;
    result->numberOfLiterals = CRANK_VM_AT_GET_BITS(decodedInteger, 0, 15);
    result->hasPrimitive = CRANK_VM_AT_GET_BITS(decodedInteger, 16, 1);
    result->largeFrameRequired = CRANK_VM_AT_GET_BITS(decodedInteger, 17, 1);
    result->numberOfTemporaries = CRANK_VM_AT_GET_BITS(decodedInteger, 18, 6);
    result->numberOfArguments = CRANK_VM_AT_GET_BITS(decodedInteger, 24, 4);
    result->requiresCounters = CRANK_VM_AT_GET_BITS(decodedInteger, 28, 1);
    result->flag = CRANK_VM_AT_GET_BITS(decodedInteger, 29, 1);
    return CRANK_VM_OK;
};

LIB_CRANK_VM_EXPORT size_t crankvm_object_header_getSmalltalkSize(crankvm_object_header_t *header);

#define crankvm_string_printf_arg(x) ((int)crankvm_object_header_getSmalltalkSize((crankvm_object_header_t*)(x))), (char*)((crankvm_oop_t)(x) + 8)

typedef struct crankvm_context_s crankvm_context_t;

LIB_CRANK_VM_EXPORT uintptr_t crankvm_object_getIdentityHash(crankvm_context_t *context, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT uintptr_t crankvm_object_getBehaviorHash(crankvm_context_t *context, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_getClassWithIndex(crankvm_context_t *context, uint32_t classIndex, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_getClassWithPointerIndex(crankvm_context_t *context, uint32_t classIndex, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_getClass(crankvm_context_t *context, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT int crankvm_object_isClass(crankvm_context_t *context, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT int crankvm_object_isMetaclassInstance(crankvm_context_t *context, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT int crankvm_object_isClassWithName(crankvm_context_t *context, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_class_getNameOop(crankvm_context_t *context, crankvm_oop_t object);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_forInteger(crankvm_context_t *context, intptr_t integer);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_forInteger32(crankvm_context_t *context, int32_t integer);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_forUInteger32(crankvm_context_t *context, uint32_t integer);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_forInteger64(crankvm_context_t *context, int64_t integer);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_forUInteger64(crankvm_context_t *context, uint64_t integer);
#ifdef CRANK_VM_64_BITS
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_forInteger128(crankvm_context_t *context, __int128 integer);
#endif
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_forBoolean(crankvm_context_t *context, int boolean);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_object_forFloat(crankvm_context_t *context, double value);
LIB_CRANK_VM_EXPORT int crankvm_object_tryToDecodeFloat(crankvm_context_t *context, crankvm_oop_t object, double *resultValue);
LIB_CRANK_VM_EXPORT void crankvm_object_prettyPrintTo(crankvm_context_t *context, crankvm_oop_t object, FILE *output);

#endif //_CRANK_VM_OBJECT_MODEL_H_
