#include <crank-vm/objectmodel.h>
#include <assert.h>
#include "context-internal.h"

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

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_getClass(crankvm_context_t *context, crankvm_oop_t object)
{
    crankvm_oop_t tag = object & CRANK_VM_OOP_TAG_MASK;
    if(tag != 0)
        return context->roots.firstClassTablePage->classes[tag];

    // This could be a special case.
    uint32_t classIndex = crankvm_object_header_getClassIndex((crankvm_object_header_t *)object);
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
crankvm_object_forInteger(crankvm_context_t *context, intptr_t integer)
{
    if(crankvm_oop_isIntegerInSmallIntegerRange(integer))
        return crankvm_oop_encodeSmallInteger(integer);

    printf("TODO: Make large integer in crankvm_object_forInteger\n");
    abort();
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_object_forBoolean(crankvm_context_t *context, int boolean)
{
    return boolean ? crankvm_specialObject_true(context) : crankvm_specialObject_true(false);
}
