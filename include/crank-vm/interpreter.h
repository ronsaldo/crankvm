#ifndef CRANK_VM_INTERPRETER_H
#define CRANK_VM_INTERPRETER_H

#include <crank-vm/objectmodel.h>
#include <crank-vm/error.h>
#include <crank-vm/special-objects.h>
#include <crank-vm/context.h>
#include <string.h>

typedef struct crankvm_interpreter_state_s crankvm_interpreter_state_t;

#define CRANK_UNIMPLEMENTED() \
do { \
    fflush(stdout); \
    fprintf(stderr, "Unimplemented function %s in %s:%d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
    abort(); \
} while(0)

// Interpreter accessors.
LIB_CRANK_VM_EXPORT crankvm_context_t *crankvm_interpreter_getContext(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_getReceiver(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_MethodContext_t *crankvm_interpreter_getMethodContext(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_CompiledCode_t *crankvm_interpreter_getCompiledCode(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_MethodContext_t *crankvm_interpreter_createMethodContextForMessageSendWithLookupClass(crankvm_interpreter_state_t *self, crankvm_oop_t receiver, crankvm_oop_t selector, crankvm_oop_t lookupClass, size_t argumentCount, crankvm_oop_t *arguments);
LIB_CRANK_VM_EXPORT crankvm_MethodContext_t *crankvm_interpreter_createMethodContextForMessageSend(crankvm_interpreter_state_t *self, crankvm_oop_t receiver, crankvm_oop_t selector, size_t argumentCount, crankvm_oop_t *arguments);

// Oop stack access
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_pushOop(crankvm_interpreter_state_t *self, crankvm_oop_t oop);
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_checkSizeToPop(crankvm_interpreter_state_t *self, intptr_t size);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_popOop(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_stackOopAt(crankvm_interpreter_state_t *self, intptr_t size);

LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_checkReceiverSlotIndex(crankvm_interpreter_state_t *self, size_t index);
LIB_CRANK_VM_EXPORT crankvm_oop_t *crankvm_interpreter_getReceiverSlots(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_getReceiverSlot(crankvm_interpreter_state_t *self, size_t index);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_setReceiverSlot(crankvm_interpreter_state_t *self, size_t index, crankvm_oop_t value);

LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_checkTemporaryIndex(crankvm_interpreter_state_t *self, size_t index);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_getTemporary(crankvm_interpreter_state_t *self, size_t index);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_setTemporary(crankvm_interpreter_state_t *self, size_t index, crankvm_oop_t value);

LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_checkLiteralIndex(crankvm_interpreter_state_t *self, size_t index);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_getLiteral(crankvm_interpreter_state_t *self, size_t index);

typedef struct crankvm_primitive_context_s
{
    crankvm_context_t *context;
    crankvm_interpreter_state_t *interpreter;
    crankvm_primitive_error_code_t error;

    uint32_t argumentCount;
    struct
    {
        crankvm_oop_t *arguments;
        crankvm_oop_t receiver;
        crankvm_oop_t result;
        crankvm_MethodContext_t *primitiveMethodContext;
    } roots;
} crankvm_primitive_context_t;

typedef void (*crankvm_primitive_function_t) (crankvm_primitive_context_t *primitiveContext);

typedef struct crankvm_plugin_primitive_s
{
    const char *name;
    crankvm_primitive_function_t function;
} crankvm_plugin_primitive_t;

typedef struct crankvm_plugin_s
{
    const char *name;
    const crankvm_plugin_primitive_t primitives[];
} crankvm_plugin_t;

CRANK_VM_INLINE void
crankvm_primitive_success(crankvm_primitive_context_t *primitiveContext)
{
    primitiveContext->error = CRANK_VM_PRIMITIVE_SUCCESS;
}

CRANK_VM_INLINE void
crankvm_primitive_successWithResult(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t result)
{
    primitiveContext->error = CRANK_VM_PRIMITIVE_SUCCESS;
    primitiveContext->roots.result = result;
}

CRANK_VM_INLINE void
crankvm_primitive_failWithCode(crankvm_primitive_context_t *primitiveContext, crankvm_primitive_error_code_t code)
{
    if(primitiveContext->error == CRANK_VM_PRIMITIVE_SUCCESS)
        primitiveContext->error = code;
}

CRANK_VM_INLINE void
crankvm_primitive_failWithVMErrorCode(crankvm_primitive_context_t *primitiveContext, crankvm_primitive_error_code_t code)
{
    crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR);
}

CRANK_VM_INLINE void
crankvm_primitive_fail(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR);
}

CRANK_VM_INLINE uint32_t
crankvm_primitive_getArgumentCount(crankvm_primitive_context_t *primitiveContext)
{
    return primitiveContext->argumentCount;
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_primitive_getArgument(crankvm_primitive_context_t *primitiveContext, size_t index)
{
    if(index >= primitiveContext->argumentCount)
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_NUMBER_OF_ARGUMENTS);
        return 0;
    }
    return primitiveContext->roots.arguments[index];
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_primitive_getReceiver(crankvm_primitive_context_t *primitiveContext)
{
    return primitiveContext->roots.receiver;
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_primitive_getStackAt(crankvm_primitive_context_t *primitiveContext, size_t index)
{
    if(index > primitiveContext->argumentCount)
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_NUMBER_OF_ARGUMENTS);
        return 0;
    }
    else if(index == primitiveContext->argumentCount)
    {
        return primitiveContext->roots.receiver;
    }

    return primitiveContext->roots.arguments[primitiveContext->argumentCount - index - 1];
}

CRANK_VM_INLINE int
crankvm_primitive_hasFailed(crankvm_primitive_context_t *primitiveContext)
{
    return primitiveContext->error != 0;
}

CRANK_VM_INLINE intptr_t
crankvm_primitive_getSmallIntegerValue(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t oop)
{
    if(crankvm_primitive_hasFailed(primitiveContext))
        return 0;

    if(!crankvm_oop_isSmallInteger(oop))
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);
        return 0;
    }
    return crankvm_oop_decodeSmallInteger(oop);
}

CRANK_VM_INLINE double
crankvm_primitive_getNumberAsFloatValue(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t oop)
{
    if(crankvm_primitive_hasFailed(primitiveContext))
        return 0.0;

    if(crankvm_oop_isSmallFloat(oop))
    {
        return crankvm_oop_decodeSmallFloat(oop);
    }
    else if(crankvm_oop_isSmallInteger(oop))
    {
        return (double)crankvm_oop_decodeSmallInteger(oop);
    }
    else
    {
        double decodedFloat = 0.0f;
        if(!crankvm_object_tryToDecodeFloat(primitiveContext->context, oop, &decodedFloat))
        {
            crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);
            return 0;
        }
        return decodedFloat;
    }
}

CRANK_VM_INLINE intptr_t
crankvm_primitive_getCharacterValue(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t oop)
{
    if(crankvm_primitive_hasFailed(primitiveContext))
        return 0;

    if(!crankvm_oop_isCharacter(oop))
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);
        return 0;
    }

    return crankvm_oop_decodeCharacter(oop);
}

CRANK_VM_INLINE size_t
crankvm_primitive_getSizeValue(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t oop)
{
    intptr_t value = crankvm_primitive_getSmallIntegerValue(primitiveContext, oop);
    if(value < 0)
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);
        return 0;
    }

    return value;
}

CRANK_VM_INLINE uint8_t *
crankvm_primitive_getBytesPointer(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t oop)
{
    if(!crankvm_oop_isPointer(oop))
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);
        return 0;
    }

    // TODO: Validate the object format.
    return (uint8_t *)(oop + sizeof(crankvm_object_header_t));
}

CRANK_VM_INLINE void
crankvm_primitive_returnSmallInteger(crankvm_primitive_context_t *primitiveContext, intptr_t integer)
{
    if(!crankvm_oop_isIntegerInSmallIntegerRange(integer))
        return crankvm_primitive_fail(primitiveContext);
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_oop_encodeSmallInteger(integer));
}

CRANK_VM_INLINE void
crankvm_primitive_returnInteger(crankvm_primitive_context_t *primitiveContext, intptr_t integer)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forInteger(primitiveContext->context, integer));
}

CRANK_VM_INLINE void
crankvm_primitive_returnInteger32(crankvm_primitive_context_t *primitiveContext, int32_t integer)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forInteger32(primitiveContext->context, integer));
}

CRANK_VM_INLINE void
crankvm_primitive_returnUInteger32(crankvm_primitive_context_t *primitiveContext, uint32_t integer)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forUInteger32(primitiveContext->context, integer));
}

CRANK_VM_INLINE void
crankvm_primitive_returnInteger64(crankvm_primitive_context_t *primitiveContext, int64_t integer)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forInteger64(primitiveContext->context, integer));
}

CRANK_VM_INLINE void
crankvm_primitive_returnUInteger64(crankvm_primitive_context_t *primitiveContext, uint64_t integer)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forUInteger64(primitiveContext->context, integer));
}

#ifdef CRANK_VM_64_BITS
CRANK_VM_INLINE void
crankvm_primitive_returnInteger128(crankvm_primitive_context_t *primitiveContext, int64_t integer)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forInteger128(primitiveContext->context, integer));
}
#endif

CRANK_VM_INLINE void
crankvm_primitive_returnBoolean(crankvm_primitive_context_t *primitiveContext, int boolean)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forBoolean(primitiveContext->context, boolean));
}

CRANK_VM_INLINE void
crankvm_primitive_returnCharacter(crankvm_primitive_context_t *primitiveContext, uint32_t character)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_oop_encodeCharacter(character));
}

CRANK_VM_INLINE void
crankvm_primitive_returnFloat(crankvm_primitive_context_t *primitiveContext, double floatValue)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forFloat(primitiveContext->context, floatValue));
}

CRANK_VM_INLINE void
crankvm_primitive_returnOop(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t result)
{
    return crankvm_primitive_successWithResult(primitiveContext, result);
}

CRANK_VM_INLINE crankvm_context_t*
crankvm_primitive_getContext(crankvm_primitive_context_t *primitiveContext)
{
    return primitiveContext->context;
}

CRANK_VM_INLINE crankvm_MethodContext_t*
crankvm_primitive_getPrimitiveMethodContext(crankvm_primitive_context_t *primitiveContext)
{
    return primitiveContext->roots.primitiveMethodContext;
}

CRANK_VM_INLINE crankvm_MethodContext_t*
crankvm_primitive_getPrimitiveSenderMethodContext(crankvm_primitive_context_t *primitiveContext)
{
    return (crankvm_MethodContext_t*)crankvm_primitive_getPrimitiveMethodContext(primitiveContext)->baseClass.sender;
}

CRANK_VM_INLINE void
crankvm_primitive_finishReplacingMethodContext(crankvm_primitive_context_t *primitiveContext, crankvm_MethodContext_t *newMethodContext)
{
    primitiveContext->roots.primitiveMethodContext = newMethodContext;
}

CRANK_VM_INLINE crankvm_oop_t
crankvm_primitive_getLiteral(crankvm_primitive_context_t *primitiveContext, size_t literalIndex)
{
    if(crankvm_interpreter_checkLiteralIndex(primitiveContext->interpreter, literalIndex))
    {
        crankvm_primitive_fail(primitiveContext);
        return 0;
    }
    return crankvm_interpreter_getLiteral(primitiveContext->interpreter, literalIndex);
}

CRANK_VM_INLINE char *
crankvm_primitive_stringToCString(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t oop)
{
    if(!crankvm_oop_isPointer(oop))
    {
        crankvm_primitive_fail(primitiveContext);
        return 0;
    }

    crankvm_object_header_t *objectHeader = (crankvm_object_header_t*)oop;
    if(crankvm_object_header_getObjectFormat(objectHeader) < CRANK_VM_OBJECT_FORMAT_INDEXABLE_8)
    {
        crankvm_primitive_fail(primitiveContext);
        return 0;
    }

    size_t size = crankvm_object_header_getSmalltalkSize(objectHeader);
    char *allocated = (char*)crankvm_context_malloc(primitiveContext->context, size + 1);
    memcpy(allocated, objectHeader + 1, size);
    allocated[size] = 0;
    return allocated;
}

#endif //CRANK_VM_INTERPRETER_H
