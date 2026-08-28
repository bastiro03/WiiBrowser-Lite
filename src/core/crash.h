#pragma once
#include <gctypes.h>
#ifdef __cplusplus
extern "C" {
#endif
void Crash_Init(void);
void Crash_Dump(const char* reason, u32 srr0, u32 srr1, u32 lr);
#ifdef __cplusplus
}
#endif
