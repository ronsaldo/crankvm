#include "system-primitives.h"

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
