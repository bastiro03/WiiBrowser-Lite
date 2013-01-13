 /****************************************************************************
 * Copyright (C) 2009
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * Settings.h
 *
 * Settings Class
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <string.h>
#include <stdio.h>
#include <gctypes.h>

#define N 9

enum {
	LANG_JAPANESE = 0,
	LANG_ENGLISH,
	LANG_GERMAN,
	LANG_FRENCH,
	LANG_SPANISH,
	LANG_ITALIAN,
	LANG_DUTCH,
	LANG_SIMP_CHINESE,
	LANG_TRAD_CHINESE,
	LANG_KOREAN,
	LANG_ROMANIAN,
	LANG_ESTONIAN,
	LANG_BRAZILIAN_PORTUGUESE,
	LANG_HUNGARIAN,
	LANG_POLISH,
	LANG_RUSSIAN,
	LANG_PORTUGUESE,
	LANG_SLOVAK,
	LANG_TAMIL,
	LANG_SWEDISH,
	LANG_DANISH,
	LANG_BULGARIAN,
	LANG_LENGTH
};

class SSettings
{
    public:
		//!Constructor
        SSettings();
		//!Destructor
		~SSettings();
		//!Set Default Settings
		void SetDefault();
		//!Load Settings
		bool Load();
        //!Find the config file in the default paths
        bool FindConfig();
		//!Check folder
        int CheckFolder(const char* folder);
        //!Check file
        bool CheckFile(const char* path);
        //!Check file integrity
        bool CheckIntegrity(const char *path);
        //!Return favorite
        char *GetUrl(int f);
        //!Save Settings
        bool Save();
		//!Reset Settings
        bool Reset();
		//!Set a Setting
		//!\param name Settingname
		//!\param value Settingvalue
        bool SetSetting(char * name, char * value);
        //!Variables
        int Language;
        int Revision;
        bool ShowTooltip;
        bool Autoupdate;
        bool Music;

        char Homepage[256];
        char DefaultFolder[256];
        char ConfigPath[256];
        char BootDevice[6];
        char *Favorites[N];

    protected:
        void ParseLine(char *line);
        void TrimLine(char *dest, char *src, int size);
        FILE * file;
};

enum
{
    FILENAME = 0,
    RELATIVE,
    ABSOLUTE
};

enum
{
    SD = 0,
    USB1,
    USB2,
    USB3,
    USB4,
    USB5,
    USB6,
    USB7,
    USB8,
	DVD,
    MAXDEVICES
};

const char DeviceName[MAXDEVICES][6] =
{
    "sd",
    "usb1",
    "usb2",
    "usb3",
    "usb4",
    "usb5",
    "usb6",
    "usb7",
    "usb8",
	"dvd",
};

#endif
