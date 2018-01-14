#include "arithmetic-primitives.h"

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerAdd, 1)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerSubtract, 2)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerLessThan, 3)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerGreaterThan, 4)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerLessOrEqual, 5)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerGreaterOrEqual, 6)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerEqual, 7)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerNotEqual, 8)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerMultiply, 9)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerDivide, 10)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerQuo, 13)

void
crankvm_primitive_integerAdd(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnInteger(primitiveContext, left + right);
}

void
crankvm_primitive_integerSubtract(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnInteger(primitiveContext, left - right);
}

void
crankvm_primitive_integerMultiply(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext))
        return;

#ifdef CRANK_VM_64_BITS
    __int128 result = (__int128)left * (__int128)right;
#else
    int64_t result = (int64_t)left * (int64_t)right;
#endif

    // Check for overflow
    int inRange = result < CRANK_VM_SMALL_INTEGER_MIN_VALUE || result > CRANK_VM_SMALL_INTEGER_MAX_VALUE;
    if(!inRange)
        return crankvm_primitive_fail(primitiveContext);

    return crankvm_primitive_returnInteger(primitiveContext, result);
}

void
crankvm_primitive_integerDivide(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext))
        return;

    // We cannot divide by zero, and the module must be zero here
    if(right == 0 || (left % right) != 0)
        return crankvm_primitive_fail(primitiveContext);

    return crankvm_primitive_returnInteger(primitiveContext, left / right);
}


void
crankvm_primitive_integerQuo(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext))
        return;

    // We cannot divide by zero, and the module must be zero here
    if(right == 0)
        return crankvm_primitive_fail(primitiveContext);

    return crankvm_primitive_returnInteger(primitiveContext, left / right);
}

void
crankvm_primitive_integerLessThan(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left < right);
}

void
crankvm_primitive_integerGreaterThan(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left > right);
}

void
crankvm_primitive_integerLessOrEqual(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left <= right);
}

void
crankvm_primitive_integerGreaterOrEqual(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left >= right);
}

void
crankvm_primitive_integerEqual(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left == right);
}

void
crankvm_primitive_integerNotEqual(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left != right);
}
