#ifndef __NETWORKOP_H__
#define __NETWORKOP_H__

#include <errno.h>
#include "transfer.h"

void InitNetwork();
void StopNetwork();
bool CheckConnection();

extern lwp_t networkthread;
extern u8 networkstack[GUITH_STACK];

extern bool networkinit;

#endif
