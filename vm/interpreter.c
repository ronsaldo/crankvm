#include "interpreter-internal.h"
#include "numbered-primitives.h"

// <editor-fold> Interpreter public interface
LIB_CRANK_VM_EXPORT inline crankvm_context_t*
crankvm_interpreter_getContext(crankvm_interpreter_state_t *self)
{
    return self->context;
}

LIB_CRANK_VM_EXPORT inline crankvm_MethodContext_t*
crankvm_interpreter_getMethodContext(crankvm_interpreter_state_t *self)
{
    return self->objects.methodContext;
}

LIB_CRANK_VM_EXPORT inline crankvm_CompiledCode_t*
crankvm_interpreter_getCompiledCode(crankvm_interpreter_state_t *self)
{
    return self->objects.method;
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t
crankvm_interpreter_getReceiver(crankvm_interpreter_state_t *self)
{
    return self->objects.receiver;
}

LIB_CRANK_VM_EXPORT inline crankvm_error_t
crankvm_interpreter_pushOop(crankvm_interpreter_state_t *self, crankvm_oop_t oop)
{
    if(self->stackPointer >= self->stackLimit)
        return CRANK_VM_ERROR_STACK_OVERFLOW;

    self->objects.methodContext->stackSlots[self->stackPointer++] = oop;
    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT inline crankvm_error_t
crankvm_interpreter_checkSizeToPop(crankvm_interpreter_state_t *self, intptr_t size)
{
    if(self->stackPointer - size < 0)
        return CRANK_VM_ERROR_STACK_UNDERFLOW;

    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t
crankvm_interpreter_popOop(crankvm_interpreter_state_t *self)
{
    assert(self->stackPointer > 0);
    crankvm_oop_t result = self->objects.methodContext->stackSlots[--self->stackPointer];
    self->objects.methodContext->stackSlots[self->stackPointer] = self->context->roots.nilOop;
    return result;
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t
crankvm_interpreter_stackOopAt(crankvm_interpreter_state_t *self, intptr_t size)
{
    assert(self->stackPointer > size);
    return self->objects.methodContext->stackSlots[self->stackPointer - size - 1];
}

LIB_CRANK_VM_EXPORT inline crankvm_error_t
crankvm_interpreter_checkReceiverSlotIndex(crankvm_interpreter_state_t *self, size_t index)
{
    if(!crankvm_oop_isPointer(self->objects.receiver) ||
        index >= crankvm_object_header_getSlotCount((crankvm_object_header_t*)self->objects.receiver))
        return CRANK_VM_ERROR_OUT_OF_BOUNDS;

    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t*
crankvm_interpreter_getReceiverSlots(crankvm_interpreter_state_t *self)
{
    assert(crankvm_oop_isPointer(self->objects.receiver));
    return (crankvm_oop_t*) (self->objects.receiver + sizeof(crankvm_object_header_t));
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t
crankvm_interpreter_getReceiverSlot(crankvm_interpreter_state_t *self, size_t index)
{
    assert(crankvm_interpreter_checkReceiverSlotIndex(self, index) == CRANK_VM_OK);
    return crankvm_interpreter_getReceiverSlots(self)[index];
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t
crankvm_interpreter_setReceiverSlot(crankvm_interpreter_state_t *self, size_t index, crankvm_oop_t value)
{
    assert(crankvm_interpreter_checkReceiverSlotIndex(self, index) == CRANK_VM_OK);
    crankvm_interpreter_getReceiverSlots(self)[index] = value;
    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT inline crankvm_error_t
crankvm_interpreter_checkTemporaryIndex(crankvm_interpreter_state_t *self, size_t index)
{
    if(index >= self->stackPointer)
        return CRANK_VM_ERROR_OUT_OF_BOUNDS;
    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t
crankvm_interpreter_getTemporary(crankvm_interpreter_state_t *self, size_t index)
{
    assert(crankvm_interpreter_checkTemporaryIndex(self, index) == CRANK_VM_OK);
    return self->objects.methodContext->stackSlots[index];
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t
crankvm_interpreter_setTemporary(crankvm_interpreter_state_t *self, size_t index, crankvm_oop_t value)
{
    assert(crankvm_interpreter_checkTemporaryIndex(self, index) == CRANK_VM_OK);
    self->objects.methodContext->stackSlots[index] = value;
    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT inline crankvm_error_t
crankvm_interpreter_checkLiteralIndex(crankvm_interpreter_state_t *self, size_t index)
{
    if(index >= self->codeHeader.numberOfLiterals)
        return CRANK_VM_ERROR_OUT_OF_BOUNDS;
    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT inline crankvm_oop_t
crankvm_interpreter_getLiteral(crankvm_interpreter_state_t *self, size_t index)
{
    assert(crankvm_interpreter_checkLiteralIndex(self, index) == CRANK_VM_OK);
    return self->objects.method->literals[index];
}

// </editor-fold> Interpreter public interface

#define UNIMPLEMENTED() \
do { \
    fprintf(stderr, "Unimplemented function %s in %s:%d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
    abort(); \
} while(false)

#define fetchNextInstruction() crankvm_interpreter_fetchNextInstruction(self)
#define pushOop(oop) do { \
    crankvm_error_t error = crankvm_interpreter_pushOop(self, oop); \
    if(error) return error; \
} while(0)

#define checkSizeToPop(size) do { \
    crankvm_error_t error = crankvm_interpreter_checkSizeToPop(self, size); \
    if(error) return error; \
} while(0)

#define checkReceiverSlotIndex(index) do { \
    crankvm_error_t error = crankvm_interpreter_checkReceiverSlotIndex(self, index); \
    if(error) return error; \
} while(0)

#define checkTemporaryIndex(index) do { \
    crankvm_error_t error = crankvm_interpreter_checkTemporaryIndex(self, index); \
    if(error) return error; \
} while(0)

#define checkLiteralIndex(index) do { \
    crankvm_error_t error = crankvm_interpreter_checkLiteralIndex(self, index); \
    if(error) return error; \
} while(0)

#define popOop() crankvm_interpreter_popOop(self)

#define _theContext (self->context)
#define _theSpecialObjectsArray (_theContext->roots.specialObjectsArray)
#define _theSpecialSelectors (_theSpecialObjectsArray->specialSelectors)

static inline crankvm_error_t
crankvm_interpreter_fetchNextInstruction(crankvm_interpreter_state_t *self)
{
    self->nextPC = self->pc;
    self->nextBytecode = self->instructions[self->nextPC++];
    return CRANK_VM_OK;
}

static inline uint8_t
crankvm_interpreter_fetchByte(crankvm_interpreter_state_t *self)
{
    return self->instructions[self->pc++];
}

static inline crankvm_error_t
crankvm_interpreter_fetchMethodContext(crankvm_interpreter_state_t *self)
{
    // Validate the context
    printf("crankvm_interpreter_fetchMethodContext %p\n", self->objects.methodContext);
    crankvm_error_t error = crankvm_MethodContext_validate(self->context, self->objects.methodContext);
    if(error)
        return error;

    // Read some elements for easier access.
    self->objects.receiver = self->objects.methodContext->receiver;
    self->objects.method = (crankvm_CompiledCode_t*)self->objects.methodContext->method;

    // Decode the method/fullblock header.
    error = crankvm_specialObject_getCompiledCodeHeader(&self->codeHeader, self->objects.method);
    if(error)
        return error;

    crankvm_MethodContext_t *methodContext = self->objects.methodContext;
    self->pc = crankvm_oop_decodeSmallInteger(methodContext->baseClass.pc);
    self->stackPointer = crankvm_oop_decodeSmallInteger(methodContext->stackp);
    self->stackLimit = crankvm_object_header_getSlotCount((crankvm_object_header_t *)methodContext);
    if(self->pc <= 0 || self->stackPointer > self->stackLimit)
        return CRANK_VM_ERROR_INVALID_PARAMETER;
    --self->pc;

    // Get the pointer into the instructions.
    crankvm_oop_t methodSelector = crankvm_CompiledCode_getSelector(_theContext, self->objects.method);
    printf("\tmethod: %p [#%.*s]pc: %d stackp: %d\n",
        (void*)self->objects.method,
        crankvm_string_printf_arg(methodSelector),
        (int)self->nextPC, (int)self->stackPointer);
    self->instructions = (uint8_t*)(self->objects.methodContext->method + sizeof(crankvm_object_header_t));
    self->currentBytecodeSetOffset = self->codeHeader.isAlternateBytecode ? 256 : 0;

    // Fetch the first bytecode.
    error = crankvm_interpreter_fetchNextInstruction(self);
    if(error)
        return error;

    return CRANK_VM_OK;
}

static inline crankvm_error_t
crankvm_interpreter_storeMethodContextState(crankvm_interpreter_state_t *self)
{
    crankvm_MethodContext_t *methodContext = self->objects.methodContext;

    // Store back the pc and the stack pointer
    methodContext->baseClass.pc = crankvm_oop_encodeSmallInteger(self->pc + 1);
    methodContext->stackp = crankvm_oop_encodeSmallInteger(self->stackPointer);

    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_localMethodReturnOop(crankvm_interpreter_state_t *self, crankvm_oop_t returnValue)
{
    // Store back some of the pointers.
    crankvm_interpreter_storeMethodContextState(self);

    crankvm_MethodContext_t *returnContext = (crankvm_MethodContext_t*)self->objects.methodContext->baseClass.sender;

    // Clear the sender of the context.
    self->objects.methodContext->baseClass.sender = crankvm_specialObject_nil(self->context);

    // If the return context is nil, then it is time to finish the interepreter
    if(crankvm_object_isNil(self->context, returnContext))
    {
        if(self->callerReturnValuePointer)
            *self->callerReturnValuePointer = returnValue;
        self->returnFromInterpreter = true;
        return CRANK_VM_OK;
    }

    // Activate the return context.
    printf("Returning into context: %p\n", returnContext);
    self->objects.methodContext = returnContext;
    crankvm_interpreter_fetchMethodContext(self);

    // Push the return value.
    pushOop(returnValue);
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_returnOopFromMethod(crankvm_interpreter_state_t *self, crankvm_oop_t oop)
{
    if(crankvm_oop_isNil(self->context, self->objects.methodContext->closureOrNil))
        return crankvm_interpreter_localMethodReturnOop(self, oop);

    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_returnOopFromBlock(crankvm_interpreter_state_t *self, crankvm_oop_t oop)
{
    return crankvm_interpreter_localMethodReturnOop(self, oop);
}

static crankvm_error_t
crankvm_interpreter_activateMethodWithArguments(crankvm_interpreter_state_t *self, int expectedArgumentCount, crankvm_oop_t methodOop)
{
    // Get the compiled method context.
    crankvm_compiled_code_header_t calledHeader;
    crankvm_error_t error = crankvm_specialObject_getCompiledCodeHeader(&calledHeader, (crankvm_CompiledCode_t*)methodOop);
    if(error)
        return error;

    // Check the number of arguments
    if(calledHeader.numberOfArguments != expectedArgumentCount)
        return CRANK_VM_ERROR_CALLED_METHOD_ARGUMENT_MISMATCH;

    // Compute the initial pc
    uintptr_t initialPC = (calledHeader.numberOfLiterals + 1) *sizeof(crankvm_oop_t) + 1;
    size_t compiledMethodSize = crankvm_object_header_getSmalltalkSize((crankvm_object_header_t*)methodOop);
    if(initialPC >= compiledMethodSize)
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    // Create the new context.
    crankvm_MethodContext_t *newContext = crankvm_MethodContext_create(_theContext, calledHeader.largeFrameRequired);

    // Set the sender context.
    newContext->baseClass.sender = (crankvm_oop_t)self->objects.methodContext;
    newContext->baseClass.pc = crankvm_oop_encodeSmallInteger(initialPC);
    newContext->method = methodOop;

    // Pop the arguments into the new context.
    for(int i = 0; i < expectedArgumentCount; ++i)
        newContext->stackSlots[expectedArgumentCount - i - 1] = popOop();

    // Pop the receiver into the new context.
    newContext->receiver = popOop();
    newContext->stackp = crankvm_oop_encodeSmallInteger(expectedArgumentCount + calledHeader.numberOfTemporaries);

    // Store the current method context
    crankvm_interpreter_storeMethodContextState(self);

    // Change into the new context.
    self->objects.methodContext = newContext;
    printf("Activating new context %p\n", newContext);
    return crankvm_interpreter_fetchMethodContext(self);
}

static crankvm_error_t
crankvm_interpreter_sendTo(crankvm_interpreter_state_t *self, int expectedArgumentCount, crankvm_oop_t selector)
{
    checkSizeToPop(expectedArgumentCount + 1);

    crankvm_oop_t receiver = crankvm_interpreter_stackOopAt(self, expectedArgumentCount);
    printf("Send #%.*s to %p\n", crankvm_string_printf_arg(selector), (void*)receiver);

    // Get the receiver class.
    crankvm_oop_t receiverClass = crankvm_object_getClass(_theContext, receiver);
    if(crankvm_oop_isNil(_theContext, receiverClass))
        return CRANK_VM_ERROR_RECEIVER_CLASS_NIL;

    // Lookup the selector
    crankvm_oop_t methodOop = crankvm_Behavior_lookupSelector(_theContext, (crankvm_Behavior_t*)receiverClass, selector);
    if(crankvm_oop_isNil(_theContext, methodOop))
    {
        printf("TODO: Send doesNotUnderstand:\n");
        UNIMPLEMENTED();
    }

    // TODO: Create the context, and invoke the method.
    if(!crankvm_oop_isCompiledCode(methodOop))
    {
        printf("TODO: Send to non-compiled method\n");
        UNIMPLEMENTED();
    }

    return crankvm_interpreter_activateMethodWithArguments(self, expectedArgumentCount, methodOop);
}

static crankvm_error_t
crankvm_interpreter_sendToSpecialSelector(crankvm_interpreter_state_t *self, int expectedArgumentCount, crankvm_special_selector_with_arg_count_t specialSelector)
{
    return crankvm_interpreter_sendTo(self, expectedArgumentCount, specialSelector.selector);
}

/// I am used for a primitive whose invocation context is inlined. (i.e. I do not create a new activation context for the primitive)
/*static crankvm_error_t
crankvm_interpreter_invokeNormalInlinedPrimitive(crankvm_interpreter_state_t *self, crankvm_primitive_function_t primitiveFunction)
{
    UNIMPLEMENTED();
}*/

static crankvm_error_t
crankvm_interpreter_invokeNormalPrimitive(crankvm_interpreter_state_t *self, crankvm_primitive_function_t primitiveFunction)
{
    // Create the primitive context
    crankvm_primitive_context_t primitiveContext = {
        .context = _theContext,
        .interpreter = self,
        .error = CRANK_VM_PRIMITIVE_SUCCESS,
        .argumentCount = self->codeHeader.numberOfArguments,
        .roots = {
            .arguments = &self->objects.methodContext->stackSlots[0],
            .receiver = self->objects.receiver,
            .result = _theContext->roots.nilOop
        },
    };

    // Call the primitive
    primitiveFunction(&primitiveContext);

    // Did we fail?
    if(primitiveContext.error)
    {
        crankvm_oop_t errorObject = crankvm_oop_encodeSmallInteger(primitiveContext.error);
        if(primitiveContext.error < CRANK_VM_PRIMITIVE_ERROR_KNOWN_COUNT)
            errorObject = _theSpecialObjectsArray->primitiveErrorTable->errorNameArray[primitiveContext.error - 1];

        // Store the error object in the first temporary.
        return crankvm_interpreter_setTemporary(self, self->codeHeader.numberOfArguments, errorObject);
    }

    // Handle the success case
    return crankvm_interpreter_localMethodReturnOop(self, primitiveContext.roots.result);
}

static crankvm_primitive_function_t
crankvm_interpreter_getNumberedPrimitive(crankvm_interpreter_state_t *self, unsigned int primitiveNumber)
{
    if(primitiveNumber >= crankvm_numberedPrimitiveTableSize)
    {
        printf("Using unexistent primitive %d\n", primitiveNumber);
        return crankvm_primitive_primitiveFailUnexistent;
    }

    crankvm_primitive_function_t result = crankvm_numberedPrimitiveTable[primitiveNumber];
    if(result)
        return result;

    printf("Using unexistent primitive %d\n", primitiveNumber);
    return crankvm_primitive_primitiveFailUnexistent;
}

static crankvm_error_t
crankvm_interpreter_invokeNumberedPrimitive(crankvm_interpreter_state_t *self, unsigned int primitiveNumber)
{
    return crankvm_interpreter_invokeNormalPrimitive(self, crankvm_interpreter_getNumberedPrimitive(self, primitiveNumber));
}

static crankvm_error_t
crankvm_interpreter_pushReceiverVariable(crankvm_interpreter_state_t *self, unsigned int slotIndex)
{
    checkReceiverSlotIndex(slotIndex);
    pushOop(crankvm_interpreter_getReceiverSlot(self, slotIndex));

    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_popStoreReceiverVariable(crankvm_interpreter_state_t *self, unsigned int slotIndex)
{
    checkReceiverSlotIndex(slotIndex);
    checkSizeToPop(1);
    crankvm_interpreter_setReceiverSlot(self, slotIndex, popOop());
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_pushTemporary(crankvm_interpreter_state_t *self, unsigned int temporaryIndex)
{
    checkTemporaryIndex(temporaryIndex);
    pushOop(crankvm_interpreter_getTemporary(self, temporaryIndex));

    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_popStoreTemporalVariable(crankvm_interpreter_state_t *self, unsigned int temporaryIndex)
{
    checkTemporaryIndex(temporaryIndex);
    checkSizeToPop(1);
    crankvm_interpreter_setTemporary(self, temporaryIndex, popOop());
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_pushLiteral(crankvm_interpreter_state_t *self, unsigned int literalIndex)
{
    checkLiteralIndex(literalIndex);
    pushOop(crankvm_interpreter_getLiteral(self, literalIndex));

    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_pushLiteralVariable(crankvm_interpreter_state_t *self, unsigned int literalIndex)
{
    checkLiteralIndex(literalIndex);
    crankvm_Association_t *literalVariable = (crankvm_Association_t *)crankvm_interpreter_getLiteral(self, literalIndex);
    if(crankvm_object_isNil(_theContext, literalVariable))
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    pushOop(literalVariable->value);
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_notBooleanMagic(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_jump(crankvm_interpreter_state_t *self, int delta)
{
    self->pc += delta;
    fetchNextInstruction();
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_jumpIfBoolean(crankvm_interpreter_state_t *self, intptr_t delta, crankvm_oop_t branchValue, crankvm_oop_t continueValue)
{
    fetchNextInstruction();
    checkSizeToPop(1);
    crankvm_oop_t value = popOop();
    if(value == branchValue)
    {
        self->pc += delta;
        fetchNextInstruction();
    }
    else if(value != continueValue)
    {
        return crankvm_interpreter_notBooleanMagic(self);
    }

    return CRANK_VM_OK;

}

static crankvm_error_t
crankvm_interpreter_jumpIfFalse(crankvm_interpreter_state_t *self, intptr_t delta)
{
    return crankvm_interpreter_jumpIfBoolean(self, delta, _theContext->roots.falseOop, _theContext->roots.trueOop);
}

static crankvm_error_t
crankvm_interpreter_jumpIfTrue(crankvm_interpreter_state_t *self, intptr_t delta)
{
    return crankvm_interpreter_jumpIfBoolean(self, delta, _theContext->roots.trueOop, _theContext->roots.falseOop);
}

// <editor-fold> Implementation of the bytecodes

static crankvm_error_t
crankvm_interpreter_bytecodePushReceiverVariableShort(crankvm_interpreter_state_t *self, int index)
{
    fetchNextInstruction();
    return crankvm_interpreter_pushReceiverVariable(self, index);
}

static crankvm_error_t
crankvm_interpreter_bytecodePushTempShort(crankvm_interpreter_state_t *self, int index)
{
    fetchNextInstruction();
    return crankvm_interpreter_pushTemporary(self, index);
}

static crankvm_error_t
crankvm_interpreter_bytecodePushLiteralShort(crankvm_interpreter_state_t *self, int index)
{
    fetchNextInstruction();
    return crankvm_interpreter_pushLiteral(self, index);
}

static crankvm_error_t
crankvm_interpreter_bytecodePushLiteralVariableShort(crankvm_interpreter_state_t *self, int index)
{
    fetchNextInstruction();
    return crankvm_interpreter_pushLiteralVariable(self, index);
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStoreReceiverVariableShort(crankvm_interpreter_state_t *self, int index)
{
    fetchNextInstruction();
    return crankvm_interpreter_popStoreReceiverVariable(self, index);
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStoreTemporalVariableShort(crankvm_interpreter_state_t *self, int index)
{
    fetchNextInstruction();
    return crankvm_interpreter_popStoreTemporalVariable(self, index);
}

static crankvm_error_t
crankvm_interpreter_bytecodePushReceiver(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop(self->objects.receiver);
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushTrue(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop(crankvm_specialObject_true(self->context));
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushFalse(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop(crankvm_specialObject_false(self->context));
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushNil(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop(crankvm_specialObject_nil(self->context));
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushMinusOne(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop(crankvm_oop_encodeSmallInteger(-1));
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushZero(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop(crankvm_oop_encodeSmallInteger(0));
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushOne(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop(crankvm_oop_encodeSmallInteger(1));
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushTwo(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop(crankvm_oop_encodeSmallInteger(2));
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnReceiver(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_returnOopFromMethod(self, self->objects.receiver);
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnTrue(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_returnOopFromMethod(self, crankvm_specialObject_true(self->context));
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnFalse(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_returnOopFromMethod(self, crankvm_specialObject_false(self->context));
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnNil(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_returnOopFromMethod(self, crankvm_specialObject_nil(self->context));
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnTop(crankvm_interpreter_state_t *self)
{
    checkSizeToPop(1);
    return crankvm_interpreter_returnOopFromMethod(self, popOop());
}

static crankvm_error_t
crankvm_interpreter_bytecodeBlockReturnTop(crankvm_interpreter_state_t *self)
{
    checkSizeToPop(1);
    return crankvm_interpreter_returnOopFromBlock(self, popOop());
}

static crankvm_error_t
crankvm_interpreter_bytecodeExtendedPush(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeExtendedStore(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeExtendedPopAndStore(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSingleExtendedSend(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeDoubleExtendedDoAnything(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSingleExtendedSuperSend(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSecondExtendedSend(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStackTop(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    checkSizeToPop(1);
    popOop();
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodeDuplicateStackTop(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    checkSizeToPop(1);
    pushOop(crankvm_interpreter_stackOopAt(self, 0));
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushThisContext(crankvm_interpreter_state_t *self)
{
    fetchNextInstruction();
    pushOop((crankvm_oop_t)self->objects.methodContext);
    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushOrPopNewArray(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeCallPrimitive(crankvm_interpreter_state_t *self)
{
    unsigned int primitiveNumber = crankvm_interpreter_fetchByte(self) + (crankvm_interpreter_fetchByte(self)<<8);
    fetchNextInstruction();

    // Is this an inline primitive?
    if(primitiveNumber & 0x8000)
    {
        printf("TODO: Invoke inline primitive: %d\n", primitiveNumber);
        return CRANK_VM_ERROR_ILLEGAL_INSTRUCTION;
    }

    return crankvm_interpreter_invokeNumberedPrimitive(self, primitiveNumber);
}

static crankvm_error_t
crankvm_interpreter_bytecodePushRemoteTempLong(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeStoreRemoteTempLong(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStoreRemoteTempLong(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushClosureCopyCopiedValues(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeShortJump(crankvm_interpreter_state_t *self, unsigned int extraDelta)
{
    return crankvm_interpreter_jump(self, 1 + extraDelta);
}

static crankvm_error_t
crankvm_interpreter_bytecodeShortJumpIfFalse(crankvm_interpreter_state_t *self, unsigned int extraDelta)
{
    return crankvm_interpreter_jumpIfFalse(self, 1 + extraDelta);
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJump(crankvm_interpreter_state_t *self, int i)
{
    intptr_t delta = ((i - 4) << 8) | crankvm_interpreter_fetchByte(self);
    return crankvm_interpreter_jump(self, delta);
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJumpIfFalse(crankvm_interpreter_state_t *self, int i)
{
    intptr_t delta = (i << 8) | crankvm_interpreter_fetchByte(self);
    return crankvm_interpreter_jumpIfFalse(self, delta);
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJumpIfTrue(crankvm_interpreter_state_t *self, int i)
{
    intptr_t delta = (i << 8) | crankvm_interpreter_fetchByte(self);
    return crankvm_interpreter_jumpIfTrue(self, delta);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageAdd(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->add);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageMinus(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->subtract);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageLessThan(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->lessThan);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageGreaterThan(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->greaterThan);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageLessEqual(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->lessOrEqual);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageGreaterEqual(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->greaterOrEqual);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageEqual(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->equal);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageNotEqual(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->notEqual);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageMultiply(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->multiply);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageDivide(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->divide);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageRemainder(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->remainder);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageMakePoint(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->makePoint);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageBitShift(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->bitShift);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageIntegerDivision(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->integerDivide);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageBitAnd(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->bitAnd);
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageBitOr(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->bitOr);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageAt(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->at);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageAtPut(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->atPut);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageSize(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->size);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageNext(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->next);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageNextPut(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->nextPut);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageAtEnd(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->atEnd);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageIdentityEqual(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->identityEquals);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageClass(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->clazz);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageBlockCopy(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->blockCopy);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageValue(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->value);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageValueArg(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->valueWithArg);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageDo(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->doBlock);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageNew(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->newObject);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageNewArray(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->newObjectWithSize);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageX(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->x);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageY(crankvm_interpreter_state_t *self)
{
    return crankvm_interpreter_sendToSpecialSelector(self, 1, _theSpecialSelectors->y);
}

static crankvm_error_t
crankvm_interpreter_bytecodeSendShortArgs0(crankvm_interpreter_state_t *self, int selectorIndex)
{
    checkLiteralIndex(selectorIndex);
    return crankvm_interpreter_sendTo(self, 0, crankvm_interpreter_getLiteral(self, selectorIndex));
}

static crankvm_error_t
crankvm_interpreter_bytecodeSendShortArgs1(crankvm_interpreter_state_t *self, int selectorIndex)
{
    checkLiteralIndex(selectorIndex);
    return crankvm_interpreter_sendTo(self, 1, crankvm_interpreter_getLiteral(self, selectorIndex));
}

static crankvm_error_t
crankvm_interpreter_bytecodeSendShortArgs2(crankvm_interpreter_state_t *self, int selectorIndex)
{
    checkLiteralIndex(selectorIndex);
    return crankvm_interpreter_sendTo(self, 2, crankvm_interpreter_getLiteral(self, selectorIndex));
}

// </editor-fold> End of implementation of the bytecodes.

crankvm_error_t
crankvm_interpreter_run(crankvm_interpreter_state_t *self)
{
    if(self->callerReturnValuePointer)
        *self->callerReturnValuePointer = crankvm_specialObject_nil(self->context);

    // Validate and fetch the method context
    crankvm_error_t error = crankvm_interpreter_fetchMethodContext(self);
    if(error)
        return error;

    // Fetch the first instruction.
    self->returnFromInterpreter = false;
    while(!self->returnFromInterpreter)
    {
        self->currentBytecode = self->nextBytecode;
        self->pc = self->nextPC;

        printf("Bytecode: %02X\n", self->currentBytecode);

        switch(self->currentBytecode + self->currentBytecodeSetOffset)
        {
#define BYTECODE_DISPATCH_NAME_ARGS(name, ...) error = crankvm_interpreter_bytecode ## name (self, __VA_ARGS__)
#define BYTECODE_DISPATCH_NAME(name) error = crankvm_interpreter_bytecode ## name (self)

// SqueakV3Plus closures bytecode set
#define BYTECODE_TABLE_OFFSET 0
#include "SqueakV3PlusClosuresBytecodeSetDispatchTable.inc"
#undef BYTECODE_TABLE_OFFSET

// SistaV1 set
//#define BYTECODE_TABLE_OFFSET 256
//#include "SistaV1BytecodeSetDispatchTable.inc"
//#undef BYTECODE_TABLE_OFFSET

#undef BYTECODE_DISPATCH_NAME_ARGS
#undef BYTECODE_DISPATCH_NAME

        default:
            return CRANK_VM_OK;
        }

        if(error)
            return error;
    }

    return CRANK_VM_OK;
}

crankvm_error_t
crankvm_interpret(crankvm_context_t *context, crankvm_MethodContext_t *methodContext, crankvm_oop_t *callerReturnValuePointer)
{
    crankvm_interpreter_state_t state;
    memset(&state, 0, sizeof(state));
    state.objects.methodContext = methodContext;
    state.context = context;
    state.callerReturnValuePointer = callerReturnValuePointer;

    return crankvm_interpreter_run(&state);
}
