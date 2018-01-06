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

#endif //CRANK_VM_SPECIAL_OBJECTS_H
