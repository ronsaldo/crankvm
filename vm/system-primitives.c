#include "system-primitives.h"


CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_primitiveFail, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_PRIMITIVE_FAIL) // This must fail

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
