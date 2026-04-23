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

enum
{
    DISABLED = 0,
    STABLE,
    NIGHTLY,
    CHANNELS
};

enum
{
    NEVER = 0,
    UNZIP,
    ASK
};

typedef struct favorite
{
    char url[512];
    char name[512];
} favorite;

class SSettings
{
    public:
		//!Constructor
        SSettings();
		//!Destructor
		~SSettings();
		//!Set Default Settings
		void SetDefault();
		//!Set Default folder
		void ChangeFolder();
		//!Load Settings
		bool Load();
        //!Find the config file in the default paths
        bool FindConfig();
        //!Return favorite
        struct favorite *GetFav(int f);
        //!Return top site
        char *GetUrl(int f);
        //!Return index
        int FindUrl(char *url);
        //!Remove top site
        void Remove(int f, bool update = false);
        //!Check if url is in favorites
        int IsBookmarked(char *url);
        //!Add favorite
        void AddFavorite(char *url, char *title);
        //!Remove favorite
        void DelFavorite(char *url);
        //!Save Settings
        bool Save(bool clean);
		//!Reset Settings
        bool Reset();
		//!Set a Setting
		//!\param name Settingname
		//!\param value Settingvalue
        bool SetSetting(char *name, char *value);
        //!Set a Startup Setting
		//!\param name Settingname
		//!\param value Settingvalue
        bool SetStartup(char *name, char *value);
        //!Set Start Page
		void SetStartPage(char *page);
        //!Get Start Page
		int GetStartPage(char *dest);

        //!Variables
        int Language;
        int UserAgent;
        int ZipFile;
        int Autoupdate;
        int RevInt;

        bool MuteSound;
        bool ShowTooltip;
        bool ShowThumbs;
        bool Restore;

        char AppPath[256];
        char ConfigPath[256];
        char BootDevice[6];

        char Homepage[512];
        char Revision[7];
        char Uuid[20];

        char DefaultFolder[256];
        char UserFolder[256];
        char Proxy[256];

        //!Plugin settings
        char StartPage[512];
        char DownloadFolder[256];

        bool DocWrite;
        bool IFrame;
        bool ExecLua;
        bool CleanExit;

        char *TopSites[N];
        u8 *Thumbnails[N];

        struct favorite *Favorites;
    protected:
        //!Find value
        void ParseLine(char *line);
        //!Discard spaces
        void TrimLine(char *dest, char *src, int size);
        //!Check file
        bool CheckFile(const char* path);
        //!Check file integrity
        bool CheckIntegrity(const char *path);
        //!Check path
        bool IsWritable(const char *path);
        //!Load favorites
        bool LoadFavorites();
        //!Save favorites
        bool SaveFavorites();
        //!Create XML file
        bool CreateXMLFile();
        //!Variables
        FILE * file;
        int num_fav;
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
    USB,
    CARDA,
    CARDB,
	DVD,
    MAXDEVICES
};

const char DeviceName[MAXDEVICES][6] =
{
    "sd",
    "usb",
    "carda",
    "cardb",
	"dvd",
};

enum
{
    EMPTY = 0,
    LIBCURL,
    WIIBROWSER,
    MOZILLA,
	IEXPLORER,
	ANDROID,
    MAXAGENTS
};

const char AgentName[MAXAGENTS][11] =
{
    "None",
    "Libcurl",
    "WiiBrowser",
    "Firefox",
	"IExplorer",
	"Android",
};

int CheckFolder(const char *folder);

#endif
