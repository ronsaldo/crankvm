#ifndef CRANK_VM_SPECIAL_OBJECTS_H
#define CRANK_VM_SPECIAL_OBJECTS_H

#include <crank-vm/objectmodel.h>
#include <crank-vm/error.h>

typedef enum crankvm_class_index_pun_e
{
    CRANKVM_CLASS_INDEX_PUN_FORWARDED = 8,
    CRANKVM_CLASS_INDEX_PUN_FREE_OBJECT = 0,
    CRANKVM_CLASS_INDEX_PUN_SEGMENT_BRIDGE = 3,

    CRANKVM_CLASS_INDEX_PUN_ARRAY_CLASS = 16,
    CRANKVM_CLASS_INDEX_PUN_WEAK_ARRAY_CLASS = 17,
    CRANKVM_CLASS_INDEX_PUN_64_BITS_LONG_CLASS = 19,
    CRANKVM_CLASS_INDEX_PUN_32_BITS_LONG_CLASS = 18,

#ifdef CRANK_VM_64_BITS
    CRANKVM_CLASS_INDEX_PUN_WORD_SIZE_BITS_LONG_CLASS = CRANKVM_CLASS_INDEX_PUN_64_BITS_LONG_CLASS,
#else
    CRANKVM_CLASS_INDEX_PUN_WORD_SIZE_BITS_LONG_CLASS = CRANKVM_CLASS_INDEX_PUN_32_BITS_LONG_CLASS,
#endif
    CRANKVM_CLASS_INDEX_PUN_IS_ITSELF_CLASS = 31,

    CRANKVM_CLASS_INDEX_PUN_FIRST = CRANKVM_CLASS_INDEX_PUN_ARRAY_CLASS,
    CRANKVM_CLASS_INDEX_PUN_LAST = CRANKVM_CLASS_INDEX_PUN_IS_ITSELF_CLASS,
} crankvm_class_index_pun_t;

#define CRANK_VM_CLASS_TABLE_PAGE_BITS 10
#define CRANK_VM_CLASS_TABLE_PAGE_SIZE (1<<CRANK_VM_CLASS_TABLE_PAGE_BITS)
#define CRANK_VM_CLASS_TABLE_PAGE_MASK (CRANK_VM_CLASS_TABLE_PAGE_SIZE - 1)

#define CRANK_VM_CLASS_TABLE_ROOT_COUNT (1<<(22 - CRANK_VM_CLASS_TABLE_PAGE_BITS))

#define CRANK_VM_OBJECT_STACK_PAGE_SLOTS 40962

#define CRANK_VM_METHOD_CONTEXT_SMALL_FRAME_SIZE 16
#define CRANK_VM_METHOD_CONTEXT_LARGE_FRAME_SIZE 56

/**
 * Hidden special object layout
 */
typedef struct crankvm_HiddenSpecialObject_s
{
    crankvm_object_header_t objectHeader;
    /* Here goes the slots*/
} crankvm_HiddenSpecialObject_t;

/**
 * ProtoObject layout
 */
typedef struct crankvm_ProtoObject_s
{
    crankvm_object_header_t objectHeader;
    /* Here goes the slots*/
} crankvm_ProtoObject_t;

typedef crankvm_ProtoObject_t crankvm_Object_t;
typedef crankvm_Object_t crankvm_Collection_t;

typedef struct crankvm_AdditionalMethodState_s
{
    crankvm_Object_t baseClass;
    crankvm_oop_t method;
    crankvm_oop_t selector;
} crankvm_AdditionalMethodState_t;

typedef struct crankvm_CompiledCode_s
{
    crankvm_Object_t baseClass;
    crankvm_oop_t codeHeader;
    crankvm_oop_t literals[];
} crankvm_CompiledCode_t;

/**
 * Array layout
 */
typedef struct crankvm_Array_s
{
    crankvm_Object_t baseClass;

    crankvm_oop_t slots[];
} crankvm_Array_t;

/**
 * HashedCollection
 */
typedef struct crankvm_MethodDictionary_s
{
    crankvm_Collection_t baseClass;
    crankvm_oop_t tally;
    crankvm_Array_t *array;
    crankvm_oop_t keys[];
} crankvm_MethodDictionary_t;

/**
 * Behavior layout
 */
typedef struct crankvm_Behavior_s
{
    crankvm_Object_t baseClass;

    struct crankvm_Behavior_s *superclass;
    crankvm_MethodDictionary_t *methodDict;
    crankvm_oop_t format;
    crankvm_oop_t layout;
} crankvm_Behavior_t;

/**
 * ClassDescription layout
 */
typedef struct crankvm_ClassDescription_s
{
    crankvm_Behavior_t baseClass;

    crankvm_oop_t instanceVariables;
    crankvm_oop_t organization;
} crankvm_ClassDescription_t;

/**
 * Class layout - for debugging purposes
 */
typedef struct crankvm_Class_s
{
    crankvm_ClassDescription_t baseClass;

    crankvm_oop_t subclasses;
    crankvm_oop_t name;

    // We only care about the name, for debugging purposes
} crankvm_Class_t;

/**
 * Metaclass layout - for debugging purposes
 */
typedef struct crankvm_Metaclass_s
{
    crankvm_ClassDescription_t baseClass;

    crankvm_Class_t *thisClass;

    // We only care about the name, for debugging purposes
} crankvm_Metaclass_t;

/**
 * ClassTablePage layout
 */
typedef struct crankvm_ClassTablePage_s
{
    crankvm_HiddenSpecialObject_t baseClass;

    crankvm_oop_t classes[];
} crankvm_ClassTablePage_t;

/**
 * ObjectStack layout
  */
typedef struct crankvm_ObjectStackPage_s
{
    crankvm_HiddenSpecialObject_t baseClass;

    uintptr_t top; // Stack pointer. 0 Means empty
    uintptr_t my;
    uintptr_t free;
    uintptr_t next;

    uintptr_t elements[CRANK_VM_OBJECT_STACK_PAGE_SLOTS - 4];
} crankvm_ObjectStackPage_t;

/**
 * HiddenRoots layout
 */
typedef struct crankvm_HiddenRoots_s
{
    crankvm_HiddenSpecialObject_t baseClass;

    crankvm_ClassTablePage_t *classTablePages[CRANK_VM_CLASS_TABLE_ROOT_COUNT];

    crankvm_ObjectStackPage_t *markStackRoot;
    crankvm_ObjectStackPage_t *weaklingStackRoot;
    crankvm_ObjectStackPage_t *mournQueueRoot;
    crankvm_oop_t rememberedSetRoot;
} crankvm_HiddenRoots_t;

/**
 * InstructionStream
 */
typedef struct crankvm_InstructionStream_s
{
    crankvm_Object_t baseClass;

    crankvm_oop_t sender;
    crankvm_oop_t pc;
} crankvm_InstructionStream_t;

/**
 * MethodContext (Context in Pharo)
 */
typedef struct crankvm_MethodContext_s
{
    crankvm_InstructionStream_t baseClass;

    crankvm_oop_t stackp;
    crankvm_oop_t method;
    crankvm_oop_t closureOrNil;
    crankvm_oop_t receiver;

    crankvm_oop_t stackSlots[];
} crankvm_MethodContext_t;

/**
 * Association
 */
typedef struct crankvm_Association_s
{
    crankvm_Object_t baseClass;

    crankvm_oop_t key;
    crankvm_oop_t value;
} crankvm_Association_t;

/**
 * ProcessorScheduler
 */
typedef struct crankvm_ProcessorScheduler_s
{
    crankvm_Object_t baseClass;

    crankvm_oop_t quiescentProcessLists;
    crankvm_oop_t activeProcess;
} crankvm_ProcessorScheduler_t;

/**
 * Link
 */
typedef struct crankvm_Link_s
{
    crankvm_Object_t baseClass;

    crankvm_oop_t next;
} crankvm_Link_t;

/**
 * Process
 */
typedef struct crankvm_Process_s
{
    crankvm_Link_t baseClass;

    crankvm_oop_t suspendedContext;
    crankvm_oop_t priority;
    crankvm_oop_t myList;
    crankvm_oop_t name;
    crankvm_oop_t env;
    crankvm_oop_t effectiveProcess;
    crankvm_oop_t terminating;
} crankvm_Process_t;

typedef struct crankvm_special_selector_with_arg_count_s
{
    crankvm_oop_t selector;
    crankvm_oop_t argumentCountOop;
} crankvm_special_selector_with_arg_count_t;

typedef struct crankvm_special_selectors_s
{
    crankvm_Object_t baseClass;

    // Arithmetic selectors

    //#+ 1 #- 1 #< 1 #> 1 #<= 1 #>= 1 #= 1 #~= 1
    crankvm_special_selector_with_arg_count_t add;
    crankvm_special_selector_with_arg_count_t subtract;
    crankvm_special_selector_with_arg_count_t lessThan;
    crankvm_special_selector_with_arg_count_t greaterThan;
    crankvm_special_selector_with_arg_count_t lessOrEqual;
    crankvm_special_selector_with_arg_count_t greaterOrEqual;
    crankvm_special_selector_with_arg_count_t equal;
    crankvm_special_selector_with_arg_count_t notEqual;

	//#* 1 #/ 1 #\\ 1 #@ 1 #bitShift: 1 #// 1 #bitAnd: 1 #bitOr: 1
    crankvm_special_selector_with_arg_count_t multiply;
    crankvm_special_selector_with_arg_count_t divide;
    crankvm_special_selector_with_arg_count_t remainder;
    crankvm_special_selector_with_arg_count_t makePoint;
    crankvm_special_selector_with_arg_count_t bitShift;
    crankvm_special_selector_with_arg_count_t integerDivide;
    crankvm_special_selector_with_arg_count_t bitAnd;
    crankvm_special_selector_with_arg_count_t bitOr;

    //#at: 1 #at:put: 2 #size 0 #next 0 #nextPut: 1 #atEnd 0 #== 1 #class 0
    crankvm_special_selector_with_arg_count_t at;
    crankvm_special_selector_with_arg_count_t atPut;
    crankvm_special_selector_with_arg_count_t size;
    crankvm_special_selector_with_arg_count_t next;
    crankvm_special_selector_with_arg_count_t nextPut;
    crankvm_special_selector_with_arg_count_t atEnd;
    crankvm_special_selector_with_arg_count_t identityEquals;
    crankvm_special_selector_with_arg_count_t clazz;

	//#blockCopy: 1 #value 0 #value: 1 #do: 1 #new 0 #new: 1 #x 0 #y 0
    crankvm_special_selector_with_arg_count_t blockCopy;
    crankvm_special_selector_with_arg_count_t value;
    crankvm_special_selector_with_arg_count_t valueWithArg;
    crankvm_special_selector_with_arg_count_t doBlock;
    crankvm_special_selector_with_arg_count_t newObject;
    crankvm_special_selector_with_arg_count_t newObjectWithSize;
    crankvm_special_selector_with_arg_count_t x;
    crankvm_special_selector_with_arg_count_t y;

} crankvm_special_selectors_t;

typedef enum crankvm_primitive_error_code_e
{
    CRANK_VM_PRIMITIVE_SUCCESS = 0,

    CRANK_VM_PRIMITIVE_ERROR,
    CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER,
    CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT,
    CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX,
    CRANK_VM_PRIMITIVE_ERROR_BAD_NUMBER_OF_ARGUMENTS,
    CRANK_VM_PRIMITIVE_ERROR_INAPPROPIATE_OPERATION,
    CRANK_VM_PRIMITIVE_ERROR_UNSUPPORTED_OPERATION,
    CRANK_VM_PRIMITIVE_ERROR_NO_MODIFICATION,
    CRANK_VM_PRIMITIVE_ERROR_INSUFFICIENT_OBJECT_MEMORY,
    CRANK_VM_PRIMITIVE_ERROR_INSUFFICIENT_C_MEMORY,
    CRANK_VM_PRIMITIVE_ERROR_NOT_FOUND,
    CRANK_VM_PRIMITIVE_ERROR_BAD_METHOD,
    CRANK_VM_PRIMITIVE_ERROR_INTERNAL_ERROR_IN_NAMED_PRIMITIVE_MACHINERY,
    CRANK_VM_PRIMITIVE_ERROR_OBJECT_MAY_MOVE,
    CRANK_VM_PRIMITIVE_ERROR_RESOURCE_LIMIT_EXCEEDED,
    CRANK_VM_PRIMITIVE_ERROR_OBJECT_IS_PINNED,
    CRANK_VM_PRIMITIVE_ERROR_WRITE_BEYOND_END_OF_OBJECT,
    CRANK_VM_PRIMITIVE_ERROR_OBJECT_MOVED,
    CRANK_VM_PRIMITIVE_ERROR_OBJECT_NOT_PINNED,
    CRANK_VM_PRIMITIVE_ERROR_CALLBACK_ERROR,

    CRANK_VM_PRIMITIVE_ERROR_LAST_KNOWN,
} crankvm_primitive_error_code_t;

typedef struct crankvm_primitive_error_table_s
{
    crankvm_Object_t baseClass;

    union
    {
        struct
        {
            crankvm_oop_t error;
            crankvm_oop_t badReceiver;
            crankvm_oop_t badArgument;
            crankvm_oop_t badIndex;
            crankvm_oop_t badNumberOfArgumnets;
            crankvm_oop_t inappropiateOperation;
            crankvm_oop_t unsupportedOperation;
            crankvm_oop_t noModification;
            crankvm_oop_t insufficientObjectMemory;
            crankvm_oop_t insufficientCMemory;
            crankvm_oop_t notFound;
            crankvm_oop_t badMethod;
            crankvm_oop_t internalErrorInNamedPrimitiveMachinery;
            crankvm_oop_t objectMayMove;
            crankvm_oop_t resourceLimitExceeded;
            crankvm_oop_t objectIsPinned;
            crankvm_oop_t primitiveWriteBeyondEndOfObject;
            crankvm_oop_t objectMoved;
            crankvm_oop_t objectNotPinned;
            crankvm_oop_t callbackError;
        };

        crankvm_oop_t errorNameArray[CRANK_VM_PRIMITIVE_ERROR_LAST_KNOWN];
    };

} primitive_error_table_t;

typedef struct crankvm_special_object_array_s
{
    crankvm_object_header_t objectHeader;

    crankvm_oop_t nilObject; // 0
	crankvm_oop_t falseObject; // 1
	crankvm_oop_t trueObject; // 2

	crankvm_Association_t *schedulerAssociation; // 3

    crankvm_Behavior_t *classBitmap; // 4
	crankvm_Behavior_t *classSmallInteger; // 5
	crankvm_Behavior_t *classByteString; // 6 ClassString "N.B.  Actually class ByteString"
	crankvm_Behavior_t *classArray; // 7.
	crankvm_Behavior_t *old_SmalltalkDictionary; // 8."  "Do not delete!"
	crankvm_Behavior_t *classFloat;// 9.
	crankvm_Behavior_t *classMethodContext;// 10.
	crankvm_Behavior_t *unused_classBlockContext;// 11. unused by the VM"
	crankvm_Behavior_t *classPoint; //12.
	crankvm_Behavior_t *classLargePositiveInteger; //13.

    crankvm_oop_t TheDisplay; //14.

    crankvm_Behavior_t *classMessage; //15.
	crankvm_oop_t unused_classCompiledMethod; //16. unused by the VM"

    crankvm_oop_t TheLowSpaceSemaphore; //17.

    crankvm_Behavior_t *classSemaphore; //18.
	crankvm_Behavior_t *classCharacter; //19.

    crankvm_oop_t selectorDoesNotUnderstand; //20.
	crankvm_oop_t selectorCannotReturn; //21.
	crankvm_oop_t processSignalingLowSpace; //22.	"was TheInputSemaphore"
	crankvm_special_selectors_t *specialSelectors; //23.

    crankvm_oop_t characterTable; //24 nil.	"Must be unused by the VM"
	crankvm_oop_t selectorMustBeBoolean; //25.
	crankvm_Behavior_t *classByteArray; //26.
	crankvm_oop_t unused_ClassProcess;//; //27. unused"
	crankvm_oop_t compactClasses; //28.
	crankvm_oop_t theTimerSemaphore; //29.
	crankvm_oop_t theInterruptSemaphore; //30.

    crankvm_oop_t unknown_1; //31.
    crankvm_oop_t unknown_2; //32.
    crankvm_oop_t unknown_3; //33.

	crankvm_oop_t selectorCannotInterpret; //34.
	crankvm_oop_t old_MethodContextProto; //35.
	crankvm_oop_t classBlockClosure; //36.
	crankvm_oop_t old_BlockContextProto; //37.
	crankvm_oop_t externalObjectsArray; //38.
	crankvm_oop_t classMutex; //39.
	crankvm_oop_t processInExternalCodeTag; //40.
	crankvm_oop_t theFinalizationSemaphore; //41.
	crankvm_oop_t classLargeNegativeInteger; //42.

	crankvm_Behavior_t *classExternalAddress; //43.
	crankvm_Behavior_t *classExternalStructure; //44.
	crankvm_Behavior_t *classExternalData; //45.
	crankvm_Behavior_t *classExternalFunction; //46.
	crankvm_Behavior_t *classExternalLibrary; //47.

	crankvm_oop_t selectorAboutToReturn; //48.
	crankvm_oop_t selectorRunWithIn; //49.

	crankvm_oop_t selectorAttemptToAssign; //50.
	primitive_error_table_t *primitiveErrorTable; //51. in VMClass class>>initializePrimitiveErrorCodes"
	crankvm_Behavior_t *classAlien; //52.
	crankvm_oop_t selectorInvokeCallback; //53.
	crankvm_Behavior_t *classUnsafeAlien; //54.

	crankvm_Behavior_t *classWeakFinalizer; //55.

	crankvm_oop_t foreignCallbackProcess; //56.

	crankvm_oop_t selectorUnknownBytecode; //57.
	crankvm_oop_t selectorCounterTripped; //58.
	crankvm_oop_t selectorSistaTrap; //59.

	crankvm_oop_t lowcodeContextMark; //60.
	crankvm_oop_t lowcodeNativeContextClass; //61.

    crankvm_Association_t *crankVMEntryContext;
} crankvm_special_object_array_t;

typedef struct crankvm_context_s crankvm_context_t;

LIB_CRANK_VM_EXPORT crankvm_special_object_array_t *crankvm_context_getSpecialObjectsArray(crankvm_context_t *context);

static inline crankvm_oop_t
crankvm_specialObject_nil(crankvm_context_t *context)
{
    return crankvm_context_getSpecialObjectsArray(context)->nilObject;
}

static inline crankvm_oop_t
crankvm_specialObject_true(crankvm_context_t *context)
{
    return crankvm_context_getSpecialObjectsArray(context)->trueObject;
}

static inline crankvm_oop_t
crankvm_specialObject_false(crankvm_context_t *context)
{
    return crankvm_context_getSpecialObjectsArray(context)->falseObject;
}

static inline crankvm_error_t
crankvm_specialObject_getCompiledCodeHeader(crankvm_compiled_code_header_t *result, crankvm_CompiledCode_t *compiledCode)
{
    return crankvm_object_decodeCompiledCodeHeader(result, compiledCode->codeHeader);
}

static inline crankvm_object_format_t
crankvm_Behavior_getInstanceSpec(crankvm_Behavior_t *behavior)
{
    return (crankvm_oop_decodeSmallInteger(behavior->format) >> 16) & 0x1F;
}

static inline crankvm_oop_t
crankvm_Behavior_getInstanceSize(crankvm_Behavior_t *behavior)
{
    return crankvm_oop_decodeSmallInteger(behavior->format) & 0xFFFF;
}

LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_MethodContext_validate(crankvm_context_t *context, crankvm_MethodContext_t *methodContext);

LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_CompiledCode_validate(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode);
LIB_CRANK_VM_EXPORT size_t crankvm_CompiledCode_getNumberOfLiterals(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_CompiledCode_getSelector(crankvm_context_t *context, crankvm_CompiledCode_t *compiledCode);

LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_MethodDictionary_atOrNil(crankvm_context_t *context, crankvm_MethodDictionary_t *methodDict, crankvm_oop_t key);

LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_Behavior_lookupSelector(crankvm_context_t *context, crankvm_Behavior_t *behavior, crankvm_oop_t selector);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_Behavior_getName(crankvm_context_t *context, crankvm_Behavior_t *behavior);

LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_Behavior_basicNew(crankvm_context_t *context, crankvm_Behavior_t *behavior);
LIB_CRANK_VM_EXPORT crankvm_oop_t crankvm_Behavior_basicNewWithVariable(crankvm_context_t *context, crankvm_Behavior_t *behavior, size_t variableSize);

LIB_CRANK_VM_EXPORT crankvm_MethodContext_t *crankvm_MethodContext_create(crankvm_context_t *context, int largeFrame);

#endif //CRANK_VM_SPECIAL_OBJECTS_H
