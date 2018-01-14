#ifndef CRANK_VM_INTERPRETER_INTERNAL_H
#define CRANK_VM_INTERPRETER_INTERNAL_H

#include "crank-vm/crank-vm.h"
#include "crank-vm/interpreter.h"
#include "context-internal.h"
#include "heap.h"
#include <string.h>
#include <assert.h>

struct crankvm_interpreter_state_s
{
    crankvm_context_t *context;
    crankvm_oop_t *callerReturnValuePointer;

    crankvm_compiled_code_header_t codeHeader;

    intptr_t stackPointer;
    intptr_t stackLimit;
    uint8_t *instructions;

    intptr_t pc;
    int currentBytecode;
    int currentBytecodeSetOffset;

    intptr_t nextPC;
    int nextBytecode;
    bool returnFromInterpreter;

    struct
    {
        crankvm_MethodContext_t *methodContext;
        crankvm_CompiledCode_t *method;
        crankvm_oop_t receiver;
    } objects;

};

#define CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(name, number) /* Nothing generated */

#endif //CRANK_VM_INTERPRETER_H
