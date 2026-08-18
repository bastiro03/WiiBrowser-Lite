#ifndef ZIP7_INC_7Z_TYPES_H
#define ZIP7_INC_7Z_TYPES_H

/* 7zTypes.h was renamed to Types.h in newer 7-Zip SDK releases.
   This forwarder pulls in the base types and adds the C-API glue
   macros/typedefs that the older base header does not define. */

#include "Types.h"

#ifndef EXTERN_C_BEGIN
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#define EXTERN_C extern "C"
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define EXTERN_C
#endif
#endif

#ifndef ISzAllocPtr
typedef ISzAlloc *ISzAllocPtr;
#endif

#endif