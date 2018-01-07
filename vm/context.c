#include "context-internal.h"
#include <string.h>
#include <malloc.h>

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_context_create(crankvm_context_t **returnContext)
{
    // Allocate the context
    crankvm_context_t *context = malloc(sizeof(crankvm_context_t));
    if(!context)
        return CRANK_VM_ERROR_OUT_OF_MEMORY;

    memset(context, 0, sizeof(crankvm_context_t));

    // Initialize the context
    context->heap.maxCapacity = CRANK_VM_CONTEXT_DEFAULT_MAX_HEAP_CAPACITY;

    *returnContext = context;
    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT void
crankvm_context_destroy(crankvm_context_t *context)
{
    if(!context)
        return;

    crankvm_heap_destroy(&context->heap);
    free(context);
}

LIB_CRANK_VM_EXPORT void
crankvm_context_setMaxHeapCapacity(crankvm_context_t *context, size_t capacity)
{
    if(!context)
        return;

    context->heap.maxCapacity = capacity;
}

LIB_CRANK_VM_EXPORT size_t
crankvm_context_getMaxHeapCapacity(crankvm_context_t *context)
{
    if(!context)
        return 0;

    return context->heap.maxCapacity;
}
