/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * main.h
 ***************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "FreeTypeGX.h"
#include "Settings.h"

extern SSettings Settings;
bool InitMPlayer();
void LoadMPlayerFile();

void ExitApp();
extern int ExitRequested;
extern int GuiShutdown;
extern FreeTypeGX *fontSystem[];

extern "C" {
    extern int controlledbygui;
    extern void __exception_setreload(int t);

    int mplayer_main(); // in mplayer.c
    void wiiLoadFile(char *filename, char *partitionlabel);
    void wiiSetCacheFill(int fill);
    void wiiSetOnlineCacheFill(int fill);
    void wiiSetProperty(int cmd, float val);
}

#endif
