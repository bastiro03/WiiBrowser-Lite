#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool MountManager_Init(void);
bool MountManager_IsInserted(const char* dev);
void MountManager_Poll(void);
void MountManager_Deinit(void);
#ifdef __cplusplus
}
#endif
