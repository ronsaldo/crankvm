#ifndef _CRANK_VM_COMMON_H_
#define _CRANK_VM_COMMON_H_

#ifndef CRANK_VM_STATIC
#   define CRANK_VM_EXPORT
#   define CRANK_VM_IMPORT
#else
#   ifdef _WIN32
#       define CRANK_VM_EXPORT declspec(dllexport)
#       define CRANK_VM_IMPORT declspec(dllimport)
#   else
#       define CRANK_VM_EXPORT __attribute__ ((visibility ("default")))
#       define CRANK_VM_IMPORT __attribute__ ((visibility ("default")))
#   endif
#endif

#ifdef __cplusplus
#   define CRANK_VM_EXTERN_C extern "C"
#   define CRANK_VM_INLINE inline
#else
#   define CRANK_VM_EXTERN_C
#   define CRANK_VM_INLINE static inline
#endif

#ifdef BUILD_LIB_CRANK_VM
#define LIB_CRANK_VM_EXPORT CRANK_VM_EXTERN_C CRANK_VM_EXPORT
#else
#define LIB_CRANK_VM_EXPORT CRANK_VM_EXTERN_C CRANK_VM_IMPORT
#endif

#endif //_CRANK_VM_COMMON_H_
