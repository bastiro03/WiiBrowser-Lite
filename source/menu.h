/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.h
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#define MAXLEN          256
#undef TIME
#undef DEBUG

#include <libwiigui/gui.h>
#include <curl/curl.h>

#include <ogcsys.h>
#include <update.h>
#include <string>

extern "C" {
#include "entities.h"
}

void InitGUIThreads();
void MainMenu (int menuitem);
void StopUpdateThread();

void EnableVideoImg();
bool VideoImgVisible();

extern u8 HWButton;
extern GuiImage *bgImg;
extern CURL *curl_handle;
extern GuiImage *bgImg;

enum
{
	MENU_EXIT = -1,
	MENU_NONE,
	MENU_SPLASH,
	MENU_SETTINGS,
	MENU_FAVORITES,
	MENU_BROWSE,
	MENU_HOME,
	MENU_DEFAULT
};

enum
{
    NET_ERR = 1,
    IP_ERR
};

#ifdef __cplusplus
extern "C" {
#endif
    void DisableVideoImg();
    void DoMPlayerGuiDraw();
#ifdef __cplusplus
}
#endif

#endif
