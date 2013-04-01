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

bool downloadUpdate(int appversion) {
	char updateFile[30];
	sprintf(updateFile, "update.dol");
    bool result = false;

    char url[50];
	sprintf(url, "http://wiibrowser.googlecode.com/files/R%d.dol", appversion);
    struct block HTML;

    save_mem("starting update");
    CURL *curl_upd = curl_easy_init();
	FILE *hfile = fopen(updateFile, "wb");

	if (hfile)
	{
	    HTML = downloadfile(curl_upd, url, hfile);
		if(HTML.size>0)
		{
		    save_mem("got file");
		    remove("boot.dol");
		    result = (rename(updateFile, "boot.dol")==0);
		}
		if(HTML.size==0 || !result) {
		    fclose(hfile);
            remove(updateFile); // delete update file
		}
	}
	if (result)
	{
	    Settings.Revision = appversion;
	    Settings.Save();
	}

	curl_easy_cleanup(curl_upd);
	return result;
}

int checkUpdate() {
    char updateFile[] = "update.cfg";
    int result = 0;

    char url[] = "https://dl.dropbox.com/s/wp0xh4gloks9i2p/wiibrowser.cfg?dl=1";
    // char url[] = "http://static.tumblr.com/4piuknb/rqKmgml5e/wiibrowser.cfg";
    struct block HTML;

    CURL *curl_upd = curl_easy_init();
	FILE *hfile = fopen(updateFile, "wb");

	if (hfile)
	{
	    HTML = downloadfile(curl_upd, url, hfile);
		if(HTML.size>0)
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
