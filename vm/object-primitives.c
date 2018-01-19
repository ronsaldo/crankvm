#include "object-primitives.h"

/*
(60 primitiveAt)
(61 primitiveAtPut)
(62 primitiveSize)
(63 primitiveStringAt)
(64 primitiveStringAtPut)
*/

/*
"StorageManagement Primitives (68-79)"
(68 primitiveObjectAt)
(69 primitiveObjectAtPut)
(72 primitiveArrayBecomeOneWay)	"Blue Book: primitiveBecome"
(73 primitiveInstVarAt)
(74 primitiveInstVarAtPut)
(75 primitiveIdentityHash)
(76 primitiveStoreStackp)			"Blue Book: primitiveAsObject"
(77 primitiveSomeInstance)
(78 primitiveNextInstance)
(79 primitiveNewMethod)

*/

// Object accessing primitives.
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_atPut, 61)

// StorageManagement Primitives (68-79)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_new, 70)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_newWithArg, 71)

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_identityEquals, 110)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_identityNotEquals, 169)

crankvm_oop_t
crankvm_primitive_atObjectPut(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t object, intptr_t index, crankvm_oop_t value)
{
    // Check the index
    intptr_t objectSize = crankvm_object_header_getSmalltalkSize((crankvm_object_header_t*)object);
    if(index < 1 || index > objectSize)
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);
        return crankvm_specialObject_nil(primitiveContext->context);
    }
    --index;

    // Check the format
    crankvm_object_format_t format = crankvm_oop_getFormat(object);
    crankvm_oop_t *slots = (crankvm_oop_t *) (object + sizeof(crankvm_object_header_t));
    if(format < CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
        return slots[index] = value;

    // For compiled code, the index must be after the first literal.
    if(format > CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD)
    {
        if(index < crankvm_CompiledCode_getFirstPC(primitiveContext->context, (crankvm_CompiledCode_t*)object))
        {
            crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);
            return crankvm_specialObject_nil(primitiveContext->context);
        }
    }

    if(format == CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
    {
        fprintf(stderr, "TODO: crankvm_primitive_atObjectPut: 64\n");
        abort();
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1)
    {
        fprintf(stderr, "TODO: crankvm_primitive_atObjectPut: 64\n");
        abort();
    }
    else if(format == CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3)
    {
        intptr_t integerValue = crankvm_primitive_getSmallIntegerValue(primitiveContext, value);
        if(crankvm_primitive_hasFailed(primitiveContext) || integerValue < 0 || integerValue > 0xFFFF )
        {
            crankvm_primitive_fail(primitiveContext);
            return crankvm_specialObject_nil(primitiveContext->context);
        }

        uint16_t *data = (uint16_t*)slots;
        data[index] = integerValue;
    }
    else
    {
        // Check the range
        intptr_t integerValue = crankvm_primitive_getSmallIntegerValue(primitiveContext, value);
        if(crankvm_primitive_hasFailed(primitiveContext) || integerValue < 0 || integerValue > 255 )
        {
            crankvm_primitive_fail(primitiveContext);
            return crankvm_specialObject_nil(primitiveContext->context);
        }

        uint8_t *data = (uint8_t*)slots;
        data[index] = integerValue;
    }

    return value;
}

void
crankvm_primitive_atPut(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getStackAt(primitiveContext, 2);
    intptr_t index = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 1));
    crankvm_oop_t value = crankvm_primitive_getStackAt(primitiveContext, 0);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    if(!crankvm_oop_isPointer(receiver))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);
    if(index < 1)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);

    crankvm_oop_t result = crankvm_primitive_atObjectPut(primitiveContext, receiver, index, value);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    return crankvm_primitive_returnOop(primitiveContext, result);

}

// StorageManagement Primitives (68-79)
void
crankvm_primitive_new(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getReceiver(primitiveContext);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    // The receiver must be a class.
    if(!crankvm_object_isClass(primitiveContext->context, receiver))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

    // Instantiate the class.
    crankvm_oop_t result = crankvm_Behavior_basicNew(primitiveContext->context, (crankvm_Behavior_t*)receiver);

    // Check the result
    if(crankvm_oop_isNil(primitiveContext->context, result))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_INSUFFICIENT_OBJECT_MEMORY);

    return crankvm_primitive_returnOop(primitiveContext, result);
}

void
crankvm_primitive_newWithArg(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getReceiver(primitiveContext);
    intptr_t variableSize = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getArgument(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    // The receiver must be a class.
    if(!crankvm_object_isClass(primitiveContext->context, receiver))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

    // Instantiate the class.
    crankvm_oop_t result = crankvm_Behavior_basicNewWithVariable(primitiveContext->context, (crankvm_Behavior_t*)receiver, variableSize);

    // Check the result
    if(crankvm_oop_isNil(primitiveContext->context, result))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_INSUFFICIENT_OBJECT_MEMORY);

    return crankvm_primitive_returnOop(primitiveContext, result);
}

void
crankvm_primitive_identityEquals(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t left = crankvm_primitive_getReceiver(primitiveContext);
    crankvm_oop_t right = crankvm_primitive_getArgument(primitiveContext, 0);
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left == right);
}

void
crankvm_primitive_identityNotEquals(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t left = crankvm_primitive_getReceiver(primitiveContext);
    crankvm_oop_t right = crankvm_primitive_getArgument(primitiveContext, 0);
    if(!crankvm_primitive_hasFailed(primitiveContext))
        crankvm_primitive_returnBoolean(primitiveContext, left != right);
}
