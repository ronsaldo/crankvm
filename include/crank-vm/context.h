#ifndef _CRANK_VM_CONTEXT_H_
#define _CRANK_VM_CONTEXT_H_

#include <crank-vm/common.h>
#include <crank-vm/oop.h>
#include <crank-vm/error.h>
#include <stddef.h>

#if CRANK_VM_WORD_SIZE == 4
#   define CRANK_VM_CONTEXT_DEFAULT_MAX_HEAP_CAPACITY (1ull*(1024*1024)) /* 1 GB */
#else
#   define CRANK_VM_CONTEXT_DEFAULT_MAX_HEAP_CAPACITY   (8ull*(1024*1024*1024)) /* 8 GB */
#endif

typedef struct crankvm_context_s crankvm_context_t;

/**
 * Create a new isolated crank vm context.
 */
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_context_create(crankvm_context_t **returnContext);

/**
 * Destroy a crank vm context.
 */
LIB_CRANK_VM_EXPORT void crankvm_context_destroy(crankvm_context_t *context);

/**
 * Sets the maximum capacity of the heap used by the context.
 * The maximum heap capacity is used to reserve a very large virtual address space.
 */
LIB_CRANK_VM_EXPORT void crankvm_context_setMaxHeapCapacity(crankvm_context_t *context, size_t capacity);

/**
 * Gets the maximum capacity of the heap used by the context.
 */
LIB_CRANK_VM_EXPORT size_t crankvm_context_getMaxHeapCapacity(crankvm_context_t *context);

/**
 * Loads a smalltalk image into the context from memory.
 */
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_context_loadImageFromMemory(crankvm_context_t *context, size_t imageSize, const void *imageData);

/**
 * Loads a smalltalk image into the context from a file name.
 */
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_context_loadImageFromFileNamed(crankvm_context_t *context, const char *fileName);

/**
 * Loads a smalltalk image into the context from memory.
 */
LIB_CRANK_VM_EXPORT crankvm_error_t crankvm_context_run(crankvm_context_t *context);

/**
 * Tells if something is nil
 */
LIB_CRANK_VM_EXPORT int crankvm_object_isNil(crankvm_context_t *context, void *pointer);

static inline int
crankvm_object_isNilOrNull(crankvm_context_t *context, void *pointer)
{
    return !pointer || crankvm_object_isNil(context, pointer);
}

static inline int
crankvm_oop_isNil(crankvm_context_t *context, crankvm_oop_t oop)
{
    return crankvm_object_isNil(context, (void*)oop);
}

static inline int
crankvm_oop_isNilOrNull(crankvm_context_t *context, crankvm_oop_t oop)
{
    return crankvm_object_isNilOrNull(context, (void*)oop);
}

#endif //_CRANK_VM_CONTEXT_H_
