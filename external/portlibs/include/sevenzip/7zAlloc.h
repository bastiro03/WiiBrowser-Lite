/* 7zAlloc.h -- Allocation functions : Igor Pavlov : Public domain */

#ifndef __7Z_ALLOC_H
#define __7Z_ALLOC_H

#include "7zTypes.h"

EXTERN_C_BEGIN

void *SzAlloc(void *p, size_t size);
void SzFree(void *p, void *address);

void *SzAllocTemp(void *p, size_t size);
void SzFreeTemp(void *p, void *address);

EXTERN_C_END

#endif