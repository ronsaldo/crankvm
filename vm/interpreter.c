#include "crank-vm/crank-vm.h"
#include "context-internal.h"
#include "heap.h"
#include <string.h>

#define UNIMPLEMENTED_BYTECODE() \
do { \
    fprintf(stderr, "Unimplemented bytecode %s in %s:%d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
    abort(); \
} while(false)

typedef struct crankvm_interpreter_state_s
{
    crankvm_context_t *context;
    crankvm_MethodContext_t *methodContext;
    crankvm_oop_t *callerReturnValuePointer;

    intptr_t stackPointer;
    uint8_t *instructions;

    intptr_t pc;
    int currentBytecode;

    intptr_t nextPC;
    int nextBytecode;
} crankvm_interpreter_state_t;

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
    crankvm_error_t error = crankvm_MethodContext_validate(self->context, self->methodContext);
    if(error)
        return error;

    self->nextPC = crankvm_oop_decodeSmallInteger(self->methodContext->baseClass.pc);
    self->stackPointer = crankvm_oop_decodeSmallInteger(self->methodContext->stackp);
    if(self->nextPC <= 0)
        return CRANK_VM_ERROR_INVALID_PARAMETER;
    --self->nextPC;

    // Get the pointer into the instructions.
    printf("\tmethod: %p pc: %d stackp: %d\n", (void*)self->methodContext->method, (int)self->pc, (int)self->stackPointer);
    self->instructions = (uint8_t*)(self->methodContext->method + sizeof(crankvm_object_header_t));

    // Fetch the first bytecode.
    error = crankvm_interpreter_fetchNextInstruction(self);
    if(error)
        return error;

    return CRANK_VM_OK;
}

static crankvm_error_t
crankvm_interpreter_bytecodePushReceiverVariableShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushTempShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushLiteralShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushLiteralVariableShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStoreReceiverVariableShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStoreTemporalVariableShort(crankvm_interpreter_state_t *self, int index)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushReceiver(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushTrue(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushFalse(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushNil(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushMinusOne(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushZero(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushOne(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushTwo(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnReceiver(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnTrue(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnFalse(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnNil(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeReturnTop(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeBlockReturnTop(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeExtendedPush(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeExtendedStore(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeExtendedPopAndStore(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSingleExtendedSend(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeDoubleExtendedDoAnything(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSingleExtendedSuperSend(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSecondExtendedSend(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStackTop(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeDuplicateStackTop(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushThisContext(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushOrPopNewArray(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeCallPrimitive(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushRemoteTempLong(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeStoreRemoteTempLong(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePopStoreRemoteTempLong(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodePushClosureCopyCopiedValues(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeShortJump(crankvm_interpreter_state_t *self, int extraDelta)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeShortJumpIfFalse(crankvm_interpreter_state_t *self, int extraDelta)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJump(crankvm_interpreter_state_t *self, int i)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJumpIfFalse(crankvm_interpreter_state_t *self, int i)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeLongJumpIfTrue(crankvm_interpreter_state_t *self, int i)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageAdd(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageMinus(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageLessThan(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageGreaterThan(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageLessEqual(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageGreaterEqual(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageEqual(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageNotEqual(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageMultiply(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageDivide(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageRemainder(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageMakePoint(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageBitShift(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageIntegerDivision(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageBitAnd(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeArithmeticMessageBitOr(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}


static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageAt(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageAtPut(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageSize(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageNext(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageNextPut(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageAtEnd(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageIdentityEqual(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageClass(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageBlockCopy(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageValue(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageValueArg(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageDo(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageNew(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageNewArray(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageX(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSpecialMessageY(crankvm_interpreter_state_t *self)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSendShortArgs0(crankvm_interpreter_state_t *self, int selectorIndex)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSendShortArgs1(crankvm_interpreter_state_t *self, int selectorIndex)
{
    UNIMPLEMENTED_BYTECODE();
}

static crankvm_error_t
crankvm_interpreter_bytecodeSendShortArgs2(crankvm_interpreter_state_t *self, int selectorIndex)
{
    UNIMPLEMENTED_BYTECODE();
}

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
    for(;;)
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
    state.methodContext = methodContext;
    state.context = context;
    state.callerReturnValuePointer = callerReturnValuePointer;

    return crankvm_interpreter_run(&state);
}
