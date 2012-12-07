/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.h
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#undef TIME
#undef DEBUG

#include <ogcsys.h>
#include <curl/curl.h>
#include <update.h>

void InitGUIThreads();
void MainMenu (int menuitem);
void StopUpdateThread();

extern u8 HWButton;
extern CURL *curl_handle;

enum
{
	MENU_EXIT = -1,
	MENU_NONE,
	SPLASH,
	MENU_SETTINGS,
	MENU_SETTINGS_FILE,
	MENU_BROWSE_DEVICE,
	HOME,
	MENU_DEFAULT
};

enum
{
    NET_ERR = 1,
    IP_ERR
};

#endif
