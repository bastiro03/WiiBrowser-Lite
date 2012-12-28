/*
**  Created by wplaat
**
**  Copyright (C) 2008
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
#include <ctype.h>
#include <gccore.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <fat.h>
#include <ogc/lwp_watchdog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>

#include "httplib.h"
#include "entities.h"

// -----------------------------------------------------------
// VARIABLES
// -----------------------------------------------------------

char res[MAX_LEN];
char report[MAX_LEN];
bool deter = TRUE;

char *http_host;
u16 http_port;
char *http_path;
u32 http_max_size;

http_res result;
http_res retval;
u32 content_length;
u8 *http_data;

u32 http_status;
u32 outlen;
u8  *outbuf;

const struct block emptyblock = {NULL, 0};

// -----------------------------------------------------------
// FUNCTIONS
// -----------------------------------------------------------

void debugInt(u32 msg);
void save(struct block *b, FILE *hfile);

char * adjustUrl(const char* link, char* url);
char * getHost(char *url);
char * findChr (const char *str, char chr);

#ifdef LIBCURL
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
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
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
    chunk.memory = malloc(1);   /* will be grown as needed by the realloc above */
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
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
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
            Debug(url); Debug(curl_easy_strerror(res));
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
#else

struct block downloadfile(const char *url)
{
    char type[TYPE];
	u8 *outbuf;
	u32 outlen, http_status;
	Debug("request");

	memset(report, 0, sizeof(report));
	retval = http_request(url, 1 << 31, type);

	while(retval == HTTPR_ERR_MOVED)
	{
		Debug("redirect");
		strcpy(report, adjustUrl(report, url));
		strcpy(url, report);
		retval = http_request(report, 1 << 31, type);
    }
    if (retval != HTTPR_OK) {
    	Debug("FAILED");
		return emptyblock;
	}

	http_get_result(&http_status, &outbuf, &outlen);

	struct block b;
	b.data = outbuf;
	b.size = outlen;

    strcpy(b.type, findChr(type, ';'));  Debug(b.type);
    if (!strstr(b.type, "text") && !strstr(b.type, "image"))
        save(b.data, b.size, b.type);
	return b;
}

// -----------------------------------------------------------
// URL METHODES
// -----------------------------------------------------------

char * getHost(char *url) {
    const char *p=url+7;
    char *c=strchr (p, '/');
    return strndup(url,(c+1)-url);
}

char * adjustUrl(const char* link, char* url) {
    if (link[0]=='/') url=getHost(url);
    url[strlen(url)-1]='\0';
    strcpy (res, url);
    if (strstr(link, "http://")!=link && strstr(link, "https://")!=link) {
        if (link[0]!='/')
            strcat(res, "/");
        else if (link[1]=='/')
            strcpy(res, "http:");
        strcat(res, link);
        return res;
    }
    return link;
}

// -----------------------------------------------------------
// TCP METHODES
// -----------------------------------------------------------

s32 tcp_socket(void)
{
        Debug( "tcp_socket:enter\n");
        s32 s;

        s = net_socket (AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (s < 0)
		{
			Debug ( "net_socket failed\n");
        }

	    Debug ( "tcp_socket: leave\n");
        return s;
}

s32 tcp_connect(char *host, const u16 port)
{
	    Debug ( "tcp_connect:enter\n");

        struct hostent *hp;
        struct sockaddr_in sa;
        s32 s, res;
        s64 t;

        hp = net_gethostbyname(host);
        if (!hp || !(hp->h_addrtype == PF_INET))
		{
			Debug ("net_gethostbyname failed\n");
            return errno;
        }

        s = tcp_socket();
        if (s < 0)
		{
		   Debug ("Error creating socket, exiting");
           return s;
	    }

        memset (&sa, 0, sizeof (struct sockaddr_in));
        //sa.sin_family= PF_INET;
		sa.sin_family= AF_INET;
        sa.sin_len = sizeof (struct sockaddr_in);
        sa.sin_port= htons(port);
        memcpy ((char *) &sa.sin_addr, hp->h_addr_list[0], hp->h_length);

        Debug ("DNS resolve\n");

        t = gettime();
        while (true)
		{
                if (ticks_to_millisecs (diff_ticks (t, gettime ())) > TCP_CONNECT_TIMEOUT)
				{
					Debug("tcp_connect timeout\n");
                    net_close (s);
                    return -ETIMEDOUT;
                }

                res = net_connect (s, (struct sockaddr *) &sa, sizeof (struct sockaddr_in));

                if (res < 0)
				{
                   if (res == -EISCONN)  break;

                   if (res == -EINPROGRESS || res == -EALREADY)
				   {
                       usleep (20 * 1000);
                       continue;
                   }

   				   Debug ( "net_connect failed\n");
                   net_close(s);
                   return res;
                }
                break;
        }
		Debug ( "tcp_connect:leave\n");
        return s;
}

char * tcp_readln(const s32 s, const u16 max_length, const u64 start_time, const u16 timeout)
{
        char *buf;
        u16 c;
        s32 res;
        char *ret;

        buf = (char *) malloc (max_length);

        c = 0;
        ret = NULL;
        while (true)
		{
                if (ticks_to_millisecs (diff_ticks (start_time, gettime ())) > timeout)
                        break;

                res = net_read (s, &buf[c], 1);

                if ((res == 0) || (res == -EAGAIN))
				{
                        usleep (20 * 1000);
                        continue;
                }

                if (res < 0)
				{
						Debug("tcp_readln failed\n");
                        break;
                }

                if ((c > 0) && (buf[c - 1] == '\r') && (buf[c] == '\n'))
				{
                        if (c == 1)
						{
                                ret = strdup ("");
                                break;
                        }
                        ret = strndup (buf, c - 1);
                        break;
                }
                c++;
                if (c == max_length) break;
        }
        free (buf);
		return ret;
}

bool tcp_read(const s32 s, u8 **buffer, const u32 length)
{
	    Debug("tcp_read:enter\n");

        u8 *p;
        u32 step, left, block, received;
        s64 t;
        s32 res;

        step = 0;
        p = *buffer;
        left = length;
        received = 0;
	    t = gettime ();
        Debug("while");
        while (left)
		{
                if (ticks_to_millisecs (diff_ticks (t, gettime ())) > TCP_BLOCK_RECV_TIMEOUT)
			    {
				   Debug("tcp_read timeout\n");
                   break;
                }

                block = left;
                debugInt(block);
                if (block > 2048) block = 2048;
                Debug("net read enter");
                res = net_read (s, p, block);
                Debug("net read done");
                if ((res == 0) || (res == -EAGAIN))
				{
                    usleep (20 * 1000);
                    continue;
                }

                if (res < 0)
				{
					Debug("net_read failed\n");
                    break;
                }

                received += res;
                left -= res;
                p += res;

                if ((received / TCP_BLOCK_SIZE) > step)
				{
                    t = gettime ();
                    step++;
                }
        }

		Debug("tcp_read:leave\n");
        return left == 0;
}

bool tcp_write(const s32 s, const u8 *buffer, const u32 length)
{
    Debug("tcp_write: enter\n");

        const u8 *p;
        u32 step, left, block, sent;
        s64 t;
        s32 res;

        step = 0;
        p = buffer;
        left = length;
        sent = 0;

        t = gettime ();
        while (left)
		{
                if (ticks_to_millisecs (diff_ticks (t, gettime ())) > TCP_BLOCK_SEND_TIMEOUT)
				{
 					Debug("tcp_write: timeout\n");
                    break;
                }

                block = left;
                if (block > 2048) block = 2048;

                res = net_write (s, p, block);
                if ((res == 0) || (res == -56))
				{
                    usleep (20 * 1000);
                    continue;
                }

                if (res < 0)
				{
					Debug("tcp_write: failed");
                    break;
                }
                sent += res;
                left -= res;
                p += res;

                if ((sent / TCP_BLOCK_SIZE) > step)
				{
                    t = gettime ();
                    step++;
                }
        }
		Debug("tcp_write: leave\n");
        return left == 0;
}

// -----------------------------------------------------------
// HTTP METHODES
// -----------------------------------------------------------

bool http_split_url(char **host, char **path, const char *url)
{
		Debug("http_split_url: enter\n");

        const char *p;
        char *c;

        if (strncasecmp (url, "http://", 7))
		{
		   Debug("http_split_url: leave [false]\n");
           return false;
		}

        p = url + 7;
        c = strchr (p, '/');

        if (c)
        {
            if (c[0] == 0)
            {
                Debug("http_split_url: leave [false]\n");
                return false;
            }

            *host = strndup (p, c - p);
            Debug(*host);
            *path = strdup (c);

            Debug("http_split_url: leave [true]\n");
            return true;
        }
        return false;
}

http_res http_request (const char *url, const u32 max_size, char *type)
{
        if(!deter)
        remove ("debug.txt");

        Debug (url);
	    Debug ("http_request: enter\n");
 		Debug("begin request");

        int linecount;
		bool chunked=false;
		int emptycount=0;
		Debug("split");
        if (!http_split_url(&http_host, &http_path, url)) return HTTPR_ERR;

        http_port = 80;
        http_max_size = max_size;

        http_status = 404;
        content_length = 0;
        http_data = NULL;
		Debug("connect");
        int s = tcp_connect(http_host, http_port);

		Debug("tcp_connect");
        if (s < 0)
		{
            result = HTTPR_ERR_CONNECT;
            return result;
        }

        char *request = (char *) malloc(2048);
        char *r = request;
        Debug("request");

		r += sprintf (r,"GET %s HTTP/1.1\r\n", http_path);
        r += sprintf (r,"Host: %s\r\n", http_host);
		r += sprintf (r,"User-Agent: %s/%s (compatible; v3.X; Nintendo Wii)\r\n", APPNAME, APPVERSION);
        r += sprintf (r,"Accept: */*\r\n");     //r += sprintf (r,"Accept: text/xml,text/html,text/plain,image/png,image/jpeg\r\n");
        r += sprintf (r,"Cache-Control: no-cache\r\n\r\n");

		Debug("write");
        bool b = tcp_write (s, (u8 *) request, strlen (request));

        free (request);
        linecount = 0;
        char *line=NULL;
		Debug("FOR ENTER");
        for (linecount=0; linecount < 32; linecount++)
		{
			Debug("read line");
            line = tcp_readln ((s32) s, (u16) 0x1ff, (u64) gettime(), (u16) HTTP_TIMEOUT);
			if (line) Debug(line);

            if (!line)
			{
                http_status = 404;
                result = HTTPR_ERR_REQUEST;
                break;
            }

            if (chunked && (emptycount==1))
			{
			   sscanf (line, "%x", &content_length);
			   Debug("chunksize:");
			   debugInt(content_length);    //if (content_length>10000) { content_length=0; return HTTPR_ERR; }  // Protect against crash frame
			   free (line);
               line = NULL;
               break;
			}

            if (strlen(line)<1)
			{
			    if (chunked)
				{
				    if (++emptycount>1)
					{
					  free (line);
					  line = NULL;
                      break;
					}
					Debug("Empty line\n");
				}
                else
                {
                  free (line);
                  line = NULL;
                  break;
			    }
            }

            char *c=strchr(line, ' ');
            if (c) {
                for (;c!=line-1;c--)
                    *c=tolower(*c);
            }

            sscanf (line, "http/1.%*u %u", &http_status);
            sscanf (line, "content-length: %u", &content_length);
            sscanf (line, "location: %s", report);
            sscanf (line, "content-type: %s", type);

			if (strstr(line, "chunked")!=0)
			{
				Debug("Chunked frame found");
				free (line);
				line = NULL;
				chunked=true;
            }

            free (line);
            line = NULL;
        }

        Debug("FOR EXIT");
		Debug("content_length:");
		debugInt(content_length);

        if ((http_status==301||http_status==302||http_status==303||http_status==307)&&strlen(report)>0)
        {
            Debug("redirecting");
            result = HTTPR_ERR_MOVED;
            return result;
        }

        if (http_status != 200 && http_status != 410)
		{
            result = HTTPR_ERR_STATUS;
            net_close (s);
            return result;
        }

	    if (linecount == 32) http_status = 404;

        if (content_length > http_max_size)
		{
           result = HTTPR_ERR_TOOBIG;
           net_close (s);
           return result;
        }

        http_data = (u8 *) malloc (content_length);
        b = tcp_read (s, &http_data, content_length);

        if (!chunked && !content_length) //No "Content-Length" header!
        {
            Debug("making an effort..");
            u32 p, res, block=2048;
            for (res=block, p=0; res>0; p+=block) {
                http_data = (u8 *) realloc (http_data, p+block);
                res = net_read (s, http_data+p, block);
            }
            if (p-=block>0)
                content_length=p;
        }

        if (chunked==true)
        {
           unsigned int temp=content_length; u8 *tmp;
           Debug("Ciclo");
           for (;temp!=0;content_length+=temp)
           {
                line = tcp_readln ((s32) s, (u16) 0xff, (u64) gettime(), (u16) HTTP_TIMEOUT);
                free (line);
                Debug("CRLF gone.."); //Due to exactly cutting the chunk the CRLF is leftover and so this was a lazy way to get around that, i could just increase the content length by 2

                line = tcp_readln ((s32) s, (u16) 0xff, (u64) gettime(), (u16) HTTP_TIMEOUT);
                Debug(line);
                sscanf (line, "%x", &temp);
                free (line);

                http_data = (u8 *) realloc (http_data,content_length+temp);
                tmp=http_data+content_length;
                b = tcp_read (s,&tmp,temp);
           }
        }

        if (!b)
		{
           free (http_data);
           http_data = NULL;
           result = HTTPR_ERR_RECEIVE;
           net_close (s);
           return result;
        }

        result = HTTPR_OK;
        net_close(s);

		Debug("http_request: leave\n");
        return result;
}

extern bool http_get_result(u32 *_http_status, u8 **content, u32 *length)
{
		Debug("http_get_result: enter\n");

        if (http_status) *_http_status = http_status;

        if (result == HTTPR_OK)
		{
           *content = http_data;
           *length = content_length;
        }
		else
		{
           *content = NULL;
           *length = 0;
        }

        free (http_host);
        free (http_path);

		Debug("http_get_result: leave\n");
        return true;
}
#endif /*LIBCURL*/

// -----------------------------------------------------------
// DEBUG METHODES
// -----------------------------------------------------------

void Debug(const char * msg)
{
    #ifdef DEBUG_LEVEL
        FILE *f = fopen ("debug.txt", "a");
        fputs (msg ,f);
        fputs ("\r\n" ,f);
        fclose(f);
    #endif
}

void debugInt(u32 msg)
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
    char page[20]="page";
    const char *c, *ext;
    if (!(c=mime2ext(b->type)))
    {
        ext=strchr(b->type, '/')+1;
        strcat(page, ".");
        strncat(page, ext, 10);
    }
    else
        strncat(page, c, 10);
    FILE *f = hfile;
    if (!f)
        f = fopen (page, "wb");
    fwrite (b->data, b->size, 1, f);
    fclose(f);
}

// -----------------------------------------------------------
// EXTENSIONS METHODES
// -----------------------------------------------------------

char * findChr (const char *str, char chr) {
    char *c=strchr(str, chr);
    if (c!=NULL)
        *c='\0';
    return (char*)str;
}

// -----------------------------------------------------------
// THE END
// -----------------------------------------------------------
