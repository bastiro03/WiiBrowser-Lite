/*
**  Created by gave92
**
**  Copyright (C) 2012 - 2013
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

#include "config.h"
#include "httplib.h"

#include "main.h"
#include "menu.h"

extern "C" {
#include "urlcode.h"
}

// -----------------------------------------------------------
// VARIABLES
// -----------------------------------------------------------

enum
{
    HEAD,
    POST,
    GET,
    REQUESTS
};

enum
{
    DFOUND = -1,
    DCOMPLETE = -2,
    DSTOPPED = -3,
};

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
static int firstRun = true;

// -----------------------------------------------------------
// DATA HANDLING
// -----------------------------------------------------------

struct HeaderStruct {
    char *memory;
    u32 size;
    bool download;
    char filename[256];
};

struct MemoryStruct {
    char *memory;
    u32 size;
};

char *findChr (const char *str, char chr);
bool mustdownload(char *content);

int parseline(HeaderStruct *mem, size_t realsize);
void fillstruct(CURL *handle, HeaderStruct *head, struct block *dest);

int close_callback (void *clientp, curl_socket_t item) {
	return net_close(item);
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        Debug("not enough memory (realloc returned NULL)\n");
        exit(EXIT_FAILURE);
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->memory[mem->size+=realsize] = 0;
    return realsize;
}

static size_t writedata(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

static size_t parseheader(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct HeaderStruct *mem = (struct HeaderStruct *)userp;
    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        Debug("not enough memory (realloc returned NULL)\n");
        exit(EXIT_FAILURE);
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->memory[mem->size+=realsize] = 0;
    return parseline(mem, realsize);
}

int parseline(HeaderStruct *mem, size_t realsize)
{
    unsigned int i;
    for(i = mem->size - realsize; i <= mem->size; i++)
        mem->memory[i] = tolower(mem->memory[i]);

    char *line = &mem->memory[mem->size - realsize];
    char buff[50];
    bzero(buff, sizeof(buff));

    if(!strncmp(line, "content-type", 12))
    {
        sscanf(line, "content-type: %s", buff);
        findChr(buff, ';');

        if(mustdownload(buff))
            mem->download = 1;
    }

    if(!strncmp(line, "content-disposition", 19))
    {
        strcpy(mem->filename, line);
    }

    else if(!strncmp(line, "\r\n", 2))
        return (!mem->download) * realsize;
    return realsize;
}

// -----------------------------------------------------------
// SET HEADERS
// -----------------------------------------------------------

void setmainheaders(CURL *curl_handle, const char *url)
{
    if(!curl_handle)
        return;

    /* send all data to this function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* set proxy if specified */
    if(validProxy())
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, Settings.Proxy);

    /* follow redirects */
    curl_easy_setopt(curl_handle, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);

    /* some servers don't like requests that are made without a user-agent
    field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, Agents[Settings.UserAgent]);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);

    /* proper function to close sockets */
    curl_easy_setopt(curl_handle, CURLOPT_CLOSESOCKETFUNCTION, close_callback);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
}

int showprogress(void *bar,
                    double total, /* dltotal */
                    double done, /* dlnow */
                    double ultotal,
                    double ulnow)
{
    char msg[20];
    sprintf(msg, "Loading...%2.2f%%", done*100.0/total);
    SetMessage(msg);

    if(CancelDownload())
        return 1;
    return 0;
}

void setrequestheaders(CURL *curl_handle, int request)
{
    if(!curl_handle)
        return;

    if(request == HEAD)
    {
        /* get header only */
        curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1);
        curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);
    }

    else if(request == GET)
    {
        /* reset handle to perform get */
        curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, showprogress);
    }

    else if(request == POST)
    {

    }
}

// -----------------------------------------------------------
// REQUEST FUNCTIONS
// -----------------------------------------------------------

struct block postrequest(CURL *curl_handle, const char *url, curl_httppost *data)
{
    char *ct = NULL;
    char *post = findChr(url, '?');
    struct block b, h;
    int res;

    struct HeaderStruct head;
    struct MemoryStruct chunk;

    chunk.memory = (char *)malloc(1);   /* will be grown as needed by the realloc above */
    chunk.size = 0; /* no data at this point */

    head.memory = (char *)malloc(1);   /* will be grown as needed by the realloc above */
    head.size = 0; /* no data at this point */
    head.download = 0; /* not yet known at this point */

    setmainheaders(curl_handle, url);
    setrequestheaders(curl_handle, POST);

    if(curl_handle) {
        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, parseheader);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)&head);

        if (data == NULL)
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post+1);
        else curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST, data);

        if ((res = curl_easy_perform(curl_handle)) != 0)      /*error!*/
        {
            if (data)
                curl_formfree(data);

            if (res == CURLE_WRITE_ERROR)
            {
                fillstruct(curl_handle, &head, &h);
                free(head.memory);
                return h;
            }

            Debug(curl_easy_strerror((CURLcode)res));
            return emptyblock;
        }

        if (data)
            curl_formfree(data);

        if(CURLE_OK != curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct) || !ct)
            return emptyblock;
    }
    else
        return emptyblock;

	b.data = chunk.memory;
	b.size = chunk.size;
	free(head.memory);

    findChr(ct, ';');
    strcpy(b.type, ct);
	return b;
}

bool postcomment(CURL *curl_handle, char *name, char *content)
{
    char request[512];
    char *r = request;

    r += sprintf(r, "http://www.htmlcommentbox.com/post?");
    r += sprintf(r, "ajax=true&page=http%%3A%%2F%%2Fwiibrowser.altervista.org%%2Fmainsite%%2Ftracker.php");
    r += sprintf(r, "&refer=http%%3A%%2F%%2Fwiibrowser.altervista.org%%2Fmainsite%%2Ftracker.php%%23HCB_comment_box&opts=16862");
    r += sprintf(r, "&mod=%%241%%24wq1rdBcg%%249ZNKlgbkqbvBdR6hqvyy91&replies_to=&name=%s //%s&content=%s", name, Settings.Uuid, content);

    struct block html = postrequest(curl_handle, request, NULL);
    free(html.data);
    return (html.size > 0);
}

struct block getrequest(CURL *curl_handle, const char *url, FILE *hfile)
{
    char *ct = NULL;
    struct block b, h;
    int res;

    struct HeaderStruct head;
    struct MemoryStruct chunk;

    chunk.memory = (char *)malloc(1);   /* will be grown as needed by the realloc above */
    chunk.size = 0; /* no data at this point */

    head.memory = (char *)malloc(1);   /* will be grown as needed by the realloc above */
    head.size = 0; /* no data at this point */
    head.download = 0; /* not yet known at this point */

    setmainheaders(curl_handle, url);
    setrequestheaders(curl_handle, GET);

    if(curl_handle) {
        /* we pass our 'chunk' struct or 'hfile' to the callback function */
        if(hfile)
        {
            /* send all data to this function */
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writedata);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)hfile);
        }
        else
        {
            curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, parseheader);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)&head);
        }

        if ((res = curl_easy_perform(curl_handle)) != 0)      /*error!*/
        {
            if (res == CURLE_ABORTED_BY_CALLBACK)
            {
                h.size = DSTOPPED;
                return h;
            }

            if (res == CURLE_WRITE_ERROR)
            {
                fillstruct(curl_handle, &head, &h);
                free(head.memory);
                return h;
            }

            Debug(curl_easy_strerror((CURLcode)res));
            return emptyblock;
        }

        if(CURLE_OK != curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct) || !ct)
            return emptyblock;
    }
    else
        return emptyblock;

	b.data = chunk.memory;
	b.size = chunk.size;

    findChr(ct, ';');
    strcpy(b.type, ct);
    free(head.memory);

    if(hfile)
    {
        h.size = DCOMPLETE;
        fclose(hfile);
        return h;
    }
	return b;
}

struct curl_httppost *multipartform(const char *url)
{
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    char *temp = strrchr(url, '?');
    char *begin = url_decode(temp);
    char *mid = NULL;

    char *name, *value;
    char *file, *path;

    /* Fill in the filename field */
    do
    {
        mid = strchr(begin, '=');
        begin++;
        name = strndup(begin, mid-begin);

        begin = strchr(begin, '&');
        mid++;
        value = strndup(mid, begin-mid);

        if(strstr(value, "_UPLOAD"))
        {
            path = strrchr(value, '_');
            file = strndup(value, path-value);

            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, name,
                CURLFORM_FILE, file,
                CURLFORM_END);

            free(file);
        }
        else
        {
            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, name,
                CURLFORM_COPYCONTENTS, value,
                CURLFORM_END);
        }

        free(name);
        free(value);
    }
    while ((begin = strchr(begin, '&')));

    free(begin);
    return formpost;
}

struct block downloadfile(CURL *curl_handle, const char *url, FILE *hfile)
{
    const char *mode = strrchr(url, '\\');
    findChr(url, '\\');

    if (firstRun)
        firstRun = false;
    else curl_easy_reset(curl_handle);

    if (!mode)
        return getrequest(curl_handle, url, hfile);

    if (strcasestr(mode + 1, "post"))
        return postrequest(curl_handle, url, NULL);

    else if (strcasestr(mode + 1, "multipart"))
    {
        curl_httppost *data = multipartform(url);
        return postrequest(curl_handle, url, data);
    }

    return getrequest(curl_handle, url, hfile);
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

bool mustdownload(char content[])
{
    if (strstr(content, "text/html") || strstr(content, "application/xhtml")
            || strstr(content, "text") || strstr(content, "image")
#ifdef MPLAYER
                || strstr(content, "video")
#endif
        )
        return false;
    return true;
}

char *findChr (const char *str, char chr) {
    char *c=strrchr(str, chr);
    if (c!=NULL)
        *c='\0';
    return (char*)c;
}

bool validProxy()
{
    if(strlen(Settings.Proxy))
        return true;

    return false;
}

void trimline(char *init, struct block *dest)
{
    dest->data = (char *)malloc(256);

    if(init[0] == '"')
    {
        findChr(init, '"');
        strcpy(dest->data, init+1);
    }
    else strcpy(dest->data, init);
}

void fillstruct(CURL *handle, HeaderStruct *head, struct block *dest)
{
    char *ct = NULL;
    char *ft;
    dest->size = DFOUND;

    if(CURLE_OK == curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct) && ct)
    {
        findChr(ct, ';');
        strcpy(dest->type, ct);
    }

    if(strstr(head->filename, "filename"))
    {
        if((ft = findChr(head->filename, '=')))
            trimline(ft+1, dest);
    }
    else dest->data = NULL;
}
