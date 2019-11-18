#include "system-primitives.h"
#include <sys/time.h>
#include <time.h>

#define MicrosecondsFrom1901To1970 2177452800000000ULL
#define MicrosecondsPerSecond 1000000ULL

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_primitiveFail, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_PRIMITIVE_FAIL) // This must fail

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_systemPrimitive_snapshot, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_SNAPSHOT)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_systemPrimitive_quit, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_QUIT)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_systemPrimitive_exitToDebugger, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_EXIT_TO_DEBUGGER)

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_systemPrimitive_vmParameter, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_VM_PARAMETER);

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_systemPrimitive_utcMicrosecondClock, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_VM_PARAMETER_UTC_MICROSECOND_CLOCK)
CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_systemPrimitive_localMicrosecondClock, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_VM_PARAMETER_LOCAL_MICROSECOND_CLOCK)

void
crankvm_primitive_primitiveFail(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_fail(primitiveContext);
}

void
crankvm_primitive_primitiveFailUnexistent(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_fail(primitiveContext);
}

void
crankvm_primitive_systemPrimitive_snapshot(crankvm_primitive_context_t *primitiveContext)
{
    fprintf(stderr, "Unimplemented crankvm_primitive_systemPrimitive_snapshot\n");
    abort();
    //return crankvm_primitive_success(primitiveContext);
}

void
crankvm_primitive_systemPrimitive_quit(crankvm_primitive_context_t *primitiveContext)
{
    fprintf(stderr, "Unimplemented crankvm_primitive_systemPrimitive_quit\n");
    abort();
}

void
crankvm_primitive_systemPrimitive_exitToDebugger(crankvm_primitive_context_t *primitiveContext)
{
    fprintf(stderr, "Unimplemented crankvm_primitive_systemPrimitive_exitToDebugger\n");
    abort();
}

/*
Behaviour depends on argument count:
    0 args:	return an Array of VM parameter values;
    1 arg:	return the indicated VM parameter;
    2 args:	set the VM indicated parameter.
VM parameters are numbered as follows:
    1	end of old-space (0-based, read-only)
    2	end of young-space (read-only)
    3	end of memory (read-only)
    4	allocationCount (read-only)
    5	allocations between GCs (read-write)
    6	survivor count tenuring threshold (read-write)
    7	full GCs since startup (read-only)
    8	total milliseconds in full GCs since startup (read-only)
    9	incremental GCs since startup (read-only)
    10	total milliseconds in incremental GCs since startup (read-only)
    11	tenures of surving objects since startup (read-only)
    12-20 specific to the translating VM
    21	root table size (read-only)
    22	root table overflows since startup (read-only)
    23	bytes of extra memory to reserve for VM buffers, plugins, etc.
    24	memory threshold above which shrinking object memory (rw)
    25	memory headroom when growing object memory (rw)
    26  interruptChecksEveryNms - force an ioProcessEvents every N milliseconds, in case the image  is not calling getNextEvent often (rw)
    27	number of times mark loop iterated for current IGC/FGC (read-only) includes ALL marking
    28	number of times sweep loop iterated  for current IGC/FGC (read-only)
    29	number of times make forward loop iterated for current IGC/FGC (read-only)
    30	number of times compact move loop iterated for current IGC/FGC (read-only)
    31	number of grow memory requests (read-only)
    32	number of shrink memory requests (read-only)
    33	number of root table entries used for current IGC/FGC (read-only)
    34	number of allocations done before current IGC/FGC (read-only)
    35	number of survivor objects after current IGC/FGC (read-only)
    36	millisecond clock when current IGC/FGC completed (read-only)
    37	number of marked objects for Roots of the world, not including Root Table entries for current IGC/FGC (read-only)
    38  milliseconds taken by current IGC  (read-only)
    39  Number of finalization signals for Weak Objects pending when current IGC/FGC completed (read-only)
    40	BytesPerWord for this image
    41  imageFormatVersion for the VM
    42	nil (number of stack pages in use in Stack VM)
    43	nil (desired number of stack pages in Stack VM)
    44	nil (size of eden, in bytes in Stack VM)
    45	nil (desired size of eden in Stack VM)
    46-55 nil; reserved for VM parameters that persist in the image (such as eden above)
    56	number of process switches since startup (read-only)
    57	number of ioProcessEvents calls since startup (read-only)
    58	number of ForceInterruptCheck calls since startup (read-only)
    59	number of check event calls since startup (read-only)
*/

static crankvm_oop_t
crankvm_primitive_systemPrimitive_getVMParameter(crankvm_primitive_context_t *primitiveContext, intptr_t parameterIndex)
{
    switch(parameterIndex)
    {
    case 40: return crankvm_oop_encodeSmallInteger(CRANK_VM_WORD_SIZE);
    case 44: return crankvm_oop_encodeSmallInteger(0); // Size of eden, in bytes.
    default:
        printf("Unsupported vm parameter %d requested\n", (int)parameterIndex);
        return crankvm_specialObject_nil(primitiveContext->context);
    }
}

static void
crankvm_primitive_systemPrimitive_setVMParameter(crankvm_primitive_context_t *primitiveContext, intptr_t parameterIndex, crankvm_oop_t newValue)
{
    switch(parameterIndex)
    {
    default:
        printf("Unsupported setting vm parameter %d.\n", (int)parameterIndex);
        return crankvm_primitive_returnOop(primitiveContext, newValue);
    }
}

void
crankvm_primitive_systemPrimitive_vmParameter(crankvm_primitive_context_t *primitiveContext)
{
    int argumentCount = crankvm_primitive_getArgumentCount(primitiveContext);
    if(argumentCount == 0)
    {
        size_t paramsArraySize = 59;
        crankvm_Array_t *array = crankvm_Array_create(primitiveContext->context, paramsArraySize);
        for(size_t i = 0; i < paramsArraySize; ++i)
            array->slots[i] = crankvm_primitive_systemPrimitive_getVMParameter(primitiveContext, i);
    }
    else if(argumentCount == 1)
    {
        intptr_t parameterIndex = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 0));
        if(crankvm_primitive_hasFailed(primitiveContext))
            return crankvm_primitive_fail(primitiveContext);

        return crankvm_primitive_returnOop(primitiveContext, crankvm_primitive_systemPrimitive_getVMParameter(primitiveContext, parameterIndex));
    }
    else if(argumentCount == 2)
    {
        intptr_t parameterValue = crankvm_primitive_getStackAt(primitiveContext, 0);
        intptr_t parameterIndex = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 1));
        if(crankvm_primitive_hasFailed(primitiveContext))
            return crankvm_primitive_fail(primitiveContext);

        return crankvm_primitive_systemPrimitive_setVMParameter(primitiveContext, parameterIndex, parameterValue);
    }
}

static uint64_t
getCurrentMicrosecondsInUTC()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec*MicrosecondsPerSecond + tv.tv_usec + MicrosecondsFrom1901To1970;
}

static uint64_t
getLocalTimezoneOffset()
{
    return localtime(NULL)->tm_gmtoff*MicrosecondsPerSecond;
}

static uint64_t
getCurrentMicrosecondsInLocalTime()
{
    return getCurrentMicrosecondsInUTC() + getLocalTimezoneOffset();
}

void
crankvm_primitive_systemPrimitive_utcMicrosecondClock(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_returnUInteger64(primitiveContext, getCurrentMicrosecondsInUTC());
}

void
crankvm_primitive_systemPrimitive_localMicrosecondClock(crankvm_primitive_context_t *primitiveContext)
{
    return crankvm_primitive_returnUInteger64(primitiveContext, getCurrentMicrosecondsInLocalTime());
}
