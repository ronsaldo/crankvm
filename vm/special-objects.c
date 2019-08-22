#include <crank-vm/special-objects.h>
#include <crank-vm/context.h>
#include <crank-vm/system-primitive-number.h>
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

LIB_CRANK_VM_EXPORT crankvm_oop_t
crankvm_CompiledCode_getClass(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode)
{
    crankvm_compiled_code_header_t header;
    crankvm_object_decodeCompiledCodeHeader(&header, compiledCode->codeHeader);

    size_t classBindingLiteralIndex = header.numberOfLiterals - 1;
    crankvm_oop_t classBinding = compiledCode->literals[classBindingLiteralIndex];
    return ((crankvm_Association_t*)classBinding)->value;
}

LIB_CRANK_VM_EXPORT size_t
crankvm_CompiledCode_getFirstPC(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode)
{
    return crankvm_CompiledCode_getNumberOfLiterals(context, compiledCode) + sizeof(crankvm_oop_t) + 1;
}

// MethodDictionary
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

// Behavior
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

// Array
LIB_CRANK_VM_EXPORT crankvm_Array_t *
crankvm_Array_create(crankvm_context_t *context, size_t variableSize)
{
    crankvm_Behavior_t *classArray = context->roots.specialObjectsArray->classArray;
    return (crankvm_Array_t*)crankvm_Behavior_basicNewWithVariable(context, classArray, variableSize);
}

// BlockClosure
LIB_CRANK_VM_EXPORT crankvm_BlockClosure_t *
crankvm_BlockClosure_create(crankvm_context_t *context, uintptr_t argumentCount, size_t copiedValueCount)
{
    crankvm_Behavior_t *classBlockClosure = context->roots.specialObjectsArray->classBlockClosure;
    crankvm_BlockClosure_t *result = (crankvm_BlockClosure_t*)crankvm_Behavior_basicNewWithVariable(context, classBlockClosure, copiedValueCount);
    result->numArgs = crankvm_oop_encodeSmallInteger(argumentCount);
    return result;
}

// MethodContext
LIB_CRANK_VM_EXPORT crankvm_MethodContext_t*
crankvm_MethodContext_create(crankvm_context_t *context, int largeFrame)
{
    crankvm_Behavior_t *classMethodContext = context->roots.specialObjectsArray->classMethodContext;
    if(largeFrame)
        return (crankvm_MethodContext_t*)crankvm_Behavior_basicNewWithVariable(context, classMethodContext, CRANK_VM_METHOD_CONTEXT_LARGE_FRAME_SIZE);
    else
        return (crankvm_MethodContext_t*)crankvm_Behavior_basicNewWithVariable(context, classMethodContext, CRANK_VM_METHOD_CONTEXT_SMALL_FRAME_SIZE);
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_CompiledCode_checkActivationWithArgumentCount(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode, int argumentCount, crankvm_compiled_code_header_t *parsedCompiledCodeHeader, int *parsedPrimitiveNumber, uintptr_t *parsedInitialPC)
{
    crankvm_compiled_code_header_t calledHeader;
    crankvm_error_t error = crankvm_specialObject_getCompiledCodeHeader(&calledHeader, compiledCode);
    if(error)
        return error;

    // Check the number of arguments
    if(calledHeader.numberOfArguments != argumentCount)
        return CRANK_VM_ERROR_CALLED_METHOD_ARGUMENT_MISMATCH;

    // Compute the initial pc
    uintptr_t initialPC = (calledHeader.numberOfLiterals + 1) *sizeof(crankvm_oop_t) + 1;
    size_t compiledMethodSize = crankvm_object_header_getSmalltalkSize(&compiledCode->baseClass.objectHeader);
    if(initialPC >= compiledMethodSize)
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    if(parsedCompiledCodeHeader)
        *parsedCompiledCodeHeader = calledHeader;

    if(parsedPrimitiveNumber)
    {
        if(calledHeader.hasPrimitive)
        {
            uint8_t *methodInstructions = (uint8_t*)(((crankvm_oop_t)compiledCode) + sizeof(crankvm_object_header_t));
            *parsedPrimitiveNumber = methodInstructions[initialPC] | (methodInstructions[initialPC + 1] << 8);
        }
        else
        {
            *parsedPrimitiveNumber = -1;
        }
    }

    if(parsedInitialPC)
        *parsedInitialPC = initialPC;

    return CRANK_VM_OK;
}


LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_MethodContext_createCompiledMethodActivationContext(crankvm_context_t *vmContext, crankvm_MethodContext_t **result, crankvm_CompiledCode_t *compiledCode, crankvm_oop_t receiver, int argumentCount, crankvm_oop_t *arguments, crankvm_compiled_code_header_t *calledHeader, uintptr_t initialPC)
{
    // Create the new context.
    crankvm_MethodContext_t *newContext = crankvm_MethodContext_create(vmContext, calledHeader->largeFrameRequired);

    // Setup the activated method context.
    newContext->baseClass.pc = crankvm_oop_encodeSmallInteger(initialPC);
    newContext->method = (crankvm_oop_t)compiledCode;

    // Pop the arguments into the new context.
    if(arguments)
    {
        for(int i = 0; i < argumentCount; ++i)
            newContext->stackSlots[i] = arguments[i];
    }

    // Pop the receiver into the new context.
    newContext->receiver = receiver;
    newContext->stackp = crankvm_oop_encodeSmallInteger(argumentCount + calledHeader->numberOfTemporaries);

    *result = newContext;
    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_MethodContext_createBlockClosureActivationContext(crankvm_context_t *vmContext, crankvm_MethodContext_t **result, crankvm_BlockClosure_t *blockClosure, int argumentCount, crankvm_oop_t *arguments)
{
    // Make sure the number of arguments, and the initial pc are small integers.
    if(!crankvm_oop_isSmallInteger(blockClosure->numArgs) ||
       !crankvm_oop_isSmallInteger(blockClosure->startpc))
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    // Check the expected argument count.
    intptr_t expectedArgumentCount = crankvm_oop_decodeSmallInteger(blockClosure->numArgs);
    intptr_t rawStartPC = crankvm_oop_decodeSmallInteger(blockClosure->startpc);
    if(expectedArgumentCount != argumentCount || rawStartPC <= 0)
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    // Get the compiled method, and decode its header.
    crankvm_CompiledCode_t *compiledMethod = (crankvm_CompiledCode_t*)blockClosure->outerContext->method;
    crankvm_compiled_code_header_t compiledMethodHeader;
    crankvm_error_t error = crankvm_specialObject_getCompiledCodeHeader(&compiledMethodHeader, compiledMethod);
    if(error)
        return error;

    // Check whether the initial pc is on bounds.
    size_t compiledMethodSize = crankvm_object_header_getSmalltalkSize(&compiledMethod->baseClass.objectHeader);
    uintptr_t methodInitialPC = (compiledMethodHeader.numberOfLiterals + 1) *sizeof(crankvm_oop_t) + 1;
    uintptr_t startPC = (uintptr_t)rawStartPC;
    if(startPC < methodInitialPC || methodInitialPC >= compiledMethodSize)
        return CRANK_VM_ERROR_OUT_OF_BOUNDS;

    // Create the block closure context.
    crankvm_MethodContext_t *activationContext = crankvm_MethodContext_create(vmContext, compiledMethodHeader.largeFrameRequired);
    if(!activationContext)
        return CRANK_VM_ERROR_OUT_OF_MEMORY;

    // Setup the closure activation context.
    activationContext->closureOrNil = (crankvm_oop_t)blockClosure;
    activationContext->method = (crankvm_oop_t)compiledMethod;
    activationContext->baseClass.pc = blockClosure->startpc;

    // Copy the passed arguments.
    if(arguments)
    {
        for(size_t i = 0; i < argumentCount; ++i)
            activationContext->stackSlots[i] = arguments[i];
    }

    // Copy the closure captured temporaries.
    size_t capturedTemporaryCount = crankvm_object_header_getSmalltalkSize(&blockClosure->baseClass.objectHeader) - CRANK_VM_BlockClosure_InstanceFixedSize;
    for(size_t i = 0; i < capturedTemporaryCount; ++i)
        activationContext->stackSlots[argumentCount + i] = blockClosure->capturedTemporaries[i];

    // Set the receiver and the stack pointer.
    activationContext->receiver = blockClosure->outerContext->receiver;
    activationContext->stackp = crankvm_oop_encodeSmallInteger(argumentCount + capturedTemporaryCount);

    // Return
    *result = activationContext;
    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_MethodContext_createFullBlockClosureActivationContext(crankvm_context_t *vmContext, crankvm_MethodContext_t **result, crankvm_BlockClosure_t *blockClosure, int argumentCount, crankvm_oop_t *arguments)
{
    return CRANK_VM_ERROR_UNIMPLEMENTED;
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_MethodContext_createCompiledCodeMethodActivationContext(crankvm_context_t *vmContext, crankvm_MethodContext_t **result, crankvm_CompiledCode_t *compiledCode, crankvm_oop_t receiver, int argumentCount, crankvm_oop_t *arguments)
{
    // Get the compiled method context.
    crankvm_compiled_code_header_t calledHeader;
    uintptr_t initialPC;
    int parsedPrimitiveNumber;
    crankvm_error_t error = crankvm_CompiledCode_checkActivationWithArgumentCount(vmContext, compiledCode, argumentCount, &calledHeader, &parsedPrimitiveNumber, &initialPC);
    if(error)
        return error;

    if(calledHeader.hasPrimitive)
    {
        switch(parsedPrimitiveNumber)
        {
        case CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS0:
        case CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS1:
        case CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS2:
        case CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS3:
        case CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS4:
            if(argumentCount == (parsedPrimitiveNumber - CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS0))
                return crankvm_MethodContext_createBlockClosureActivationContext(vmContext, result, (crankvm_BlockClosure_t*)receiver, argumentCount, arguments);
            break;
        case CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_WITH_ARGUMENTS:
            printf("TODO: Create block closure activation with arguments context\n");
            break;
        case CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_FULL_BLOCK_VALUE:
            printf("TODO: Create full block closure activation context\n");
            break;
        case CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_FULL_BLOCK_VALUE_WITH_ARGUMENTS:
            printf("TODO: Create full block closure activation with arguments context\n");
            break;
        default:
            /* The normal primitive activation handling logic takes care of it. */
            break;
        }
    }

    return crankvm_MethodContext_createCompiledMethodActivationContext(vmContext, result, compiledCode, receiver, argumentCount, arguments, &calledHeader, initialPC);
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_MethodContext_createObjectMessageSendActivationContext(crankvm_context_t *context, crankvm_MethodContext_t **result, crankvm_oop_t receiver, crankvm_oop_t selector, int argumentCount, crankvm_oop_t *arguments)
{
    // Get the receiver class.
    crankvm_oop_t receiverClass = crankvm_object_getClass(context, receiver);
    if(crankvm_oop_isNil(context, receiverClass))
        return CRANK_VM_ERROR_RECEIVER_CLASS_NIL;

    // Lookup the selector
    crankvm_oop_t methodOop = crankvm_Behavior_lookupSelector(context, (crankvm_Behavior_t*)receiverClass, selector);
    if(crankvm_oop_isNil(context, methodOop))
    {
        printf("TODO: Send doesNotUnderstand:\n");
        abort();
    }

    // TODO: Support calling something that is not a compiled code.
    if(!crankvm_oop_isCompiledCode(methodOop))
    {
        printf("TODO: Send to non-compiled method\n");
        abort();
    }

    crankvm_CompiledCode_t *compiledCode = (crankvm_CompiledCode_t*)methodOop;
    return crankvm_MethodContext_createCompiledCodeMethodActivationContext(context, result, compiledCode, receiver, argumentCount, arguments);
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
