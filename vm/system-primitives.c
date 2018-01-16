#include "system-primitives.h"

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_identityEquals, 110)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_identityNotEquals, 169)

void
crankvm_primitive_primitiveFail(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_fail(primitiveContext);
}

void
crankvm_primitive_primitiveFailUnexistent(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_fail(primitiveContext);
}

void
crankvm_primitive_identityEquals(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t left = crankvm_primitive_getReceiver(primitiveContext);
    crankvm_oop_t right = crankvm_primitive_getArgument(primitiveContext, 0);
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left == right);
}

void
crankvm_primitive_identityNotEquals(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t left = crankvm_primitive_getReceiver(primitiveContext);
    crankvm_oop_t right = crankvm_primitive_getArgument(primitiveContext, 0);
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left != right);
}
