/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * modified by gave92
 *
 * WiiBrowser
 * menu.h
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#undef TIME
#undef DEBUG

#include <libwiigui/gui.h>
#include <curl/multi.h>

#include <ogcsys.h>
#include <update.h>
#include <string>

extern "C" {
#include "entities.h"
}

void InitGUIThreads();
void MainMenu(int menuitem);
void StopGUIThreads();
void Cleanup();

void EnableVideoImg();
bool VideoImgVisible();
bool LoadYouTubeFile(char* newurl, char* data);

extern u8 HWButton;
extern char new_page[];

extern CURL* curl_handle;
extern CURLM* curl_multi;
extern CURLSH* curl_share;

enum
{
	MENU_EXIT = -1,
	MENU_NONE,
	MENU_SPLASH,
	MENU_SETTINGS,
	MENU_DEVELOPER,
	MENU_TOPSITES,
	MENU_FAVORITES,
	MENU_BROWSE,
	MENU_BROWSE_DEVICE,
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
void UpdatePointer();
void DoMPlayerGuiDraw();

void SetPointer(bool drag, int chan);
void ShowAction(const char* msg);
void SetMessage(const char* msg);

void CancelAction();
bool CancelDownload();
#ifdef __cplusplus
}
#endif

#endif
