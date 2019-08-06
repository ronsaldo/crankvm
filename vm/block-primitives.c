#include "block-primitives.h"

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_blockValue0, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS0)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_blockValue1, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS1)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_blockValue2, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS2)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_blockValue3, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS3)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_blockValue4, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS4)

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_blockValue0, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS0_NO_CONTEXT_SWITCH)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_blockValue1, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BLOCK_VALUE_ARGS1_NO_CONTEXT_SWITCH)

void
crankvm_primitive_blockValueFixed(crankvm_primitive_context_t *primitiveContext, int expectedArgumentCount)
{
    // Fetch the block closure, and fetch some of its parameters.
    crankvm_BlockClosure_t *blockClosure = (crankvm_BlockClosure_t*)crankvm_primitive_getReceiver(primitiveContext);
    intptr_t numArgs = crankvm_primitive_getSmallIntegerValue(primitiveContext, blockClosure->numArgs);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return;

    // Check the argument count.
    if(numArgs != expectedArgumentCount ||
        numArgs != crankvm_primitive_getArgumentCount(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    // Create the block closure activation context.
    crankvm_MethodContext_t *activationContext;
    crankvm_error_t error = crankvm_MethodContext_createBlockClosureActivationContext(
            crankvm_primitive_getContext(primitiveContext), &activationContext, blockClosure, numArgs, NULL);
    if(error)
        return crankvm_primitive_failWithVMErrorCode(primitiveContext, error);

    // Copy the arguments.
    for(int i = 0; i < expectedArgumentCount; ++i)
        activationContext->stackSlots[i] = crankvm_primitive_getArgument(primitiveContext, i);

    // Set the sender context.
    activationContext->baseClass.sender = (crankvm_oop_t)crankvm_primitive_getPrimitiveSenderMethodContext(primitiveContext);

    // Finish the primitive by activating the new method context.
    crankvm_primitive_finishReplacingMethodContext(primitiveContext, activationContext);
}

void
crankvm_primitive_blockValue0(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_blockValueFixed(primitiveContext, 0);
}

void
crankvm_primitive_blockValue1(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_blockValueFixed(primitiveContext, 1);
}

void
crankvm_primitive_blockValue2(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_blockValueFixed(primitiveContext, 2);
}

void
crankvm_primitive_blockValue3(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_blockValueFixed(primitiveContext, 3);
}

void
crankvm_primitive_blockValue4(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_blockValueFixed(primitiveContext, 4);
}
