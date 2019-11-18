#include <crank-vm/objectmodel.h>
#include <assert.h>
#include "context-internal.h"
#include <string.h>

LIB_CRANK_VM_EXPORT size_t
crankvm_object_header_getSmalltalkSize(crankvm_object_header_t *header)
{
    if(!header)
        return 0;

    size_t slotCount = crankvm_object_header_getSlotCount(header);
    crankvm_object_format_t format = crankvm_oop_getFormat((crankvm_oop_t)header);
    if(format < CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
    {
        return slotCount;
    }
    else if(format == CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
    {
        if(sizeof(uint64_t) > sizeof(crankvm_oop_t))
            return slotCount / 2;
        return slotCount;
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1)
    {
        return slotCount * (sizeof(crankvm_oop_t) / 4) - (format - CRANK_VM_OBJECT_FORMAT_INDEXABLE_32);
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3)
    {
        return slotCount * (sizeof(crankvm_oop_t) / 2) - (format - CRANK_VM_OBJECT_FORMAT_INDEXABLE_16);
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_7)
    {
        return slotCount * sizeof(crankvm_oop_t) - (format - CRANK_VM_OBJECT_FORMAT_INDEXABLE_8);
    }
    else //if(format <= CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_7)
    {
        return slotCount * sizeof(crankvm_oop_t) - (format - CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD);
    }
}

LIB_CRANK_VM_EXPORT int
crankvm_object_isNil(crankvm_context_t *context, void *pointer)
{
    return !context || !context->roots.specialObjectsArray || ((crankvm_oop_t)pointer == context->roots.specialObjectsArray->nilObject);
}

static inline uintptr_t
crankvm_context_newIdentityHash(crankvm_context_t *context)
{
    return context->lastIdentityHash = (context->lastIdentityHash*1103515245 + 12345) % 0x7fffffff;
}

LIB_CRANK_VM_EXPORT uintptr_t
crankvm_object_getIdentityHash(crankvm_context_t *context, crankvm_oop_t oop)
{
    crankvm_oop_t tag = oop & CRANK_VM_OOP_TAG_MASK;
    if(tag != 0)
    {
        if(crankvm_oop_isSmallInteger(oop))
            return crankvm_oop_decodeSmallInteger(oop);
        if(crankvm_oop_isCharacter(oop))
            return crankvm_oop_decodeCharacter(oop);
        if(crankvm_oop_isSmallFloat(oop))
            return oop >> CRANK_VM_OOP_TAG_BITS;
        return 0;
    }

    uintptr_t rawHash = crankvm_object_header_getIdentityHash((crankvm_object_header_t*) oop);
    if(rawHash == 0)
    {
        rawHash = crankvm_context_newIdentityHash(context);
        crankvm_object_header_setIdentityHash((crankvm_object_header_t*)oop, rawHash);
    }

    return rawHash;
}


LIB_CRANK_VM_EXPORT uintptr_t
crankvm_object_getBehaviorHash(crankvm_context_t *context, crankvm_oop_t oop)
{
    if(!crankvm_oop_isPointer(oop))
        return 0;

    uintptr_t rawHash = crankvm_object_header_getIdentityHash((crankvm_object_header_t*) oop);
    if(rawHash == 0)
    {
        printf("TODO: Insert behavior in class table\n");
        abort();
    }

    return rawHash;
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_getClassWithPointerIndex(crankvm_context_t *context, uint32_t classIndex, crankvm_oop_t object)
{
    if(classIndex <= CRANKVM_CLASS_INDEX_PUN_LAST)
    {
        if(classIndex == CRANKVM_CLASS_INDEX_PUN_IS_ITSELF_CLASS)
            return object;
        if(classIndex == CRANKVM_CLASS_INDEX_PUN_FORWARDED)
            return crankvm_specialObject_nil(context);
    }
    assert(classIndex >= CRANKVM_CLASS_INDEX_PUN_FIRST);

    // Fetch the page
    uint32_t pageIndex = classIndex >> CRANK_VM_CLASS_TABLE_PAGE_BITS;
    if(pageIndex >= context->numberOfClassTablePages)
        return crankvm_specialObject_nil(context);

    crankvm_ClassTablePage_t *page = (crankvm_ClassTablePage_t*)context->roots.hiddenRootsObject->classTablePages[pageIndex];
    if(crankvm_object_isNil(context, page))
        return crankvm_specialObject_nil(context);

    // Fetch the page element
    uint32_t pageElementIndex = classIndex & CRANK_VM_CLASS_TABLE_PAGE_MASK;
    return page->classes[pageElementIndex];
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_getClassWithIndex(crankvm_context_t *context, uint32_t classIndex, crankvm_oop_t object)
{
    if(classIndex <= CRANKVM_CLASS_INDEX_PUN_LAST)
        return context->roots.firstClassTablePage->classes[classIndex];
    return crankvm_object_getClassWithPointerIndex(context, classIndex,object);
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_getClass(crankvm_context_t *context, crankvm_oop_t object)
{
    crankvm_oop_t tag = object & CRANK_VM_OOP_TAG_MASK;
    if(tag != 0)
        return context->roots.firstClassTablePage->classes[tag];

    // This could be a special case.
    uint32_t classIndex = crankvm_object_header_getClassIndex((crankvm_object_header_t *)object);
    return crankvm_object_getClassWithIndex(context, classIndex, object);
}

LIB_CRANK_VM_EXPORT int
crankvm_object_isClass(crankvm_context_t *context, crankvm_oop_t object)
{
    if(!crankvm_oop_isPointer(object))
        return 0;

    uint32_t classIndex = crankvm_object_header_getIdentityHash((crankvm_object_header_t *)object);
    return crankvm_object_getClassWithIndex(context, classIndex, object) == object;
}

LIB_CRANK_VM_EXPORT int
crankvm_object_isMetaclassInstance(crankvm_context_t *context, crankvm_oop_t object)
{
    if(!crankvm_oop_isPointer(object))
        return 0;

    crankvm_object_header_t *header = (crankvm_object_header_t*)object;
    if(crankvm_object_header_getSlotCount(header) < crankvm_Metaclass_instSize)
        return 0;

    if(crankvm_object_header_getObjectFormat(header) > CRANK_VM_OBJECT_FORMAT_VARIABLE_SIZE_IVARS)
        return 0;

    if(!crankvm_object_isClass(context, object))
        return 0;

    crankvm_Metaclass_t *metaClass = (crankvm_Metaclass_t*)object;
    crankvm_oop_t thisClassOop = (crankvm_oop_t)metaClass->thisClass;
    return crankvm_object_getClass(context, thisClassOop) == object;
}

LIB_CRANK_VM_EXPORT int
crankvm_object_isClassWithName(crankvm_context_t *context, crankvm_oop_t object)
{
    return crankvm_object_isMetaclassInstance(context, crankvm_object_getClass(context, object));
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_class_getNameOop(crankvm_context_t *context, crankvm_oop_t classOop)
{
    if(crankvm_oop_isNil(context, classOop))
        return classOop;

    if(crankvm_object_isMetaclassInstance(context, classOop))
    {
        return ((crankvm_Metaclass_t*)classOop)->thisClass->name;
    }
    else if(crankvm_object_isClassWithName(context, classOop))
    {
        return ((crankvm_Class_t*)classOop)->name;
    }
    else
    {
        return crankvm_specialObject_nil(context);
    }
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forInteger(crankvm_context_t *context, intptr_t integer)
{
    if(crankvm_oop_isIntegerInSmallIntegerRange(integer))
        return crankvm_oop_encodeSmallInteger(integer);

    int positive = integer >= 0;
    if(!positive)
        integer = -integer;
    return crankvm_LargeInteger_encodeUnormalizedValue(context, positive, sizeof(intptr_t), (uint8_t*)&integer);
}

#if CRANK_VM_WORD_SIZE == 4
LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forInteger32(crankvm_context_t *context, int32_t integer)
{
    if(crankvm_oop_isIntegerInSmallIntegerRange(integer))
        return crankvm_oop_encodeSmallInteger(integer);

    int positive = integer >= 0;
    if(!positive)
        integer = -integer;
    return crankvm_LargeInteger_encodeUnormalizedValue(context, positive, 4, (uint8_t*)&integer);
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forUInteger32(crankvm_context_t *context, uint32_t integer)
{
    if(integer <= CRANK_VM_SMALL_INTEGER_MAX_VALUE)
        return crankvm_oop_encodeSmallInteger(integer);

    return crankvm_LargeInteger_encodeUnormalizedValue(context, 1, 4, (uint8_t*)&integer);
}
#else
LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forInteger32(crankvm_context_t *context, int32_t integer)
{
    return crankvm_oop_encodeSmallInteger(integer);
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forUInteger32(crankvm_context_t *context, uint32_t integer)
{
    return crankvm_oop_encodeSmallInteger(integer);
}
#endif


LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forInteger64(crankvm_context_t *context, int64_t integer)
{
    if(CRANK_VM_SMALL_INTEGER_MIN_VALUE <= integer && integer <= CRANK_VM_SMALL_INTEGER_MAX_VALUE)
        return crankvm_oop_encodeSmallInteger(integer);

    int positive = integer >= 0;
    if(!positive)
        integer = -integer;
    return crankvm_LargeInteger_encodeUnormalizedValue(context, positive, 8, (uint8_t*)&integer);
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forUInteger64(crankvm_context_t *context, uint64_t integer)
{
    if(integer <= CRANK_VM_SMALL_INTEGER_MAX_VALUE)
        return crankvm_oop_encodeSmallInteger(integer);

    return crankvm_LargeInteger_encodeUnormalizedValue(context, 1, 8, (uint8_t*)&integer);
}

#ifdef CRANK_VM_64_BITS
LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forInteger128(crankvm_context_t *context, __int128 integer)
{
    if(CRANK_VM_SMALL_INTEGER_MIN_VALUE <= integer && integer <= CRANK_VM_SMALL_INTEGER_MAX_VALUE)
        return crankvm_oop_encodeSmallInteger(integer);

    int positive = integer >= 0;
    if(!positive)
        integer = -integer;

    return crankvm_LargeInteger_encodeUnormalizedValue(context, positive, 16, (uint8_t*)&integer);
}

#endif

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forBoolean(crankvm_context_t *context, int boolean)
{
    return boolean ? crankvm_specialObject_true(context) : crankvm_specialObject_false(context);
}

LIB_CRANK_VM_EXPORT void
crankvm_object_prettyPrintTo(crankvm_context_t *context, crankvm_oop_t object, FILE *output)
{
    if(!crankvm_oop_isPointer(object))
    {
        if(crankvm_oop_isSmallInteger(object))
        {
#if CRANK_VM_WORD_SIZE == 4
            fprintf(output, "(SmallInteger)%d", crankvm_oop_decodeSmallInteger(object));
#else
#    ifdef _WIN32
            fprintf(output, "(SmallInteger)%lld", crankvm_oop_decodeSmallInteger(object));
#    else
            fprintf(output, "(SmallInteger)%ld", crankvm_oop_decodeSmallInteger(object));
#    endif
#endif
        }
        else if(crankvm_oop_isSmallFloat(object))
        {
            fprintf(output, "(SmallFloat)%f", crankvm_oop_decodeSmallFloat(object));
        }
        else if(crankvm_oop_isCharacter(object))
        {
            fprintf(output, "(Character)%d", (int)crankvm_oop_decodeCharacter(object));
        }
        else
        {
            fprintf(output, "(Unsupported immediate)0x%p", (void*)object);
        }

        return;
    }

    // Support some classes
    crankvm_Behavior_t *class = (crankvm_Behavior_t*)crankvm_object_getClass(context, object);
    crankvm_special_object_array_t *specialObjects = context->roots.specialObjectsArray;
    if(class == specialObjects->classByteString)
    {
        fprintf(output, "(ByteString)'%.*s'", crankvm_string_printf_arg(object));
    }
    else if(class == (crankvm_Behavior_t*)context->roots.byteSymbolClassOop)
    {
        fprintf(output, "(ByteSymbol)#'%.*s'", crankvm_string_printf_arg(object));
    }
    else
    {
        fprintf(output, "(Oop)0x%p", (void*)object);
    }
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forFloat(crankvm_context_t *context, double value)
{
    if(crankvm_oop_isFloatInSmallFloatRange(value))
        return crankvm_oop_encodeSmallFloat(value);
    
    crankvm_Float_t *boxedFloat = (crankvm_Float_t*)crankvm_Behavior_basicNewWithVariable(context, context->roots.specialObjectsArray->classFloat, 2);
    boxedFloat->value = value;
    return (crankvm_oop_t)boxedFloat;
}

LIB_CRANK_VM_EXPORT int
crankvm_object_tryToDecodeFloat(crankvm_context_t *context, crankvm_oop_t object, double *resultValue)
{
    if(crankvm_oop_isSmallFloat(object))
    {
        *resultValue = crankvm_oop_decodeSmallFloat(object);
        return 1;
    }

    if(!crankvm_oop_isPointer(object))
        return 0;

    if(crankvm_object_getClass(context, object) != (crankvm_oop_t)context->roots.specialObjectsArray->classFloat)
        return 0;

    *resultValue = ((crankvm_Float_t*)object)->value;
    return 1;
}
