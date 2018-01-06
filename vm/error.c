#include <crank-vm/error.h>

LIB_CRANK_VM_EXPORT const char *crankvm_error_getString(crankvm_error_t error)
{
    switch(error)
    {
    case CRANK_VM_OK: return "OK";
    case CRANK_VM_ERROR_BAD_IMAGE: return "Bad image";
    case CRANK_VM_ERROR_INVALID_PARAMETER: return "Invalid parameter";
    case CRANK_VM_ERROR_NULL_POINTER: return "Null pointer";
    case CRANK_VM_ERROR_OUT_OF_BOUNDS: return "Out of bounds";
    case CRANK_VM_ERROR_OUT_OF_MEMORY: return "Out of memory";
    case CRANK_VM_ERROR_UNIMPLEMENTED: return "Unimplemented";
    case CRANK_VM_ERROR_UNSUPPORTED_OPERATION: return "Unsupported operation";
    default: return "Unknown error code";
    }
}
