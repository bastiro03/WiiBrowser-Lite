/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * update.cpp
 * Wii/GameCube auto-update management
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "html.h"

bool downloadUpdate(int appversion) {
	char updateFile[30];
	sprintf(updateFile, "update.dol");
    bool result = false;

    char url[30];
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

	curl_easy_cleanup(curl_upd);
	return result;
}

int checkUpdate() {
    char updateFile[] = "update.cfg";
    int result = 0;

    char url[] = "https://dl.dropbox.com/s/wp0xh4gloks9i2p/wiibrowser.cfg?dl=1";
    struct block HTML;

	FILE *file = fopen("wiibrowser.cfg", "r");
	if (!file)
        return 0;

    CURL *curl_upd = curl_easy_init();
	FILE *hfile = fopen(updateFile, "wb");

	if (hfile)
	{
	    HTML = downloadfile(curl_upd, url, hfile);
		if(HTML.size>0)
		{
            int old_v, new_v;
            hfile = fopen(updateFile, "r");
            fscanf (file, "APPVERSION: R%d", &old_v);
            fscanf (hfile, "APPVERSION: R%d", &new_v);

            if (new_v>old_v) {
                remove("wiibrowser.cfg");
                rename(updateFile, "wiibrowser.cfg");
                result = new_v;
            }
		}
        fclose(hfile);
        remove(updateFile); // delete update file
	}

	curl_easy_cleanup(curl_upd);
	return result;
}
