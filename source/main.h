/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * demo.h
 ***************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "FreeTypeGX.h"
#include "Settings.h"

extern SSettings Settings;

void ExitApp();
extern int ExitRequested;
extern FreeTypeGX *fontSystem[];

extern "C" {
    extern void __exception_setreload(int t);
}

#endif
