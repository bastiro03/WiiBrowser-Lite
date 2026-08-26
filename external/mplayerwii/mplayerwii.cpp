/****************************************************************************
 * WiiBrowser Lite - libmplayerwii compatibility shim
 *
 * The browser source links against the Wii port of MPlayer
 * (libmplayerwii). Building the full MPlayer/FFmpeg stack is a heavy,
 * separate step that is not part of the main Makefile.  Until that is
 * wired up, this shim provides a minimal, no-op implementation of the
 * MPlayer interface used by the browser so the application still
 * compiles and links.  Video playback is therefore disabled.
 *
 * To use the real MPlayer build, compile external/mplayer into
 * libmplayerwii.a, stop adding this directory to SOURCES, and add
 * -lmplayerwii to LIBS.
 ***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "mplayerwii.h"

int controlledbygui = 0;

int mplayer_main()
{
	return 0;
}

void wiiLoadFile(char *filename, char *partitionlabel)
{
}

void wiiGotoGui()
{
}

void wiiPause()
{
}

bool wiiIsPaused()
{
	return false;
}

bool wiiIsPlaying()
{
	return false;
}

void wiiMute()
{
}

void wiiSetSeekBackward(int sec)
{
}

void wiiSetSeekForward(int sec)
{
}

void wiiSeekPos(int sec)
{
}

void wiiFastForward()
{
}

void wiiRewind()
{
}

double wiiGetTimeLength()
{
	return 0.0;
}

double wiiGetTimePos()
{
	return 0.0;
}

void wiiGetTimeDisplay(char *buf)
{
	if (buf)
		sprintf(buf, "00:00");
}

void wiiSetDVDDevice(char *dev)
{
}

bool wiiAudioOnly()
{
	return false;
}

char * wiiGetMetaTitle()
{
	return (char *)"";
}

char * wiiGetMetaArtist()
{
	return (char *)"";
}

char * wiiGetMetaAlbum()
{
	return (char *)"";
}

char * wiiGetMetaYear()
{
	return (char *)"";
}

void wiiDVDNav(int cmd)
{
}

void wiiUpdatePointer(int x, int y)
{
}

bool wiiPlayingDVD()
{
	return false;
}

bool wiiInDVDMenu()
{
	return false;
}

void wiiSetCacheFill(int fill)
{
}

void wiiSetOnlineCacheFill(int fill)
{
}

void wiiSetAutoResume(int enable)
{
}

void wiiSetVolume(int vol)
{
}

void wiiSetProperty(int cmd, float val)
{
}

void wiiSetCodepage(char *cp)
{
}

void wiiSetAudioLanguage(char *lang)
{
}

void wiiSetSubtitleLanguage(char *lang)
{
}

void wiiSetSubtitleColor(char *color)
{
}

void wiiSetSubtitleSize(float size)
{
}

bool wiiFindRestorePoint(char *filename, char *partitionlabel)
{
	return false;
}

void wiiLoadRestorePoints(char *buffer, int size)
{
}

char * wiiSaveRestorePoints(char *path)
{
	return NULL;
}