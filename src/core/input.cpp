/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * modified by gave92
 *
 * WiiBrowser
 * input.cpp
 * Wii/GameCube controller management
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include "main.h"
#include "menu.h"
#include "video.h"
#include "input.h"

#include "gui.h"
#include "settings.h"

#include "input.h"
#include "osdep/gx_supp.h"
#include "mplayerwii.h"

int rumbleRequest[4] = {0,0,0,0};
GuiTrigger userInput[4];
InputState g_inputState[4] = {};
static int rumbleCount[4] = {0,0,0,0};

bool Input_IsHeld(int chan, InputButton btn) {
    if(chan < 0 || chan >=4) return false;
    return (g_inputState[chan].held & (1u << btn)) != 0;
}
bool Input_IsDown(int chan, InputButton btn) {
    if(chan < 0 || chan >=4) return false;
    return (g_inputState[chan].down & (1u << btn)) != 0;
}

static u16 mapButtonsToInput(u32 wpadHeld, u32 wpadDown, u32 padHeld, u32 padDown) {
    u16 held=0, down=0;
    if((wpadHeld & WPAD_BUTTON_A) || (wpadHeld & WPAD_CLASSIC_BUTTON_A) || (padHeld & PAD_BUTTON_A)) held |= 1u<<INPUT_BTN_A;
    if((wpadDown & WPAD_BUTTON_A) || (wpadDown & WPAD_CLASSIC_BUTTON_A) || (padDown & PAD_BUTTON_A)) down |= 1u<<INPUT_BTN_A;
    if((wpadHeld & WPAD_BUTTON_B) || (wpadHeld & WPAD_CLASSIC_BUTTON_B) || (padHeld & PAD_BUTTON_B)) held |= 1u<<INPUT_BTN_B;
    if((wpadDown & WPAD_BUTTON_B) || (wpadDown & WPAD_CLASSIC_BUTTON_B) || (padDown & PAD_BUTTON_B)) down |= 1u<<INPUT_BTN_B;
    if((wpadHeld & WPAD_BUTTON_PLUS) || (wpadHeld & WPAD_CLASSIC_BUTTON_PLUS)) held |= 1u<<INPUT_BTN_PLUS;
    if((wpadDown & WPAD_BUTTON_PLUS) || (wpadDown & WPAD_CLASSIC_BUTTON_PLUS)) down |= 1u<<INPUT_BTN_PLUS;
    if((wpadHeld & WPAD_BUTTON_MINUS) || (wpadHeld & WPAD_CLASSIC_BUTTON_MINUS)) held |= 1u<<INPUT_BTN_MINUS;
    if((wpadDown & WPAD_BUTTON_MINUS) || (wpadDown & WPAD_CLASSIC_BUTTON_MINUS)) down |= 1u<<INPUT_BTN_MINUS;
    if((wpadHeld & WPAD_BUTTON_HOME) || (wpadHeld & WPAD_CLASSIC_BUTTON_HOME) || (padHeld & PAD_BUTTON_MENU)) held |= 1u<<INPUT_BTN_HOME;
    if((wpadDown & WPAD_BUTTON_HOME) || (wpadDown & WPAD_CLASSIC_BUTTON_HOME) || (padDown & PAD_BUTTON_MENU)) down |= 1u<<INPUT_BTN_HOME;
    // D-pad mapping (WPAD + Classic + PAD)
    if((wpadHeld & WPAD_BUTTON_UP) || (wpadHeld & WPAD_CLASSIC_BUTTON_UP) || (padHeld & PAD_BUTTON_UP)) held |= 1u<<INPUT_BTN_UP;
    if((wpadDown & WPAD_BUTTON_UP) || (wpadDown & WPAD_CLASSIC_BUTTON_UP) || (padDown & PAD_BUTTON_UP)) down |= 1u<<INPUT_BTN_UP;
    if((wpadHeld & WPAD_BUTTON_DOWN) || (wpadHeld & WPAD_CLASSIC_BUTTON_DOWN) || (padHeld & PAD_BUTTON_DOWN)) held |= 1u<<INPUT_BTN_DOWN;
    if((wpadDown & WPAD_BUTTON_DOWN) || (wpadDown & WPAD_CLASSIC_BUTTON_DOWN) || (padDown & PAD_BUTTON_DOWN)) down |= 1u<<INPUT_BTN_DOWN;
    if((wpadHeld & WPAD_BUTTON_LEFT) || (wpadHeld & WPAD_CLASSIC_BUTTON_LEFT) || (padHeld & PAD_BUTTON_LEFT)) held |= 1u<<INPUT_BTN_LEFT;
    if((wpadDown & WPAD_BUTTON_LEFT) || (wpadDown & WPAD_CLASSIC_BUTTON_LEFT) || (padDown & PAD_BUTTON_LEFT)) down |= 1u<<INPUT_BTN_LEFT;
    if((wpadHeld & WPAD_BUTTON_RIGHT) || (wpadHeld & WPAD_CLASSIC_BUTTON_RIGHT) || (padHeld & PAD_BUTTON_RIGHT)) held |= 1u<<INPUT_BTN_RIGHT;
    if((wpadDown & WPAD_BUTTON_RIGHT) || (wpadDown & WPAD_CLASSIC_BUTTON_RIGHT) || (padDown & PAD_BUTTON_RIGHT)) down |= 1u<<INPUT_BTN_RIGHT;
    // Merge into state's held/down for caller (we return held part; caller handles down separately)
    (void)down;
    return held;
}

static int osdLevel = 0;
static int volprev = 0, volnow = 0;
static int resizeprev = 0, resizeinitial = 0;

#define VOL_DELAY				30000
#define VOLDISP_MAX				500000

#define RESIZE_INITIAL_DELAY	500000 // to allow more precise adjustment
#define RESIZE_DELAY			5000

/****************************************************************************
 * UpdatePads
 *
 * Scans pad and wpad
 ***************************************************************************/
void UpdatePads()
{
	WPAD_ScanPads();
	PAD_ScanPads();

	for(int i=3; i >= 0; i--)
	{
		userInput[i].pad.btns_d = PAD_ButtonsDown(i);
		userInput[i].pad.btns_u = PAD_ButtonsUp(i);
		userInput[i].pad.btns_h = PAD_ButtonsHeld(i);
		userInput[i].pad.stickX = PAD_StickX(i);
		userInput[i].pad.stickY = PAD_StickY(i);
		userInput[i].pad.substickX = PAD_SubStickX(i);
		userInput[i].pad.substickY = PAD_SubStickY(i);
		userInput[i].pad.triggerL = PAD_TriggerL(i);
		userInput[i].pad.triggerR = PAD_TriggerR(i);

		// M4: populate unified InputState (includes WPAD Classic + GC + future Wii U Pro via libwiidrc)
		u32 wpadHeld=0, wpadDown=0;
		if(userInput[i].wpad) {
		    wpadHeld = userInput[i].wpad->btns_h;
		    wpadDown = userInput[i].wpad->btns_d;
		    // Classic extension buttons are in expansion: WPAD_Exp* classic->btns_h etc handled via WPAD Buttons already (libogc merges)
		}
		u32 padHeld = PAD_ButtonsHeld(i);
		u32 padDown = PAD_ButtonsDown(i);
		// Stick threshold -> D-pad emulation
		if(PAD_StickX(i) < -PADCAL) padHeld |= PAD_BUTTON_LEFT;
		if(PAD_StickX(i) > PADCAL)  padHeld |= PAD_BUTTON_RIGHT;
		if(PAD_StickY(i) < -PADCAL) padHeld |= PAD_BUTTON_DOWN;
		if(PAD_StickY(i) > PADCAL)  padHeld |= PAD_BUTTON_UP;
		g_inputState[i].held = mapButtonsToInput(wpadHeld, wpadDown, padHeld, padDown);
		// Down mask stored separately for Input_IsDown queries
		{
		    u16 downMask=0;
		    if((wpadDown & WPAD_BUTTON_A) || (wpadDown & WPAD_CLASSIC_BUTTON_A) || (padDown & PAD_BUTTON_A)) downMask |= 1u<<INPUT_BTN_A;
		    if((wpadDown & WPAD_BUTTON_B) || (wpadDown & WPAD_CLASSIC_BUTTON_B) || (padDown & PAD_BUTTON_B)) downMask |= 1u<<INPUT_BTN_B;
		    g_inputState[i].down = downMask;
		    g_inputState[i].up = 0; // TODO: PAD_ButtonsUp mapping
		}
		g_inputState[i].stickX = PAD_StickX(i);
		g_inputState[i].stickY = PAD_StickY(i);
		// If Wii U Pro via libwiidrc is present, it appears as classic; future: if WUPC detected, merge here
	}

	// M4: handle Wii U Pro Controller via optional libwiidrc (if linked)
	// Probe slot 4-7 for WUPC — handled gracefully if lib not present (weak symbol)
	extern void WUPC_Update(void) __attribute__((weak));
	if(WUPC_Update) WUPC_Update();
}

/****************************************************************************
 * SetupPads
 *
 * Sets up userInput triggers for use
 ***************************************************************************/
void SetupPads()
{
	PAD_Init();
	WPAD_Init();

	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

	for(int i=0; i < 4; i++)
	{
		userInput[i].chan = i;
		userInput[i].wpad = WPAD_Data(i);
	}
}

/****************************************************************************
 * ShutoffRumble
 ***************************************************************************/

void ShutoffRumble()
{
	for(int i=0;i<4;i++)
	{
		WPAD_Rumble(i, 0);
		rumbleCount[i] = 0;
	}
}

/****************************************************************************
 * DoRumble
 ***************************************************************************/

void DoRumble(int i)
{
    if(Settings.MuteSound)
    {
        WPAD_Rumble(i, 0); // rumble off
        return;
    }

	if(rumbleRequest[i] && rumbleCount[i] < 3)
	{
		WPAD_Rumble(i, 1); // rumble on
		rumbleCount[i]++;
	}
	else if(rumbleRequest[i])
	{
		rumbleCount[i] = 12;
		rumbleRequest[i] = 0;
	}
	else
	{
		if(rumbleCount[i])
			rumbleCount[i]--;
		WPAD_Rumble(i, 0); // rumble off
	}
}

/****************************************************************************
 * MPlayerInput
 ***************************************************************************/

void MPlayerResize(float fZoomHorIncr, float fZoomVertIncr)
{
	/*
	WiiSettings.videoZoomHor += fZoomHorIncr;
	WiiSettings.videoZoomVert += fZoomVertIncr;
	GX_SetScreenPos(WiiSettings.videoXshift, WiiSettings.videoYshift,
		WiiSettings.videoZoomHor, WiiSettings.videoZoomVert);
    */
}

void MPlayerInput()
{
	bool ir = false;
	bool inDVDMenu = false;
	static bool volumeUpdated = false;

	if(userInput[0].wpad->ir.valid)
		ir = true;

	if(userInput[0].wpad->btns_d & WPAD_BUTTON_1)
		osdLevel ^= 1;
	else if(ExitRequested || userInput[0].wpad->btns_d & WPAD_BUTTON_HOME)
		wiiGotoGui();

	if(!inDVDMenu)
	{
		if(userInput[0].wpad->btns_d & WPAD_BUTTON_A)
		{
			// Hack to allow people to unpause while the OSD GUI is visible by
			// pointing above the button bar and pressing A. We also need to be outside
			// the boundaries of the volume bar area, when it is visible
			int x = userInput[0].wpad->ir.x;
			int y = userInput[0].wpad->ir.y;

			int xoffset = 20;

			if(screenwidth == 768)
				xoffset = 80;

			if(!drawGui || (y < 360 &&
				(!(x > xoffset && x < xoffset+100 && y > 180))))
			{
				wiiPause();
			}
		}
		else if(userInput[0].wpad->btns_h & WPAD_BUTTON_PLUS)
		{
			volnow = gettime();

			if(diff_usec(volprev, volnow) > VOL_DELAY)
			{
				volprev = volnow;
				wiiSetVolume(50);
				volumeUpdated = true;
			}
		}
		else if(userInput[0].wpad->btns_h & WPAD_BUTTON_MINUS)
		{
			volnow = gettime();

			if(diff_usec(volprev, volnow) > VOL_DELAY)
			{
				volprev = volnow;
				wiiSetVolume(50);
				volumeUpdated = true;
			}
		}
		else if (userInput[0].wpad->btns_h & WPAD_BUTTON_B)
		{
			unsigned int delay = (resizeinitial == 1) ? RESIZE_INITIAL_DELAY : RESIZE_DELAY;
			int resizenow = gettime();

			if(userInput[0].wpad->btns_h & WPAD_BUTTON_RIGHT)
			{
				if(diff_usec(resizeprev, resizenow) > delay)
				{
					resizeinitial++;
					resizeprev = resizenow;
					MPlayerResize(+0.003F, 0.00F);
				}
			}
			else if(userInput[0].wpad->btns_h & WPAD_BUTTON_LEFT)
			{
				if(diff_usec(resizeprev, resizenow) > delay)
				{
					resizeinitial++;
					resizeprev = resizenow;
					MPlayerResize(-0.003F, 0.00F);
				}
			}
			else if(userInput[0].wpad->btns_h & WPAD_BUTTON_UP)
			{
				if(diff_usec(resizeprev, resizenow) > delay)
				{
					resizeinitial++;
					resizeprev = resizenow;
					MPlayerResize(0.00F, +0.003F);
				}
			}
			else if(userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN)
			{
				if(diff_usec(resizeprev, resizenow) > delay)
				{
					resizeinitial++;
					resizeprev = resizenow;
					MPlayerResize(0.00F, -0.003F);
				}
			}

			if(userInput[0].wpad->btns_d & (WPAD_BUTTON_RIGHT | WPAD_BUTTON_LEFT |
											WPAD_BUTTON_UP | WPAD_BUTTON_DOWN))
			{
				resizeinitial = 0;
			}
		}
		else if(userInput[0].wpad->btns_d & WPAD_BUTTON_RIGHT)
		{
			wiiFastForward();
		}
		else if(userInput[0].wpad->btns_d & WPAD_BUTTON_LEFT)
		{
			wiiRewind();
		}
		else if(userInput[0].wpad->btns_d & WPAD_BUTTON_UP)
		{
			if(!wiiIsPaused())
				wiiSetProperty(MP_CMD_SUB_SELECT, 0);
		}
		else if(userInput[0].wpad->btns_d & WPAD_BUTTON_DOWN)
		{
			if(!wiiIsPaused())
				wiiSetProperty(MP_CMD_SWITCH_AUDIO, 0);
		}
		else if(userInput[0].wpad->btns_d & WPAD_BUTTON_2)
		{
			wiiDVDNav(MP_CMD_DVDNAV_MENU);
		}
	}

	if(volumeUpdated)
	{
		volnow = gettime();

		if(volnow > volprev && diff_usec(volprev, volnow) > VOLDISP_MAX)
			volumeUpdated = false;
		else
			ir = true; // trigger display
	}

	if(ir || osdLevel)
	{
		drawGui = true;
	}
	else if(drawGui)
	{
		drawGui = false;
		ShutoffRumble();
	}
}
