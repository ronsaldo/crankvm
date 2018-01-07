#ifndef CRANK_VM_SPECIAL_OBJECTS_H
#define CRANK_VM_SPECIAL_OBJECTS_H

#include <crank-vm/objectmodel.h>

/**
 * ProtoObject layout
 */
typedef struct crankvm_ProtoObject_s
{
    crankvm_object_header_t objectHeader;
    /* Here goes the slots*/
} crankvm_ProtoObject_t;

typedef crankvm_ProtoObject_t crankvm_Object_t;

/**
 * Behavior layout
 */
typedef struct crankvm_Behavior_s
{
    crankvm_Object_t baseClass;

    crankvm_oop_t superclass;
    crankvm_oop_t format;
    crankvm_oop_t layout;
} crankvm_Behavior_t;

/**
 * Array layout
 */
typedef struct crankvm_Array_s
{
    crankvm_Object_t baseClass;

    crankvm_oop_t slots[];
} crankvm_Array_t;

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
    crankvm_Object_t baseClass;

    crankvm_oop_t stackp;
    crankvm_oop_t method;
    crankvm_oop_t closureOrNil;
    crankvm_oop_t receiver;
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

typedef struct crankvm_special_object_array_s
{
    crankvm_object_header_t objectHeader;

    crankvm_oop_t nilObject; // 0
	crankvm_oop_t falseObject; // 1
	crankvm_oop_t trueObject; // 2

	crankvm_Association_t *schedulerAssociation; // 3

    crankvm_oop_t ClassBitmap; // 4
	crankvm_oop_t ClassSmallInteger; // 5
	crankvm_oop_t ClassByteString; // ClassString "N.B.  Actually class ByteString"
	crankvm_oop_t ClassArray; // 7.
	crankvm_oop_t old_SmalltalkDictionary; // 8."  "Do not delete!"
	crankvm_oop_t ClassFloat;// 9.
	crankvm_oop_t ClassMethodContext;// 10.
	crankvm_oop_t unused_ClassBlockContext;// 11. unused by the VM"
	crankvm_oop_t ClassPoint; //12.
	crankvm_oop_t ClassLargePositiveInteger; //13.

    crankvm_oop_t TheDisplay; //14.

    crankvm_oop_t ClassMessage; //15.
	crankvm_oop_t unused_ClassCompiledMethod; //16. unused by the VM"

    crankvm_oop_t TheLowSpaceSemaphore; //17.

    crankvm_oop_t ClassSemaphore; //18.
	crankvm_oop_t ClassCharacter; //19.

    crankvm_oop_t SelectorDoesNotUnderstand; //20.
	crankvm_oop_t SelectorCannotReturn; //21.
	crankvm_oop_t ProcessSignalingLowSpace; //22.	"was TheInputSemaphore"
	crankvm_oop_t SpecialSelectors; //23.

    crankvm_oop_t CharacterTable; //nil.	"Must be unused by the VM"
	crankvm_oop_t SelectorMustBeBoolean; //25.
	crankvm_oop_t ClassByteArray; //26.
	crankvm_oop_t unused_ClassProcess;//; //27. unused"
	crankvm_oop_t CompactClasses; //28.
	crankvm_oop_t TheTimerSemaphore; //29.
	crankvm_oop_t TheInterruptSemaphore; //30.
	crankvm_oop_t SelectorCannotInterpret; //34.
	crankvm_oop_t old_MethodContextProto; //35.
	crankvm_oop_t ClassBlockClosure; //36.
	crankvm_oop_t old_BlockContextProto; //37.
	crankvm_oop_t ExternalObjectsArray; //38.
	crankvm_oop_t ClassMutex; //39.
	crankvm_oop_t ProcessInExternalCodeTag; //40.
	crankvm_oop_t TheFinalizationSemaphore; //41.
	crankvm_oop_t ClassLargeNegativeInteger; //42.

	crankvm_oop_t ClassExternalAddress; //43.
	crankvm_oop_t ClassExternalStructure; //44.
	crankvm_oop_t ClassExternalData; //45.
	crankvm_oop_t ClassExternalFunction; //46.
	crankvm_oop_t ClassExternalLibrary; //47.

	crankvm_oop_t SelectorAboutToReturn; //48.
	crankvm_oop_t SelectorRunWithIn; //49.

	crankvm_oop_t SelectorAttemptToAssign; //50.
	crankvm_oop_t old_PrimErrTableIndex; //51. in VMClass class>>initializePrimitiveErrorCodes"
	crankvm_oop_t ClassAlien; //52.
	crankvm_oop_t SelectorInvokeCallback; //53.
	crankvm_oop_t ClassUnsafeAlien; //54.

	crankvm_oop_t ClassWeakFinalizer; //55.

	crankvm_oop_t ForeignCallbackProcess; //56.

	crankvm_oop_t SelectorUnknownBytecode; //57.
	crankvm_oop_t SelectorCounterTripped; //58.
	crankvm_oop_t SelectorSistaTrap; //59.

	crankvm_oop_t LowcodeContextMark; //60.
	crankvm_oop_t LowcodeNativeContextClass; //61.
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

#endif //CRANK_VM_SPECIAL_OBJECTS_H
