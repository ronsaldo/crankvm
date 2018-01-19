#include <crank-vm/special-objects.h>
#include <crank-vm/context.h>
#include "heap.h"
#include "context-internal.h"

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_CompiledCode_validate(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode)
{
    if(crankvm_object_isNilOrNull(context, compiledCode))
        return CRANK_VM_ERROR_NULL_POINTER;

    if(crankvm_oop_getFormat((crankvm_oop_t)compiledCode) < CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD)
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT size_t
crankvm_CompiledCode_getNumberOfLiterals(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode)
{
    crankvm_compiled_code_header_t header;
    crankvm_object_decodeCompiledCodeHeader(&header, compiledCode->codeHeader);

    return header.numberOfLiterals;
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_CompiledCode_getSelector(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode)
{
    crankvm_compiled_code_header_t header;
    crankvm_object_decodeCompiledCodeHeader(&header, compiledCode->codeHeader);

    size_t selectorLiteralIndex = header.numberOfLiterals - 2;
    crankvm_oop_t selectorOrAdditionalMethodState = compiledCode->literals[selectorLiteralIndex];
    if(crankvm_oop_isBytes(selectorOrAdditionalMethodState))
        return selectorOrAdditionalMethodState;

    crankvm_AdditionalMethodState_t *additionalState = (crankvm_AdditionalMethodState_t*)selectorOrAdditionalMethodState;
    return additionalState->selector;
}

LIB_CRANK_VM_EXPORT size_t
crankvm_CompiledCode_getFirstPC(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode)
{
    return crankvm_CompiledCode_getNumberOfLiterals(context, compiledCode) + sizeof(crankvm_oop_t) + 1;
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_MethodContext_validate(crankvm_context_t *context, crankvm_MethodContext_t *methodContext)
{
    if(crankvm_object_isNilOrNull(context, methodContext))
        return CRANK_VM_ERROR_NULL_POINTER;

    if(!crankvm_oop_isSmallInteger(methodContext->baseClass.pc) ||
        !crankvm_oop_isSmallInteger(methodContext->stackp))
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    return CRANK_VM_OK;
}


LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_MethodDictionary_atOrNil(crankvm_context_t *context, crankvm_MethodDictionary_t *methodDict, crankvm_oop_t keyObject)
{
    if(crankvm_object_isNil(context, methodDict->array))
        return crankvm_specialObject_nil(context);

    uintptr_t keyHash = crankvm_object_getIdentityHash(context, keyObject);

    size_t finish = crankvm_object_header_getSlotCount((crankvm_object_header_t*)methodDict->array);
    size_t start = keyHash % finish;

    //printf("Lookup key %p %08x start: %zu finish: %zu at method dict %p\n", (void*)keyObject, (int)keyHash, start, finish, methodDict);
    for(size_t i = start; i < finish; ++i)
    {
        if(methodDict->keys[i] == keyObject)
            return methodDict->array->slots[i];
    }

    for(size_t i = 0; i < start; ++i)
    {
        if(methodDict->keys[i] == keyObject)
            return methodDict->array->slots[i];
    }

    return crankvm_specialObject_nil(context);
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_Behavior_lookupSelector(crankvm_context_t *context, crankvm_Behavior_t *behavior, crankvm_oop_t selector)
{
    crankvm_Behavior_t *currentBehavior = behavior;
    for(; !crankvm_object_isNil(context, currentBehavior); currentBehavior = currentBehavior->superclass)
    {
        // Get the method dictionary.
        crankvm_MethodDictionary_t *methodDict = currentBehavior->methodDict;
        if(crankvm_object_isNil(context, methodDict))
            continue;

        //printf("Looking at class %.*s\n", crankvm_string_printf_arg(crankvm_Behavior_getName(context, currentBehavior)));

        // Lookup the selector in the method dictionary.
        crankvm_oop_t method = crankvm_MethodDictionary_atOrNil(context, methodDict, selector);
        if(!crankvm_oop_isNil(context, method))
            return method;
    }

    return crankvm_specialObject_nil(context);
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_Behavior_getName(crankvm_context_t *context, crankvm_Behavior_t *behavior)
{
    if(crankvm_object_isNil(context, behavior))
        return crankvm_specialObject_nil(context);

    // TODO: Find a way to detect whether this is a class or a metaclass.
    return ((crankvm_Class_t*)behavior)->name;
}

static inline crankvm_oop_t
crankvm_Behavior_initializeAllocateObject(crankvm_context_t *context, crankvm_Behavior_t *behavior, crankvm_object_header_t *object)
{
    // Set the object class index.
    crankvm_object_header_setClassIndex(object, crankvm_object_getIdentityHash(context, (crankvm_oop_t)behavior));

    return (crankvm_oop_t)object;
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_Behavior_basicNew(crankvm_context_t *context, crankvm_Behavior_t *behavior)
{
    crankvm_object_format_t format = crankvm_Behavior_getInstanceSpec(behavior);
    size_t fixedSize = crankvm_Behavior_getInstanceSize(behavior);
    crankvm_object_header_t *header = crankvm_heap_newObject(context, format, fixedSize, 0);
    if(!header)
        return crankvm_specialObject_nil(context);

    return crankvm_Behavior_initializeAllocateObject(context, behavior, header);
}

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_Behavior_basicNewWithVariable(crankvm_context_t *context, crankvm_Behavior_t *behavior, size_t variableSize)
{
    crankvm_object_format_t format = crankvm_Behavior_getInstanceSpec(behavior);
    size_t fixedSize = crankvm_Behavior_getInstanceSize(behavior);
    crankvm_object_header_t *header = crankvm_heap_newObject(context, format, fixedSize, variableSize);
    if(!header)
        return crankvm_specialObject_nil(context);

    return crankvm_Behavior_initializeAllocateObject(context, behavior, header);
}

LIB_CRANK_VM_EXPORT crankvm_MethodContext_t*
crankvm_MethodContext_create(crankvm_context_t *context, int largeFrame)
{
    crankvm_Behavior_t *classMethodContext = context->roots.specialObjectsArray->classMethodContext;
    if(largeFrame)
        return (crankvm_MethodContext_t*)crankvm_Behavior_basicNewWithVariable(context, classMethodContext, CRANK_VM_METHOD_CONTEXT_LARGE_FRAME_SIZE);
    else
        return (crankvm_MethodContext_t*)crankvm_Behavior_basicNewWithVariable(context, classMethodContext, CRANK_VM_METHOD_CONTEXT_SMALL_FRAME_SIZE);
}
