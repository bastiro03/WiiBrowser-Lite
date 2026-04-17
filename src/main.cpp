/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * modified by gave92
 *
 * WiiBrowser
 * main.cpp
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <dirent.h>

#include "video.h"
#include "audio.h"
#include "menu.h"
#include "gettext.h"
#include "liste.h"
#include "input.h"
#include "filelist.h"
#include "main.h"

#include "FreeTypeGX.h"
#include "config.h"
#include "fileop.h"

#ifdef MPLAYER

#include "mplayer/input/input.h"
#include "mplayer/osdep/gx_supp.h"

extern char MPLAYER_DATADIR[512];
extern char MPLAYER_CONFDIR[512];
extern char MPLAYER_LIBDIR[512];
extern char MPLAYER_CSSDIR[512];

// MPlayer threads
#define MPLAYER_STACKSIZE (512 * 1024)
#define CACHE_STACKSIZE (16 * 1024)
static lwp_t mthread = LWP_THREAD_NULL;
static lwp_t cthread = LWP_THREAD_NULL;
static u8 cachestack[CACHE_STACKSIZE] ATTRIBUTE_ALIGN(32);

static void *
mplayerthread(void *arg)
{
	mplayer_main();
	return NULL;
}

extern "C"
{
	extern void *mplayercachethread(void *arg);

	void SuspendCacheThread()
	{
		LWP_SuspendThread(cthread);
	}
	void ResumeCacheThread()
	{
		LWP_ResumeThread(cthread);
	}
	bool CacheThreadSuspended()
	{
		if (LWP_ThreadIsSuspended(cthread))
			return true;
		return false;
	}
}

bool InitMPlayer()
{
	u8 *mplayerstack;
	char *AppPath = Settings.AppPath;

	static bool init = false;
	if (init)
		return true;

	if (AppPath[0] == 0)
		return false;

	// check if subtitle font file exists
	struct stat st;
	char filepath[1024];
	sprintf(filepath, "%s/subfont.ttf", AppPath);

	bool subtitleFontFound;
	if (stat(filepath, &st) == 0)
		subtitleFontFound = true;

	sprintf(MPLAYER_DATADIR, "%s", AppPath);
	sprintf(MPLAYER_CONFDIR, "%s", AppPath);
	sprintf(MPLAYER_LIBDIR, "%s", AppPath);
	sprintf(MPLAYER_CSSDIR, "%s/css", AppPath);
	DIR *dir = opendir(MPLAYER_CSSDIR);

	if (!dir && mkdir(MPLAYER_CSSDIR, 0777) != 0)
		sprintf(MPLAYER_CSSDIR, "off");
	else
		closedir(dir);

	setenv("HOME", MPLAYER_DATADIR, 1);
	setenv("DVDCSS_CACHE", MPLAYER_CSSDIR, 1);
	setenv("DVDCSS_METHOD", "key", 1);
	setenv("DVDCSS_VERBOSE", "0", 1);
	setenv("DVDREAD_VERBOSE", "0", 1);
	setenv("DVDCSS_RAW_DEVICE", "/dev/di", 1);

	// create mplayer thread
	mplayerstack = (u8 *)(memalign(32, MPLAYER_STACKSIZE * sizeof(u8)));
	memset(mplayerstack, 0, MPLAYER_STACKSIZE * sizeof(u8));
	LWP_CreateThread(&mthread, mplayerthread, NULL, mplayerstack, MPLAYER_STACKSIZE, 68);

	init = true;
	return true;
}

void LoadMPlayerFile(char *loadedFile)
{
	controlledbygui = 2; // signal any previous file to end
	char *partitionlabel = NULL;

	// Wait for the previous playback to acknowledge the shutdown
	// signal. Bounded at ~3 seconds (30000 x 100us); if mplayer is
	// deadlocked in demux/decode we'd rather proceed than hang
	// forever. The new-file load below will overwrite the old
	// state either way.
	int waits = 30000;
	while (controlledbygui == 2 && waits > 0)
	{
		usleep(100);
		waits--;
	}
	if (waits == 0)
		fprintf(stderr, "LoadMPlayerFile: previous playback did not end "
		                "within 3s; proceeding anyway\n");

	// set new file to load
	ShowAction("Loading...");
	wiiLoadFile(loadedFile, partitionlabel);

	// Wait for the new file to finish loading. Same timeout rationale
	// — we cap at ~3s rather than spinning forever if wiiLoadFile
	// never clears controlledbygui (e.g. bad file, codec deadlock).
	waits = 30000;
	while (controlledbygui != 0 && waits > 0)
	{
		usleep(100);
		waits--;
	}
	if (waits == 0)
		fprintf(stderr, "LoadMPlayerFile: new file did not finish loading "
		                "within 3s; bailing to menu\n");
}

void ResumeMPlayerFile()
{
	controlledbygui = 0;
}

void StopMPlayerFile()
{
	controlledbygui = 2; // signal any previous file to end
}

extern "C"
{
	void SetMPlayerSettings()
	{
		GX_SetScreenPos(0, 0, 0.8, 0.8);

		wiiSetAutoResume(1);
		wiiSetVolume(50);
		wiiSetSeekBackward(10);
		wiiSetSeekForward(30);

		wiiSetCacheFill(30);
		wiiSetOnlineCacheFill(20);
		wiiSetProperty(MP_CMD_FRAMEDROPPING, FRAMEDROPPING_AUTO);

		wiiSetProperty(MP_CMD_AUDIO_DELAY, 0);
		wiiSetProperty(MP_CMD_SUB_VISIBILITY, 0);
	}
}

#endif

SSettings Settings;
int ExitRequested = 0;
int ExitAccepted = 0;

void ExitApp()
{
#ifndef WIIFLOW
	Settings.Save(1);
	DumpList(history);
	FreeHistory(history);
#endif
	ShutoffRumble();
	StopGX();
	ShutdownAudio();
	ClearFontData();
	StopGUIThreads();
	Cleanup();
	if (HWButton)
		SYS_ResetSystem(HWButton, 0, 0);
	exit(0);
}

u8 HWButton;

void WiiResetPressed(u32 irq, void *ctx)
{
	(void)irq; (void)ctx;
	HWButton = SYS_RETURNTOMENU;
}

void WiiPowerPressed()
{
	HWButton = SYS_POWEROFF_STANDBY;
}

void WiimotePowerPressed(s32 chan)
{
	HWButton = SYS_POWEROFF_STANDBY;
}

void WaitExit()
{
	while (true)
	{
	}
}

// Embedded build fingerprint so `strings wiibrowserlite.dol | grep WBL_BUILD_ID`
// reveals which commit + timestamp produced this binary. Declared `volatile`
// + used below so LTO/--gc-sections don't strip it.
#ifndef WBL_BUILD_ID_STRING
#define WBL_BUILD_ID_STRING "WBL_BUILD_ID:unknown"
#endif
extern "C" {
	volatile const char wbl_build_id[] = WBL_BUILD_ID_STRING;
}

int main(int argc, char *argv[])
{
	// Redirect stderr/stdout to Dolphin's OSReport UART so fprintf(stderr,...)
	// output shows up in Dolphin's console (and USB Gecko on real hardware).
	// Without this, libogc stdio goes nowhere visible during development.
	SYS_STDIO_Report(true);

	// Force a reference so the linker retains wbl_build_id.
	fprintf(stderr, "%s\n", (const char *)wbl_build_id);

	SYS_SetResetCallback(WiiResetPressed);
	SYS_SetPowerCallback(WiiPowerPressed);
	WPAD_SetPowerButtonCallback(WiimotePowerPressed);

	InitVideo();	  // Initialize video
	SetupPads();	  // Initialize input
	InitAudio();	  // Initialize audio
	fatInitDefault(); // Initialize file system
	InitGUIThreads(); // Initialize GUI

#ifdef MPLAYER
	u32 size = ((1024 * MAX_HEIGHT) + ((MAX_WIDTH - 1024) * MAX_HEIGHT) + (1024 * (MAX_HEIGHT / 2) * 2)) + // textures
			   (vmode->fbWidth * vmode->efbHeight * 4) +												   // videoScreenshot
			   (32 * 1024);																				   // padding
#endif

	InitFreeType((uint8_t *)font_ttf, font_ttf_size); // Initialize font system

#ifdef MPLAYER
	// mplayer cache thread
	GX_AllocTextureMemory();

	memset(cachestack, 0, CACHE_STACKSIZE * sizeof(u8));
	LWP_CreateThread(&cthread, mplayercachethread, NULL, cachestack, CACHE_STACKSIZE, 70);
	usleep(200);
#endif

	Settings.Load();
	if (argc > 1)
		Settings.SetStartPage(argv[1]);

	if (chdir(Settings.AppPath) != 0) // set working directory
		save_mem("CHDIR failed");

	LoadLanguage();
	__exception_setreload(10);

	ResetVideo_Menu();
	MainMenu(MENU_SPLASH);
	WaitExit();
}
