#include "scheduling-primitives.h"

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_semaphoreSignal, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_SEMAPHORE_SIGNAL)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_semaphoreWait, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_SEMAPHORE_WAIT)

static crankvm_Semaphore_t*
crankvm_primitive_fetchReceiverSemaphore(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t semaphoreOop = crankvm_primitive_getReceiver(primitiveContext);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return NULL;

    if(!crankvm_oop_isPointer(semaphoreOop) ||
        crankvm_object_header_getSlotCount((crankvm_object_header_t *)semaphoreOop) < 3)
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);
        return NULL;
    }

    return (crankvm_Semaphore_t*)semaphoreOop;
}

static int crankvm_Semaphore_isEmpty(crankvm_context_t *context, crankvm_Semaphore_t *self)
{
    return crankvm_oop_isNil(context, self->baseClass.firstLink);
}

void
crankvm_primitive_semaphoreSignal(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_Semaphore_t* semaphore = crankvm_primitive_fetchReceiverSemaphore(primitiveContext);
    if(!semaphore) return;

    if(crankvm_Semaphore_isEmpty(primitiveContext->context, semaphore))
    {
        semaphore->excessSignals = crankvm_oop_encodeSmallInteger(crankvm_oop_decodeSmallInteger(semaphore->excessSignals) + 1);
    }
    else
    {
        printf("Unimplemented semaphore signal resume once\n");
        abort();
    }
}

void
crankvm_primitive_semaphoreWait(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_Semaphore_t* semaphore = crankvm_primitive_fetchReceiverSemaphore(primitiveContext);
    if(!semaphore) return;

    intptr_t excessSignals = crankvm_oop_decodeSmallInteger(semaphore->excessSignals);
    if(excessSignals <= 0)
    {
        printf("Unimplemented semaphore wait yield\n");
        abort();
    }
    else
    {
        semaphore->excessSignals = crankvm_oop_encodeSmallInteger(excessSignals - 1);
    }
}
