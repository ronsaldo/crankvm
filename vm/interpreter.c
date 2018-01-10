#include "crank-vm/crank-vm.h"
#include "context-internal.h"
#include "heap.h"
#include <string.h>
#include <assert.h>

typedef struct crankvm_interpreter_state_s
{
    crankvm_context_t *context;
    crankvm_oop_t *callerReturnValuePointer;

    intptr_t stackPointer;
    intptr_t stackLimit;
    uint8_t *instructions;

    intptr_t pc;
    int currentBytecode;

    intptr_t nextPC;
    int nextBytecode;
    bool returnFromInterpreter;

    struct
    {
        crankvm_MethodContext_t *methodContext;
        crankvm_oop_t receiver;
    } objects;

} crankvm_interpreter_state_t;

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

#define popOop() crankvm_interpreter_popOop(self)

#define _theContext (self->context)
#define _theSpecialObjectsArray (_theContext->specialObjectsArray)
#define _theSpecialSelectors (_theSpecialObjectsArray->specialSelectors)

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_CompiledCode_validate(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode)
{
    if(crankvm_context_isNilOrNull(context, compiledCode))
        return CRANK_VM_ERROR_NULL_POINTER;

    return CRANK_VM_OK;
}

LIB_CRANK_VM_EXPORT crankvm_error_t
crankvm_MethodContext_validate(crankvm_context_t *context, crankvm_MethodContext_t *methodContext)
{
    if(crankvm_context_isNilOrNull(context, methodContext))
        return CRANK_VM_ERROR_NULL_POINTER;

    if(!crankvm_oop_isSmallInteger(methodContext->baseClass.pc) ||
        !crankvm_oop_isSmallInteger(methodContext->stackp))
        return CRANK_VM_ERROR_INVALID_PARAMETER;

    return CRANK_VM_OK;
}

static inline crankvm_error_t
crankvm_interpreter_fetchNextInstruction(crankvm_interpreter_state_t *self)
{
    self->nextBytecode = self->instructions[self->nextPC];
    return CRANK_VM_OK;
}

static inline crankvm_error_t
crankvm_interpreter_fetchMethodContext(crankvm_interpreter_state_t *self)
{
    // Validate the context
    crankvm_error_t error = crankvm_MethodContext_validate(self->context, self->objects.methodContext);
    if(error)
        return error;

    self->objects.receiver = self->objects.methodContext->receiver;

    self->nextPC = crankvm_oop_decodeSmallInteger(self->objects.methodContext->baseClass.pc);
    self->stackPointer = crankvm_oop_decodeSmallInteger(self->objects.methodContext->stackp);
    self->stackLimit = crankvm_object_header_getSlotCount((crankvm_object_header_t *)self->objects.methodContext);
    if(self->nextPC <= 0 || self->stackPointer > self->stackLimit)
        return CRANK_VM_ERROR_INVALID_PARAMETER;
    --self->nextPC;

    // Get the pointer into the instructions.
    printf("\tmethod: %p pc: %d stackp: %d\n", (void*)self->objects.methodContext->method, (int)self->pc, (int)self->stackPointer);
    self->instructions = (uint8_t*)(self->objects.methodContext->method + sizeof(crankvm_object_header_t));

    // Fetch the first bytecode.
    error = crankvm_interpreter_fetchNextInstruction(self);
    if(error)
        return error;

    return CRANK_VM_OK;
}

static inline crankvm_error_t
crankvm_interpreter_storeMethodContextState(crankvm_interpreter_state_t *self)
{
    // Store back the stack pointer and the pc
    self->objects.methodContext->stackp = crankvm_oop_encodeSmallInteger(self->stackPointer);
    self->objects.methodContext->baseClass.pc = crankvm_oop_encodeSmallInteger(self->nextPC + 1);

    return CRANK_VM_OK;
}

static inline crankvm_error_t
crankvm_interpreter_pushOop(crankvm_interpreter_state_t *self, crankvm_oop_t oop)
{
    if(self->stackPointer >= self->stackLimit)
        return CRANK_VM_ERROR_STACK_OVERFLOW;

    self->objects.methodContext->stackSlots[self->stackPointer++] = oop;
    return CRANK_VM_OK;
}

static inline crankvm_error_t
crankvm_interpreter_checkSizeToPop(crankvm_interpreter_state_t *self, intptr_t size)
{
    if(self->stackPointer - size < 0)
        return CRANK_VM_ERROR_STACK_UNDERFLOW;

    return CRANK_VM_OK;
}

static inline crankvm_oop_t
crankvm_interpreter_popOop(crankvm_interpreter_state_t *self)
{
    assert(self->stackPointer > 0);
    return self->objects.methodContext->stackSlots[--self->stackPointer];
}

static inline crankvm_oop_t
crankvm_interpreter_stackOopAt(crankvm_interpreter_state_t *self, intptr_t size)
{
    assert(self->stackPointer > size);
    return self->objects.methodContext->stackSlots[self->stackPointer - size - 1];
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
    if(crankvm_context_isNil(self->context, returnContext))
    {
        if(self->callerReturnValuePointer)
            *self->callerReturnValuePointer = returnValue;
        self->returnFromInterpreter = true;
        return CRANK_VM_OK;
    }

    UNIMPLEMENTED();
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
crankvm_interpreter_sendTo(crankvm_interpreter_state_t *self, int expectedArgumentCount, crankvm_oop_t selector)
{
    checkSizeToPop(expectedArgumentCount + 1);

    crankvm_oop_t receiver = crankvm_interpreter_stackOopAt(self, expectedArgumentCount);
    printf("TODO: Send #%.*s to %p\n", crankvm_string_printf_arg(selector), (void*)receiver);
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_sendToSpecialSelector(crankvm_interpreter_state_t *self, int expectedArgumentCount, crankvm_special_selector_with_arg_count_t specialSelector)
{
    return crankvm_interpreter_sendTo(self, expectedArgumentCount, specialSelector.selector);
}

// <editor-fold> Implementation of the bytecodes

static crankvm_error_t
crankvm_interpreter_bytecodePushReceiverVariableShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushTempShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushLiteralShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushLiteralVariableShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStoreReceiverVariableShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStoreTemporalVariableShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED();
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
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeDuplicateStackTop(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushThisContext(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushOrPopNewArray(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeCallPrimitive(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED();
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
crankvm_interpreter_bytecodeShortJump(crankvm_interpreter_state_t *self, int extraDelta)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeShortJumpIfFalse(crankvm_interpreter_state_t *self, int extraDelta)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJump(crankvm_interpreter_state_t *self, int i)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJumpIfFalse(crankvm_interpreter_state_t *self, int i)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJumpIfTrue(crankvm_interpreter_state_t *self, int i)
{
    UNIMPLEMENTED();
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
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSendShortArgs1(crankvm_interpreter_state_t *self, int selectorIndex)
{
    UNIMPLEMENTED();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSendShortArgs2(crankvm_interpreter_state_t *self, int selectorIndex)
{
    UNIMPLEMENTED();
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
        self->pc = self->nextPC++;
        crankvm_interpreter_fetchNextInstruction(self);

        printf("Bytecode: %02X\n", self->currentBytecode);

        switch(self->currentBytecode)
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
    state.objects.receiver = crankvm_specialObject_nil(context);
    state.context = context;
    state.callerReturnValuePointer = callerReturnValuePointer;

    return crankvm_interpreter_run(&state);
}
