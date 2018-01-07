#include "context-internal.h"
#include <string.h>
#include <malloc.h>

crankvm_error_t crankvm_interpret(crankvm_context_t *context, crankvm_MethodContext_t *methodContext, crankvm_oop_t *returnValue);

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

LIB_CRANK_VM_EXPORT crankvm_special_object_array_t *
crankvm_context_getSpecialObjectsArray(crankvm_context_t *context)
{
    if(!context)
        return NULL;
    return context->specialObjectsArray;
}

LIB_CRANK_VM_EXPORT int
crankvm_context_isNil(crankvm_context_t *context, void *pointer)
{
    return !context || !context->specialObjectsArray || ((crankvm_oop_t)pointer == context->specialObjectsArray->nilObject);
}

static crankvm_ProcessorScheduler_t *
crankvm_context_getScheduler(crankvm_context_t *context)
{
    if(!context->specialObjectsArray ||
        crankvm_context_isNil(context, context->specialObjectsArray->schedulerAssociation) ||
        crankvm_oop_isNil(context, context->specialObjectsArray->schedulerAssociation->value))
        return NULL;

    return (crankvm_ProcessorScheduler_t*)context->specialObjectsArray->schedulerAssociation->value;
}

static crankvm_Process_t *
crankvm_context_getActiveProcess(crankvm_context_t *context)
{
    crankvm_ProcessorScheduler_t *scheduler = crankvm_context_getScheduler(context);
    if(!scheduler || crankvm_oop_isNil(context, scheduler->activeProcess))
        return NULL;

    return (crankvm_Process_t*)scheduler->activeProcess;
}

static crankvm_MethodContext_t *
crankvm_context_getEntryMethodContext(crankvm_context_t *context)
{
    if(!context->specialObjectsArray ||
        crankvm_object_header_getSlotCount((crankvm_object_header_t *)context->specialObjectsArray) < 63 ||
        crankvm_context_isNil(context, context->specialObjectsArray->crankVMEntryContext) ||
        crankvm_oop_isNil(context, context->specialObjectsArray->crankVMEntryContext->value))
        return NULL;

    printf("entry point key: '%.*s'\n", crankvm_string_printf_arg(context->specialObjectsArray->crankVMEntryContext->key));
    return (crankvm_MethodContext_t *)context->specialObjectsArray->crankVMEntryContext->value;
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_context_run(crankvm_context_t *context)
{
    crankvm_MethodContext_t *methodContext = crankvm_context_getEntryMethodContext(context);
    if(!methodContext)
    {
        crankvm_Process_t *process = crankvm_context_getActiveProcess(context);
        if(!process || crankvm_oop_isNil(context, process->suspendedContext))
            return CRANK_VM_ERROR_UNSUPPORTED_OPERATION;

        printf("Processor name: '%.*s'\n", crankvm_string_printf_arg(context->specialObjectsArray->schedulerAssociation->key));
        printf("Active process %p name '%.*s' suspended context %p\n", process, crankvm_string_printf_arg(process->name), (void*)process->suspendedContext);
        methodContext = (crankvm_MethodContext_t *)process->suspendedContext;
    }

    printf("Entry method context: %p\n", methodContext);
    crankvm_oop_t returnValue;
    crankvm_error_t error = crankvm_interpret(context, methodContext, &returnValue);
    if(error)
        return error;

    printf("Interpreter result: %p\n", (void*)returnValue);
    return CRANK_VM_OK;
}
