/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * modified by gave92
 *
 * WiiBrowser
 * update.cpp
 * Wii/GameCube auto-update management
 ***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "common.h"
#include "networkop.h"

bool downloadUpdate(int appversion) {
	char updateFile[30];
	sprintf(updateFile, "update.dol");

    bool result = false;
    char url[50];

	if (Settings.Autoupdate == STABLE)
        sprintf(url, "http://wiibrowser.googlecode.com/files/R%d.dol", appversion);
    else sprintf(url, "https://dl.dropbox.com/s/w39ycq91i3wwvn5/boot.dol?dl=1");

	Private *data = NULL;
    FILE *hfile = fopen(updateFile, "wb");

	if (hfile)
	{
	    data = AddHandle(curl_multi, url, hfile);
        while(data->code < 0)
            usleep(100);

		if(!data->code)
		{
		    remove("boot.dol");
		    result = (rename(updateFile, "boot.dol")==0);
		}
		if(data->code || !result)
		{
            remove(updateFile); // delete update file
		}
	}
	if (result)
	{
	    Settings.Revision = appversion;
	    Settings.Save();
	}

	free(data->url);
	delete(data);
	return result;
}

int checkUpdate() {
	char updateFile[30];
	sprintf(updateFile, "update.cfg");
    int result = 0;

    char url[70];
    struct block HTML;

    if (Settings.Autoupdate == STABLE)
        sprintf(url, "https://dl.dropbox.com/s/wp0xh4gloks9i2p/wiibrowser.cfg?dl=1");
    else sprintf(url, "https://dl.dropbox.com/s/acpbajdid91d7it/nightly.cfg?dl=1");

    CURL *curl_upd = curl_easy_init();
	FILE *hfile = fopen(updateFile, "wb");

	if (hfile)
	{
	    HTML = downloadfile(curl_upd, url, hfile);
		if(HTML.size)
		{
            int old_v, new_v;
            hfile = fopen(updateFile, "r");
            old_v = Settings.Revision;
            fscanf (hfile, "APPVERSION: R%d", &new_v);

            if (new_v>old_v)
                result = new_v;
		}
        fclose(hfile);
        remove(updateFile); // delete update file
	}

	curl_easy_cleanup(curl_upd);
	return result;
}
