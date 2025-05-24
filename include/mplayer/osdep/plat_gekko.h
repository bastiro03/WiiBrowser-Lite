#ifndef __PLAT_GEKKO_H__
#define __PLAT_GEKKO_H__

#ifdef GEKKO

#include <gctypes.h>

extern bool reset_pressed;
extern bool power_pressed;

void DVDGekkoTick(bool silent);
bool DVDGekkoMount();

void plat_init(int *argc, char **argv[]);
void plat_deinit(int rc);

#endif

#endif
