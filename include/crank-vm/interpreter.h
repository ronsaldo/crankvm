#ifndef CRANK_VM_INTERPRETER_H
#define CRANK_VM_INTERPRETER_H

#include <crank-vm/objectmodel.h>
#include <crank-vm/error.h>
#include <crank-vm/special-objects.h>
#include <crank-vm/context.h>

typedef struct crankvm_interpreter_state_s crankvm_interpreter_state_t;

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
    } roots;
} crankvm_primitive_context_t;

typedef void (*crankvm_primitive_function_t) (crankvm_primitive_context_t *primitiveContext);

static inline void
crankvm_primitive_success(crankvm_primitive_context_t *primitiveContext)
{
    primitiveContext->error = CRANK_VM_PRIMITIVE_SUCCESS;
}

static inline void
crankvm_primitive_successWithResult(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t result)
{
    primitiveContext->error = CRANK_VM_PRIMITIVE_SUCCESS;
    primitiveContext->roots.result = result;
}

static inline void
crankvm_primitive_failWithCode(crankvm_primitive_context_t *primitiveContext, crankvm_primitive_error_code_t code)
{
    primitiveContext->error = code;
}

static inline void
crankvm_primitive_fail(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR);
}

static inline uint32_t
crankvm_primitive_getArgumentCount(crankvm_primitive_context_t *primitiveContext)
{
    return primitiveContext->argumentCount;
}

static inline crankvm_oop_t
crankvm_primitive_getArgument(crankvm_primitive_context_t *primitiveContext, size_t index)
{
    if(index >= primitiveContext->argumentCount)
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_NUMBER_OF_ARGUMENTS);
        return 0;
    }
    return primitiveContext->roots.arguments[index];
}

static inline crankvm_oop_t
crankvm_primitive_getReceiver(crankvm_primitive_context_t *primitiveContext)
{
    return primitiveContext->roots.receiver;
}

static inline int
crankvm_primitive_hasFailed(crankvm_primitive_context_t *primitiveContext)
{
    return primitiveContext->error != 0;
}

static inline intptr_t
crankvm_primitive_getSmallIntegerValue(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t oop)
{
    if(!crankvm_oop_isSmallInteger(oop))
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);
        return 0;
    }
    return crankvm_oop_decodeSmallInteger(oop);
}

static inline void
crankvm_primitive_returnSmallInteger(crankvm_primitive_context_t *primitiveContext, intptr_t integer)
{
    if(!crankvm_oop_isIntegerInSmallIntegerRange(integer))
        return crankvm_primitive_fail(primitiveContext);
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_oop_encodeSmallInteger(integer));
}

static inline void
crankvm_primitive_returnInteger(crankvm_primitive_context_t *primitiveContext, intptr_t integer)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forInteger(primitiveContext->context, integer));
}

static inline void
crankvm_primitive_returnBoolean(crankvm_primitive_context_t *primitiveContext, int boolean)
{
    return crankvm_primitive_successWithResult(primitiveContext, crankvm_object_forBoolean(primitiveContext->context, boolean));
}

// Interpreter accessors.
LIB_CRANK_VM_EXPORT crankvm_context_t *crankvm_interpreter_getContext(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_getReceiver(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_MethodContext_t *crankvm_interpreter_getMethodContext(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_CompiledCode_t *crankvm_interpreter_getCompiledCode(crankvm_interpreter_state_t *self);

// Oop stack access
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_pushOop(crankvm_interpreter_state_t *self, crankvm_oop_t oop);
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_checkSizeToPop(crankvm_interpreter_state_t *self, intptr_t size);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_popOop(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_stackOopAt(crankvm_interpreter_state_t *self, intptr_t size);

LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_checkReceiverSlotIndex(crankvm_interpreter_state_t *self, size_t index);
LIB_CRANK_VM_EXPORT crankvm_oop_t *crankvm_interpreter_getReceiverSlots(crankvm_interpreter_state_t *self);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_getReceiverSlot(crankvm_interpreter_state_t *self, size_t index);
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_interpreter_checkTemporaryIndex(crankvm_interpreter_state_t *self, size_t index);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_interpreter_getTemporary(crankvm_interpreter_state_t *self, size_t index);

#endif //CRANK_VM_INTERPRETER_H
