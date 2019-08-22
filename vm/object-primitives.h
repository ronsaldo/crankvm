#ifndef CRANK_VM_OBJECT_PRIMITIVES_H
#define CRANK_VM_OBJECT_PRIMITIVES_H

#include "interpreter-internal.h"

void crankvm_primitive_identityEquals(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_identityNotEquals(crankvm_primitive_context_t *primitiveContext);

void crankvm_primitive_at(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_atPut(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_size(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_stringAt(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_stringAtPut(crankvm_primitive_context_t *primitiveContext);

void crankvm_primitive_objectAt(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_objectAtPut(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_new(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_newWithArg(crankvm_primitive_context_t *primitiveContext);

void crankvm_primitive_class(crankvm_primitive_context_t *primitiveContext);

void crankvm_primitive_asCharacter(crankvm_primitive_context_t *primitiveContext);
void crankvm_primitive_immediateAsInteger(crankvm_primitive_context_t *primitiveContext);

void crankvm_primitive_shallowCopy(crankvm_primitive_context_t *primitiveContext);

#endif //CRANK_VM_OBJECT_PRIMITIVES_H
