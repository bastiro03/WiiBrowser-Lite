/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * demo.cpp
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

#include "utils/mem2_manager.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "main.h"
#include "liste.h"
#include "FreeTypeGX.h"

int ExitRequested = 0;

void ExitApp()
{
	ShutoffRumble();
	StopGX();
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

int main(int argc, char *argv[]) {
	SYS_SetResetCallback(WiiResetPressed);
	SYS_SetPowerCallback(WiiPowerPressed);
	WPAD_SetPowerButtonCallback(WiimotePowerPressed);

	SetupPads(); // Initialize input
	InitVideo(); // Initialise video
	InitAudio(); // Initialize audio
	fatInitDefault(); // Initialize file system

	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
    AddMem2Area (15*1024*1024, MEM2_GUI);
    AddMem2Area (5*1024*1024, MEM2_OTHER); // vars + ttf

    fontSystem = new FreeTypeGX(font_regular_ttf, font_regular_ttf_size);   // Initialize font system
    __exception_setreload(10);

	InitGUIThreads();
	MainMenu(SPLASH);
}
