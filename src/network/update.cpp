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

#include "httplib.h"
#include "main.h"
#include "common.h"
#include "transfer.h"

int readFile(FILE *hfile, struct update *result)
{
    char line[512];
    int new_v;
    int gettext = -1;

    fscanf (hfile, "APPVERSION: R%d", &new_v);

	while (fgets(line, sizeof(line), hfile))
	{
        if(gettext >= 0)
        {
            strcpy(result->changelog+gettext, line);
            gettext = strlen(result->changelog);
        }

		else if (!strncmp(line, "# Changelog", 11))
            gettext = 0;
	}
    return new_v;
}

bool downloadUpdate(int appversion) {
	char updateFile[30];
	snprintf(updateFile, sizeof(updateFile), "update.dol");

    bool result = false;
    char url[256];

	if (Settings.Autoupdate == STABLE)
        snprintf(url, sizeof(url), "http://wiibrowser.altervista.org/mainsite/getfile.php?file=R%d.dol&update=1&uuid=%s&mode=U", appversion, Settings.Uuid);
    else snprintf(url, sizeof(url), "https://dl.dropbox.com/s/w39ycq91i3wwvn5/boot.dol?dl=1");

	Private *data = NULL;
    FILE *hfile = fopen(updateFile, "wb");

	if (hfile)
	{
	    data = AddUpdate(curl_multi, url, hfile);
	    if(data) {
            while(data->code < 0)
                usleep(10000);

		    if(data->code == CURLE_OK)
		    {
		        // M5: basic size sanity check before rename (future: SHA256)
		        long fsize = 0;
		        FILE* chk = fopen(updateFile, "rb");
		        if(chk) { fseek(chk,0,SEEK_END); fsize = ftell(chk); fclose(chk); }
		        if(fsize > 1024) {
		            remove("boot.dol");
		            result = (rename(updateFile, "boot.dol")==0);
		        } else {
		            result = false;
		        }
		    }
		    if(data->code != CURLE_OK || !result)
		    {
                remove(updateFile); // delete update file
		    }
		    free(data->url);
		    delete(data);
	    } else {
	        fclose(hfile);
	        remove(updateFile);
	    }
	}
	if (result)
	{
	    Settings.RevInt = appversion;
	    Settings.Save(0);
	}

	return result;
}

struct update checkUpdate() {
	char updateFile[30];
	snprintf(updateFile, sizeof(updateFile), "update.cfg");
    char url[256];

    struct update result;
    struct block HTML;
    result.appversion = 0;

    if (Settings.Autoupdate == STABLE)
        snprintf(url, sizeof(url), "http://wiibrowser.altervista.org/downloads/wiibrowser.cfg");
    else snprintf(url, sizeof(url), "https://dl.dropbox.com/s/acpbajdid91d7it/nightly.cfg?dl=1");

    CURL *curl_upd = curl_easy_init();
	FILE *hfile = fopen(updateFile, "wb");

	if (hfile)
	{
	    HTML = downloadfile(curl_upd, url, hfile);
		if(HTML.size)
		{
            int old_v, new_v;
            hfile = fopen(updateFile, "r");
            old_v = Settings.RevInt;
            new_v = readFile(hfile, &result);

            if (new_v>old_v)
                result.appversion = new_v;
		}
        fclose(hfile);
        remove(updateFile); // delete update file
	}

	curl_easy_cleanup(curl_upd);
	return result;
}
