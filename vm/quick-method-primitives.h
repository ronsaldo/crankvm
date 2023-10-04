#ifndef CRANK_VM_QUICK_METHOD_PRIMITIVES_H
#define CRANK_VM_QUICK_METHOD_PRIMITIVES_H

#include <crank-vm/common.h>

CRANK_VM_INLINE int
crankvm_primitive_isQuickMethod(int primitiveNumber)
{
    return 256 <= primitiveNumber && primitiveNumber <= 519;
}

#endif //CRANK_VM_QUICK_METHOD_PRIMITIVES_H
