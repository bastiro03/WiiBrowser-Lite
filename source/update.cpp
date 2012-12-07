/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * update.cpp
 * Wii/GameCube auto-update management
 ***************************************************************************/

#include <stdio.h>
#include <html.h>
#include <string.h>

bool downloadUpdate() {
	char updateFile[30];
	sprintf(updateFile, "update.dol"); // sprintf(updateFile, "%s update.dol", APPNAME);
    bool result = false;

    char url[30];
	sprintf(url, "http://192.168.0.2/boot.dol");
    struct block HTML;

    save_mem("starting update");
    CURL *curl_upd = curl_easy_init();
	FILE *hfile = fopen (updateFile, "wb");

	if (hfile)
	{
	    HTML = downloadfile(curl_upd, url, hfile);
		if(HTML.size>0)
		{
		    save_mem("got file");
		    remove("boot.dol");
		    result=(rename(updateFile, "boot.dol")==0);
		}
		if(HTML.size==0 || !result) {
		    fclose(hfile);
            remove(updateFile); // delete update file
		}
	}
	curl_easy_cleanup(curl_upd);
	return result;
}

bool checkUpdate() {
    char updateFile[] = "check update.cfg";
    bool result = false;

    char url[] = "http://192.168.0.2/wiixplore.cfg";
    struct block HTML;

	FILE *file = fopen ("wiixplore.cfg", "r");
	if (!file)
        return true;

    CURL *curl_upd = curl_easy_init();
	FILE *hfile = fopen (updateFile, "wb");

	if (hfile)
	{
	    HTML = downloadfile(curl_upd, url, hfile);
		if(HTML.size>0)
		{
            float old_v, new_v;
            hfile = fopen (updateFile, "r");
            fscanf (file, "APPVERSION: %f", &old_v);
            fscanf (hfile, "APPVERSION: %f", &new_v);

            if (new_v>old_v) {
                remove("wiixplore.cfg");
                rename(updateFile, "wiixplore.cfg");
                result=true;
            }
		}
        fclose(hfile);
        remove(updateFile); // delete update file
	}

	curl_easy_cleanup(curl_upd);
	return result;
}
