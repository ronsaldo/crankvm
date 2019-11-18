#include "object-primitives.h"

// Object accessing primitives.
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_at, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_AT)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_atPut, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_AT_PUT)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_size, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_SIZE)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_stringAt, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_STRING_AT)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_stringAtPut, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_STRING_AT_PUT)

// StorageManagement Primitives (68-79)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_objectAt, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_OBJECT_AT)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_objectAtPut, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_OBJECT_AT_PUT)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_new, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_OBJECT_NEW)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_newWithArg, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_OBJECT_NEW_WITH_ARG)

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_identityEquals, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_OBJECT_IDENTITY_EQUALS)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_identityNotEquals, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_OBJECT_IDENTITY_NOT_EQUALS)

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_identityHash, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_IDENTITY_HASH)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_behaviorHash, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_BEHAVIOR_HASH)

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_class, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_OBJECT_CLASS)

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_asCharacter, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_AS_CHARACTER)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_immediateAsInteger, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_IMMEDIATE_AS_INTEGER)

// Object cloning
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_shallowCopy, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_SHALLOW_COPY)

static crankvm_oop_t
crankvm_primitive_Object_at(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t object, intptr_t index)
{
    // Check the index
    intptr_t objectSize = crankvm_object_header_getSmalltalkSize((crankvm_object_header_t*)object);
    if(index < 1 || index > objectSize)
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);
        return crankvm_specialObject_nil(primitiveContext->context);
    }

    crankvm_object_format_t format = crankvm_oop_getFormat(object);
    if(format == CRANK_VM_OBJECT_FORMAT_VARIABLE_SIZE_IVARS)
    {
        // Add the fixed instance size.
        crankvm_Behavior_t *behavior = (crankvm_Behavior_t*)crankvm_object_getClass(crankvm_primitive_getContext(primitiveContext), object);
        index += crankvm_Behavior_getInstanceSize(behavior);

        // Check the bounds again
        if(index > objectSize)
        {
            crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);
            return crankvm_specialObject_nil(primitiveContext->context);
        }
    }

    --index;

    // Check the format
    crankvm_oop_t *slots = (crankvm_oop_t *) (object + sizeof(crankvm_object_header_t));
    if(format < CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
        return slots[index];

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
        uint32_t *data = (uint32_t*)slots;
        return crankvm_object_forUInteger64(primitiveContext->context, data[index]);
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1)
    {
        uint32_t *data = (uint32_t*)slots;
        return crankvm_object_forUInteger32(primitiveContext->context, data[index]);
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3)
    {
        uint16_t *data = (uint16_t*)slots;
        return crankvm_oop_encodeSmallInteger(data[index]);
    }
    else
    {
        uint8_t *data = (uint8_t*)slots;
        return crankvm_oop_encodeSmallInteger(data[index]);
    }
}

static crankvm_oop_t
crankvm_primitive_Object_atPut(crankvm_primitive_context_t *primitiveContext, crankvm_oop_t object, intptr_t index, crankvm_oop_t value)
{
    // Check the index
    intptr_t objectSize = crankvm_object_header_getSmalltalkSize((crankvm_object_header_t*)object);
    if(index < 1 || index > objectSize)
    {
        crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);
        return crankvm_specialObject_nil(primitiveContext->context);
    }

    crankvm_object_format_t format = crankvm_oop_getFormat(object);
    if(format == CRANK_VM_OBJECT_FORMAT_VARIABLE_SIZE_IVARS)
    {
        // Add the fixed instance size.
        crankvm_Behavior_t *behavior = (crankvm_Behavior_t*)crankvm_object_getClass(crankvm_primitive_getContext(primitiveContext), object);
        index += crankvm_Behavior_getInstanceSize(behavior);

        // Check the bounds again
        if(index > objectSize)
        {
            crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);
            return crankvm_specialObject_nil(primitiveContext->context);
        }
    }

    --index;

    // Check the format
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
        fprintf(stderr, "TODO: crankvm_primitive_Object_atPut: 64\n");
        abort();
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1)
    {
        fprintf(stderr, "TODO: crankvm_primitive_Object_atPut: 32\n");
        abort();
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3)
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
crankvm_primitive_at(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getStackAt(primitiveContext, 1);
    intptr_t index = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 0));
    printf("Primitive at: [%p]%ld\n", (void*)crankvm_primitive_getStackAt(primitiveContext, 0), index);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    if(!crankvm_oop_isPointer(receiver))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);
    if(index < 1)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);

    crankvm_oop_t result = crankvm_primitive_Object_at(primitiveContext, receiver, index);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    return crankvm_primitive_returnOop(primitiveContext, result);
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

    crankvm_oop_t result = crankvm_primitive_Object_atPut(primitiveContext, receiver, index, value);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    return crankvm_primitive_returnOop(primitiveContext, result);
}

void
crankvm_primitive_stringAt(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getStackAt(primitiveContext, 1);
    intptr_t index = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    if(!crankvm_oop_isPointer(receiver))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

    // Check the index
    intptr_t objectSize = crankvm_object_header_getSmalltalkSize((crankvm_object_header_t*)receiver);
    if(index < 1 || index > objectSize)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);
    --index;

    // Check the format
    crankvm_object_format_t format = crankvm_oop_getFormat(receiver);
    crankvm_oop_t *slots = (crankvm_oop_t *) (receiver + sizeof(crankvm_object_header_t));
    if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_64 || format >= CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

    uint32_t character = 0;
    if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1)
    {
        uint32_t *data = (uint32_t*)slots;
        character = data[index];
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3)
    {
        uint16_t *data = (uint16_t*)slots;
        character = data[index];
    }
    else
    {
        uint8_t *data = (uint8_t*)slots;
        character = data[index];
    }

    return crankvm_primitive_returnCharacter(primitiveContext, character);
}

void
crankvm_primitive_stringAtPut(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getStackAt(primitiveContext, 2);
    intptr_t index = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 1));
    crankvm_oop_t value = crankvm_primitive_getStackAt(primitiveContext, 0);
    uint32_t character = crankvm_primitive_getCharacterValue(primitiveContext, value);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    if(!crankvm_oop_isPointer(receiver))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

    // Check the index
    intptr_t objectSize = crankvm_object_header_getSmalltalkSize((crankvm_object_header_t*)receiver);
    if(index < 1 || index > objectSize)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);
    --index;

    // Check the format
    crankvm_object_format_t format = crankvm_oop_getFormat(receiver);
    crankvm_oop_t *slots = (crankvm_oop_t *) (receiver + sizeof(crankvm_object_header_t));
    if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_64 || format >= CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

    if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1)
    {
        uint32_t *data = (uint32_t*)slots;
        data[index] = character;
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3)
    {
        uint16_t *data = (uint16_t*)slots;
        data[index] = character;
    }
    else
    {
        uint8_t *data = (uint8_t*)slots;
        data[index] = character;
    }

    return crankvm_primitive_returnOop(primitiveContext, value);
}


void
crankvm_primitive_size(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getStackAt(primitiveContext, 0);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    if(!crankvm_oop_isPointer(receiver))
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

    return crankvm_primitive_returnInteger(primitiveContext, crankvm_object_header_getSmalltalkSize((crankvm_object_header_t*)receiver));
}

// StorageManagement Primitives (68-79)
void
crankvm_primitive_objectAt(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getStackAt(primitiveContext, 1);
    intptr_t index = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    // Check the format.
    crankvm_object_format_t format = crankvm_oop_getFormat(receiver);
    if(format < CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

    // Check the index.
    crankvm_CompiledCode_t *compiledCode = (crankvm_CompiledCode_t*)receiver;
    intptr_t objectSize = 1 + crankvm_CompiledCode_getNumberOfLiterals(primitiveContext->context, compiledCode);
    if(index < 1 || index > objectSize)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_INDEX);

    return crankvm_primitive_returnOop(primitiveContext, compiledCode->literals[index - /* One based, method header. */ 2]);
}

void
crankvm_primitive_objectAtPut(crankvm_primitive_context_t *primitiveContext)
{
    abort();
}

void
crankvm_primitive_shallowCopy(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getReceiver(primitiveContext);
    if(crankvm_primitive_hasFailed(primitiveContext))
        return crankvm_primitive_fail(primitiveContext);

    // Return non pointers by value.
    if(!crankvm_oop_isPointer(receiver))
        return crankvm_primitive_returnOop(primitiveContext, receiver);

    crankvm_object_header_t *clonedObject = crankvm_heap_shallowCopy(crankvm_primitive_getContext(primitiveContext), (crankvm_object_header_t*)receiver);
    return crankvm_primitive_returnOop(primitiveContext, (crankvm_oop_t)clonedObject);
}

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

void
crankvm_primitive_identityHash(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getReceiver(primitiveContext);
    if(!crankvm_oop_isPointer(receiver))
        return crankvm_primitive_fail(primitiveContext);

    uint32_t identityHash = crankvm_object_getIdentityHash(crankvm_primitive_getContext(primitiveContext), receiver);
    crankvm_primitive_returnOop(primitiveContext, crankvm_oop_encodeSmallInteger(identityHash));
}

void
crankvm_primitive_behaviorHash(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getReceiver(primitiveContext);
    if(!crankvm_oop_isPointer(receiver))
        return crankvm_primitive_fail(primitiveContext);

    uint32_t identityHash = crankvm_object_getBehaviorHash(crankvm_primitive_getContext(primitiveContext), receiver);
    crankvm_primitive_returnOop(primitiveContext, crankvm_oop_encodeSmallInteger(identityHash));
}

void
crankvm_primitive_class(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getReceiver(primitiveContext);
    crankvm_primitive_returnOop(primitiveContext, crankvm_object_getClass(primitiveContext->context, receiver));
}

void
crankvm_primitive_asCharacter(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getStackAt(primitiveContext, 0);
    if(crankvm_oop_isSmallInteger(receiver))
    {
        intptr_t value = crankvm_oop_decodeSmallInteger(receiver);
        if(crankvm_oop_isIntegerInCharacterRange(value))
        {
            return crankvm_primitive_returnOop(primitiveContext, crankvm_oop_encodeCharacter(value));
        }
    }
    else if(crankvm_oop_isCharacter(receiver))
    {
        return crankvm_primitive_returnOop(primitiveContext, receiver);
    }

    return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

}

void
crankvm_primitive_immediateAsInteger(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_oop_t receiver = crankvm_primitive_getStackAt(primitiveContext, 0);
    if(crankvm_oop_isSmallInteger(receiver))
    {
        return crankvm_primitive_returnOop(primitiveContext, receiver);
    }
    else if(crankvm_oop_isCharacter(receiver))
    {
        return crankvm_primitive_returnOop(primitiveContext, crankvm_oop_encodeSmallInteger(crankvm_oop_decodeCharacter(receiver)));
    }
    else if(crankvm_oop_isSmallFloat(receiver))
    {
        fprintf(stderr, "Unimplemented crankvm_primitive_immediateAsInteger for SmallFloats");
        abort();
    }

    return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_RECEIVER);

}
