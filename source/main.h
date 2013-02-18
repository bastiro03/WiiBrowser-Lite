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

bool InitMPlayer();
void LoadMPlayerFile(char *loadedFile);
void ExitApp();

extern SSettings Settings;
extern int ExitRequested;
extern int GuiShutdown;
extern FreeTypeGX *fontSystem[];

extern "C" {
    extern int controlledbygui;
    extern void __exception_setreload(int t);

    int mplayer_main(); // in mplayer.c
    void wiiLoadFile(char *filename, char *partitionlabel);
    void wiiGotoGui();
    void wiiPause();
    bool wiiIsPaused();
    bool wiiIsPlaying();
    void wiiMute();
    void wiiSetSeekBackward(int sec);
    void wiiSetSeekForward(int sec);
    void wiiSeekPos(int sec);
    void wiiFastForward();
    void wiiRewind();
    double wiiGetTimeLength();
    double wiiGetTimePos();
    void wiiGetTimeDisplay(char *buf);
    void wiiSetDVDDevice(char *dev);
    bool wiiAudioOnly();
    char * wiiGetMetaTitle();
    char * wiiGetMetaArtist();
    char * wiiGetMetaAlbum();
    char * wiiGetMetaYear();
    void wiiDVDNav(int cmd);
    void wiiUpdatePointer(int x, int y);
    bool wiiPlayingDVD();
    bool wiiInDVDMenu();
    void wiiSetCacheFill(int fill);
    void wiiSetOnlineCacheFill(int fill);
    void wiiSetAutoResume(int enable);
    void wiiSetVolume(int vol);
    void wiiSetProperty(int cmd, float val);
    void wiiSetCodepage(char *cp);
    void wiiSetAudioLanguage(char *lang);
    void wiiSetSubtitleLanguage(char *lang);
    void wiiSetSubtitleColor(char *color);
    void wiiSetSubtitleSize(float size);
    bool wiiFindRestorePoint(char *filename, char *partitionlabel);
    void wiiLoadRestorePoints(char *buffer, int size);
    char * wiiSaveRestorePoints(char *path);
}

enum
{
    FRAMEDROPPING_DISABLED,
    FRAMEDROPPING_AUTO,
    FRAMEDROPPING_ALWAYS
};

#endif
