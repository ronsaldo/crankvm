#ifndef CRANK_VM_ERROR_H
#define CRANK_VM_ERROR_H

#include <crank-vm/common.h>

typedef enum crankvm_error_e
{
    CRANK_VM_OK = 0,
    CRANK_VM_ERROR_INVALID_PARAMETER = -1,
    CRANK_VM_ERROR_NULL_POINTER = -2,
    CRANK_VM_ERROR_OUT_OF_BOUNDS = -3,
    CRANK_VM_ERROR_UNSUPPORTED_OPERATION = -4,
    CRANK_VM_ERROR_UNIMPLEMENTED = -5,
    CRANK_VM_ERROR_BAD_IMAGE = -6,
    CRANK_VM_ERROR_OUT_OF_MEMORY = -7,
} crankvm_error_t;

LIB_CRANK_VM_EXPORT const char *crankvm_error_getString(crankvm_error_t error);

#endif //CRANK_VM_ERROR_H
