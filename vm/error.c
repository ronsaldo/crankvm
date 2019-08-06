#include <crank-vm/error.h>

LIB_CRANK_VM_EXPORT const char *crankvm_error_getString(crankvm_error_t error)
{
    switch(error)
    {
    case CRANK_VM_OK: return "OK.";
    case CRANK_VM_ERROR_BAD_IMAGE: return "Bad image.";
    case CRANK_VM_ERROR_INVALID_PARAMETER: return "Invalid parameter.";
    case CRANK_VM_ERROR_NULL_POINTER: return "Null pointer.";
    case CRANK_VM_ERROR_OUT_OF_BOUNDS: return "Out of bounds.";
    case CRANK_VM_ERROR_OUT_OF_MEMORY: return "Out of memory.";
    case CRANK_VM_ERROR_UNIMPLEMENTED: return "Unimplemented.";
    case CRANK_VM_ERROR_UNSUPPORTED_OPERATION: return "Unsupported operation.";
    case CRANK_VM_ERROR_FAILED_TO_OPEN_FILE: return "Failed to open file.";
    case CRANK_VM_ERROR_FAILED_TO_READ_FILE: return "Failed to read file.";
    case CRANK_VM_ERROR_UNRECOGNIZED_IMAGE_FORMAT: return "Unrecognized image format.";
    case CRANK_VM_ERROR_BAD_IMAGE_WORD_SIZE: return "Bad image word size.";
    case CRANK_VM_ERROR_BAD_IMAGE_ENDIANNESS: return "Bad image endiannes.";
    case CRANK_VM_ERROR_STACK_OVERFLOW: return "Stack overflow.";
    case CRANK_VM_ERROR_STACK_UNDERFLOW: return "Stack underfow.";
    case CRANK_VM_ERROR_RECEIVER_CLASS_NIL: return "Receiver class nil.";
    case CRANK_VM_ERROR_CALLED_METHOD_ARGUMENT_MISMATCH: return "Called method argument count mismatch.";
    case CRANK_VM_ERROR_ILLEGAL_INSTRUCTION: return "Illegal instruction.";
    case CRANK_VM_ERROR_ILLEGAL_STORE: return "Illegal store instruction.";
    case CRANK_VM_ERROR_NIL_REMOTE_VECTOR: return "Accessing to nil remote vector.";
    default: return "Unknown error code.";
    }
}
