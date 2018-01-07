#ifndef _CRANK_VM_ARCH_H_
#define _CRANK_VM_ARCH_H_

#if defined(__x86_64__)
#   define CRANK_VM_64_BITS 1
#   define CRANK_VM_WORD_SIZE 8
#   define CRANK_VM_LITTLE_ENDIAN 1
#   define CRANK_VM_BIG_ENDIAN 0
#elif defined(__i386__)
#   define CRANK_VM_32_BITS 1
#   define CRANK_VM_WORD_SIZE 4
#   define CRANK_VM_LITTLE_ENDIAN 1
#   define CRANK_VM_BIG_ENDIAN 0
#else
#error Unsupported platform
#endif

#endif //_CRANK_VM_H_
