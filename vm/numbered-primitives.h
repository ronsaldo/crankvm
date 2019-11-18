#ifndef CRANK_VM_NUMBERED_PRIMITIVES_H
#define CRANK_VM_NUMBERED_PRIMITIVES_H

#include "quick-method-primitives.h"
#include "arithmetic-primitives.h"
#include "external-primitives.h"
#include "message-primitives.h"
#include "system-primitives.h"
#include "object-primitives.h"
#include "block-primitives.h"
#include "scheduling-primitives.h"

extern const size_t crankvm_numberedPrimitiveTableSize;
extern const crankvm_primitive_function_t crankvm_numberedPrimitiveTable[];

#endif //CRANK_VM_NUMBERED_PRIMITIVES_H
