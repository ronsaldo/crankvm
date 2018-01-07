#include "crank-vm/crank-vm.h"
#include "context-internal.h"
#include "heap.h"
#include <string.h>

typedef struct crankvm_interpreter_state_s
{
    crankvm_context_t *context;
    crankvm_MethodContext_t *methodContext;
    crankvm_oop_t *callerReturnValuePointer;

    intptr_t stackPointer;
    uint8_t *instructions;

    intptr_t pc;
    int currentBytecode;

    intptr_t nextPC;
    int nextBytecode;
} crankvm_interpreter_state_t;

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_CompiledCode_validate(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode)
{
    if(crankvm_context_isNilOrNull(context, compiledCode))
        return CRANK_VM_ERROR_NULL_POINTER;

    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_MethodContext_validate(crankvm_context_t *context, crankvm_MethodContext_t *methodContext)
{
    if(crankvm_context_isNilOrNull(context, methodContext))
        return CRANK_VM_ERROR_NULL_POINTER;

    if(!crankvm_oop_isSmallInteger(methodContext->baseClass.pc) ||
        !crankvm_oop_isSmallInteger(methodContext->stackp))
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    return CRANK_VM_OK;
}

static inline crankvm_error_t
crankvm_interpreter_fetchNextInstruction(crankvm_interpreter_state_t *self)
{
    self->nextBytecode = self->instructions[self->nextPC];
    return CRANK_VM_OK;
}

static inline crankvm_error_t
crankvm_interpreter_fetchMethodContext(crankvm_interpreter_state_t *self)
{
    // Validate the context
    crankvm_error_t error = crankvm_MethodContext_validate(self->context, self->methodContext);
    if(error)
        return error;

    self->nextPC = crankvm_oop_decodeSmallInteger(self->methodContext->baseClass.pc);
    self->stackPointer = crankvm_oop_decodeSmallInteger(self->methodContext->stackp);
    if(self->nextPC <= 0)
        return CRANK_VM_ERROR_INVALID_PARAMETER;
    --self->nextPC;

    // Get the pointer into the instructions.
    printf("\tmethod: %p pc: %d stackp: %d\n", (void*)self->methodContext->method, (int)self->pc, (int)self->stackPointer);
    self->instructions = (uint8_t*)(self->methodContext->method + sizeof(crankvm_object_header_t));

    // Fetch the first bytecode.
    error = crankvm_interpreter_fetchNextInstruction(self);
    if(error)
        return error;

    return CRANK_VM_OK;
}

crankvm_error_t
crankvm_interpreter_run(crankvm_interpreter_state_t *self)
{
    if(self->callerReturnValuePointer)
        *self->callerReturnValuePointer = crankvm_specialObject_nil(self->context);

    // Validate and fetch the method context
    crankvm_error_t error = crankvm_interpreter_fetchMethodContext(self);
    if(error)
        return error;

    // Fetch the first instruction.
    for(;;)
    {
        self->currentBytecode = self->nextBytecode;
        self->pc = self->nextPC++;
        crankvm_interpreter_fetchNextInstruction(self);

        printf("Bytecode: %02X\n", self->currentBytecode);

        switch(self->currentBytecode)
        {
        case 0x76:
        case 0x77:
            break;
        default:
            return CRANK_VM_OK;
        }
    }

    return CRANK_VM_OK;
}

crankvm_error_t
crankvm_interpret(crankvm_context_t *context, crankvm_MethodContext_t *methodContext, crankvm_oop_t *callerReturnValuePointer)
{
    crankvm_interpreter_state_t state;
    memset(&state, 0, sizeof(state));
    state.methodContext = methodContext;
    state.context = context;
    state.callerReturnValuePointer = callerReturnValuePointer;

    return crankvm_interpreter_run(&state);
}
