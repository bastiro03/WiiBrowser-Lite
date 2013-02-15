/*
**  Created by gave92
**
**  Copyright (C) 2012
**  ==================
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, version 2.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**  History:
**   20-11-2008  Start programming
**   30-11-2008  Added google analysic http request
**   07-12-2008  Add user_var parameter in google analysic http request
**   30-12-2008  Added store highscore on internet.
**   06-01-2009  Google analysic function improved.
**   15-01-2009  Add http chunked frame support.
**   21-01-2009  Create one unique cookie during startup.
**   26-01-2009  Increase internal buffersize to 8kB.
*/

#include <stdio.h>
#include <stdlib.h>

#include "httplib.h"
#include "main.h"

// -----------------------------------------------------------
// VARIABLES
// -----------------------------------------------------------

const char Agents[MAXAGENTS][256] =
{
    "",
    "libcurl-agent/1.0",
    "Mozilla/5.0 (Nintendo Wii; U; WiiBrowser rev37) like Gecko Firefox/4.0",
    "Mozilla/5.0 (Windows NT 6.2; WOW64; rv:16.0.1) Gecko/20121011 Firefox/16.0.1",
	"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0)",
	"Mozilla/5.0 (Linux; U; Android 1.1; en-gb; dream) AppleWebKit/525.10+ (KHTML, like Gecko) Version/3.0.4 Mobile Safari/523.12.2",
};

const struct block emptyblock = {NULL, 0};

// -----------------------------------------------------------
// FUNCTIONS
// -----------------------------------------------------------

char *findChr (const char *str, char chr);

int close_callback (void *clientp, curl_socket_t item) {
	return net_close(item);
}

struct MemoryStruct {
    char *memory;
    u32 size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("not enough memory (realloc returned NULL)\n");
        exit(EXIT_FAILURE);
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->memory[mem->size+=realsize] = 0;
    return realsize;
}

struct block downloadfile(CURL *curl_handle, const char *url, FILE *hfile)
{
    char *ct=NULL;
    struct block b;
    int res;

    struct MemoryStruct chunk;
    chunk.memory = (char *)malloc(1);   /* will be grown as needed by the realloc above */
    chunk.size = 0; /* no data at this point */

    if(curl_handle) {
        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_AUTOREFERER, 1);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

        /* some servers don't like requests that are made without a user-agent
        field, so we provide one */
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, Agents[Settings.UserAgent]);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);

        /* proper function to close sockets */
        curl_easy_setopt(curl_handle, CURLOPT_CLOSESOCKETFUNCTION, close_callback);
        curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "");

        if (strcasestr(url, "\\post")) {
            url = findChr(url, '\\');
            url = findChr(url, '?');
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, &url[strlen(url)+1]);
        }
        else {
            url = findChr(url, '\\');
            curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
        }

        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        if ((res=curl_easy_perform(curl_handle)) != 0) {   /*error!*/
            Debug(url); Debug(curl_easy_strerror((CURLcode)res));
            return emptyblock;
        }

        if(CURLE_OK != curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct) || !ct)
            return emptyblock;
    }
    else
        return emptyblock;

	b.data = chunk.memory;
	b.size = chunk.size;
    strcpy(b.type, findChr(ct, ';'));

    if (hfile)
        save(&b, hfile);
	return b;
}

// -----------------------------------------------------------
// DEBUG METHODES
// -----------------------------------------------------------

void Debug(const char *msg)
{
    #ifdef DEBUG_LEVEL
        FILE *f = fopen ("debug.txt", "a");
        fputs (msg ,f);
        fputs ("\r\n" ,f);
        fclose(f);
    #endif
}

void DebugInt(u32 msg)
{
    #ifdef DEBUG_LEVEL
        FILE *f = fopen ("debug.txt", "a");
        fprintf (f, "%d", msg);
        fputs ("\r\n" ,f);
        fclose(f);
    #endif
}

void save(struct block *b, FILE *hfile)
{
    FILE *f = hfile;
    if (!f)
        f = fopen ("page.wbr", "wb");
    fwrite (b->data, b->size, 1, f);
    fclose(f);
}

// -----------------------------------------------------------
// EXTENSIONS METHODES
// -----------------------------------------------------------

char *findChr (const char *str, char chr) {
    char *c=strrchr(str, chr);
    if (c!=NULL)
        *c='\0';
    return (char*)str;
}

// -----------------------------------------------------------
// THE END
// -----------------------------------------------------------
