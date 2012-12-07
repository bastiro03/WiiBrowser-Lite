/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * main.h
 ***************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "FreeTypeGX.h"

enum {
	METHOD_AUTO,
	METHOD_SD,
	METHOD_USB,
	METHOD_DVD,
	METHOD_SMB,
	METHOD_MC_SLOTA,
	METHOD_MC_SLOTB,
	METHOD_SD_SLOTA,
	METHOD_SD_SLOTB
};

void ExitApp();
extern int ExitRequested;
extern FreeTypeGX *fontSystem;

extern "C" {
    extern void __exception_setreload(int t);
}

#endif
