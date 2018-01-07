#ifndef CRANK_VM_CONTEXT_INTERNAL_H
#define CRANK_VM_CONTEXT_INTERNAL_H

#include <crank-vm/context.h>
#include <stdbool.h>
#include "heap.h"

struct crankvm_context_s
{
    // The heap.
    crankvm_heap_t heap;
    crankvm_oop_t specialObjectsOop;
};

#endif //CRANK_VM_CONTEXT_INTERNAL_H
