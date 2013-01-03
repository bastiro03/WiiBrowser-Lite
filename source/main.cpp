/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * main.cpp
 * Basic template/demonstration of libwiigui capabilities. For a
 * full-featured app using many more extensions, check out Snes9x GX.
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fat.h>

#include "FreeTypeGX.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "gettext.h"
#include "liste.h"
#include "input.h"
#include "filelist.h"
#include "main.h"

SSettings Settings;
int ExitRequested = 0,
    GuiShutdown = 0;

void ExitApp()
{
    Settings.Save();
	ShutoffRumble();
	StopGX();
	ShutdownAudio();
	DeinitFreeType();
	FreeHistory(history);
	StopUpdateThread();
	if (HWButton)
        SYS_ResetSystem(HWButton, 0, 0);
	exit(0);
}

u8 HWButton;
void WiiResetPressed() { HWButton = SYS_RETURNTOMENU; }
void WiiPowerPressed() { HWButton = SYS_POWEROFF_STANDBY; }
void WiimotePowerPressed(s32 chan) { HWButton = SYS_POWEROFF_STANDBY; }

int main(int argc, char *argv[])
{
	SYS_SetResetCallback(WiiResetPressed);
	SYS_SetPowerCallback(WiiPowerPressed);
	WPAD_SetPowerButtonCallback(WiimotePowerPressed);

	InitVideo(); // Initialize video
	SetupPads(); // Initialize input
	InitAudio(); // Initialize audio
	fatInitDefault(); // Initialize file system
	InitFreeType(); // Initialize font system
	InitGUIThreads(); // Initialize GUI

    __exception_setreload(10);
	Settings.Load();
	LoadLanguage();
	MainMenu(MENU_SPLASH);
	ExitApp();
}
