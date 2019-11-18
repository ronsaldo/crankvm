#include "message-primitives.h"

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_performWithArgumentsInSuperclass, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_PERFORM_IN_SUPERCLASS)

void
crankvm_primitive_performWithArgumentsInSuperclass(crankvm_primitive_context_t *primitiveContext)
{
    // Fetch the arguments.
    crankvm_oop_t receiver = crankvm_primitive_getReceiver(primitiveContext);
    crankvm_oop_t selector = crankvm_primitive_getArgument(primitiveContext, 0);
    crankvm_oop_t arguments = crankvm_primitive_getArgument(primitiveContext, 1);
    crankvm_oop_t lookupClass = crankvm_primitive_getArgument(primitiveContext, 2);
    if(crankvm_primitive_hasFailed(primitiveContext)) return;

    // Arguments must be a pointer.
    if(!crankvm_oop_isPointer(arguments))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);

    size_t argumentCount = crankvm_object_header_getSlotCount((crankvm_object_header_t *)arguments);
    crankvm_Array_t *argumentsArray = (crankvm_Array_t*)arguments;

    // Create the activation context.
    crankvm_MethodContext_t *methodContext = crankvm_interpreter_createMethodContextForMessageSendWithLookupClass(primitiveContext->interpreter,
            receiver, selector, lookupClass, argumentCount, argumentsArray->slots);

    if(crankvm_oop_isNil(primitiveContext->context, (crankvm_oop_t)methodContext))
        return crankvm_primitive_fail(primitiveContext);

    // Set the sender context.
    methodContext->baseClass.sender = (crankvm_oop_t)crankvm_primitive_getPrimitiveSenderMethodContext(primitiveContext);

    // Finish the primitive by activating the new method context.
    crankvm_primitive_finishReplacingMethodContext(primitiveContext, methodContext);
}
