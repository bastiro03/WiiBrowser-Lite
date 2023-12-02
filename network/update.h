/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * modified by gave92
 *
 * WiiBrowser
 * update.h
 * Wii/GameCube auto-update management
 ***************************************************************************/

#ifndef _UPDATE_H_
#define _UPDATE_H_

using update = struct update
{
	int appversion;
	char changelog[512];
};

extern bool downloadUpdate(int appversion);
extern struct update checkUpdate();

#endif
