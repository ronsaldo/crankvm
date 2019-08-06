#include "arithmetic-primitives.h"
#include "system-primitives.h"

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerAdd, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_ADD)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerSubtract, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_SUBTRACT)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerLessThan, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_LESS_THAN)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerGreaterThan, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_GREATER_THAN)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerLessOrEqual, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_LESS_OR_EQUAL)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerGreaterOrEqual, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_GREATER_OR_EQUAL)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerEqual, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_EQUAL)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerNotEqual, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_NOT_EQUAL)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerMultiply, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_MULTIPLY)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerDivide, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_DIVIDE)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerMod, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_MOD)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerDiv, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_DIV)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerQuo, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_QUO)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerBitAnd, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_BIT_AND)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerBitOr, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_BIT_OR)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerBitXor, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_BIT_XOR)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_integerBitShift, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_INTEGER_BIT_SHIFT)

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
    int overflow = result < CRANK_VM_SMALL_INTEGER_MIN_VALUE || result > CRANK_VM_SMALL_INTEGER_MAX_VALUE;
    if(overflow)
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
crankvm_primitive_integerDiv(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t dividend = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t divisor = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext))
        return;

    // We cannot divide by zero, and the module must be zero here
    if(divisor == 0)
        return crankvm_primitive_fail(primitiveContext);

    // Floored division technique from: http://www.microhowto.info/howto/round_towards_minus_infinity_when_dividing_integers_in_c_or_c++.html
    intptr_t quotient = dividend / divisor;
    intptr_t rem = dividend % divisor;
    if(rem != 0 && (rem < 0) != (dividend < 0))
        --quotient;

    return crankvm_primitive_returnInteger(primitiveContext, quotient);
}

void
crankvm_primitive_integerMod(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t dividend = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t divisor = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext))
        return;

    // We cannot divide by zero, and the module must be zero here
    if(divisor == 0)
        return crankvm_primitive_fail(primitiveContext);

    // Floored division technique from: http://www.microhowto.info/howto/round_towards_minus_infinity_when_dividing_integers_in_c_or_c++.html
    intptr_t rem = dividend % divisor;
    if(rem != 0 && (rem < 0) != (dividend < 0))
        rem += divisor;

    return crankvm_primitive_returnInteger(primitiveContext, rem);
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
crankvm_primitive_integerBitAnd(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnInteger(primitiveContext, left & right);
}

void
crankvm_primitive_integerBitOr(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnInteger(primitiveContext, left | right);
}

void
crankvm_primitive_integerBitXor(crankvm_primitive_context_t *primitiveContext)
{
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnInteger(primitiveContext, left ^ right);
}

void
crankvm_primitive_integerBitShift(crankvm_primitive_context_t *primitiveContext)
{
    // FIXME: Implement this properly.
    intptr_t left = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
    intptr_t right = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(!crankvm_primitive_hasFailed(primitiveContext))
    {
        printf("Bit shift %ld %ld\n", left, right);
        if(right < 0)
        {
            return crankvm_primitive_returnInteger(primitiveContext, left >> (-right));
        }
        else if(right > 0)
        {
            return crankvm_primitive_returnInteger(primitiveContext, left << right);
        }
    }
    printf("TODO: crankvm_primitive_integerBitShift\n");
    abort();
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
