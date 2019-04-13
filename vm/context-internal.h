#ifndef CRANK_VM_CONTEXT_INTERNAL_H
#define CRANK_VM_CONTEXT_INTERNAL_H

#include <crank-vm/context.h>
#include <crank-vm/special-objects.h>
#include <stdbool.h>
#include "heap.h"

struct crankvm_context_s
{
    // The heap.
    crankvm_heap_t heap;

    // Class table
    size_t numberOfClassTablePages;
    size_t nextClassTableIndex;

    // Identity hash
    uint32_t lastIdentityHash;

    struct {
        crankvm_special_object_array_t *specialObjectsArray;

        crankvm_oop_t nilOop;
        crankvm_oop_t falseOop;
        crankvm_oop_t trueOop;

        crankvm_oop_t byteSymbolClassOop; /* Use for pretty printing objects*/ 

        crankvm_oop_t freeListObject;
        crankvm_HiddenRoots_t *hiddenRootsObject;
        crankvm_ClassTablePage_t *firstClassTablePage;
    } roots;
};

#endif //CRANK_VM_CONTEXT_INTERNAL_H
