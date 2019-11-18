#ifndef CRANK_VM_SCHEDULING_PRIMITIVES_H
#define CRANK_VM_SCHEDULING_PRIMITIVES_H

#include "interpreter-internal.h"

void crankvm_primitive_semaphoreSignal(crankvm_primitive_context_t *context);
void crankvm_primitive_semaphoreWait(crankvm_primitive_context_t *context);

#endif //CRANK_VM_SCHEDULING_PRIMITIVES_H
