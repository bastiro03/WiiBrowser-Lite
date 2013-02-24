/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
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
#include "libwiigui/gui.h"

#include "mplayer/input/input.h"
#include "mplayer/osdep/gx_supp.h"

int rumbleRequest[4] = {0,0,0,0};
GuiTrigger userInput[4];
static int rumbleCount[4] = {0,0,0,0};

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
	}
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
