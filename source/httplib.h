/*
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
*/

#define _HTTP_H_
#include <errno.h>
#include <network.h>
#include <ogcsys.h>
#include <string.h>

#include <stdio.h>
#include <stdarg.h>
#include <gctypes.h>
#include <curl/curl.h>

// -----------------------------------------------------------
// DEFINES
// -----------------------------------------------------------

#define MAX_LEN 256
#define TYPE 20
#define DEBUG_LEVEL
#define LIBCURL

#define TCP_CONNECT_TIMEOUT 5000
#define TCP_BLOCK_SIZE (16 * 1024)
#define TCP_BLOCK_RECV_TIMEOUT 4000
#define TCP_BLOCK_SEND_TIMEOUT 4000
#define HTTP_TIMEOUT 300000

#define APPNAME			"WiiBrowser"
#define APPVERSION		"0.02"

struct block
{
    char *data;
    u32 size;
    char type[TYPE];
};

extern const struct block emptyblock;
struct block downloadfile(CURL *curl_handle, const char *url, FILE *hfile);

s32 tcp_socket (void);
s32 tcp_connect (char *host, const u16 port);

char *tcp_readln (const s32 s, const u16 max_length, const u64 start_time, const u16 timeout);
bool tcp_read (const s32 s, u8 **buffer, const u32 length);
bool tcp_write (const s32 s, const u8 *buffer, const u32 length);

typedef enum {
    HTTPR_OK,
    HTTPR_ERR,
    HTTPR_ERR_CONNECT,
    HTTPR_ERR_REQUEST,
    HTTPR_ERR_STATUS,
    HTTPR_ERR_TOOBIG,
    HTTPR_ERR_RECEIVE,
    HTTPR_ERR_MOVED
} http_res;

http_res http_request (const char *url, const u32 max_size, char *tipo);
bool http_get_result (u32 *http_status, u8 **content, u32 *length);
void Debug(const char * msg);

enum
{
    TCP_INIT=0,
	TCP_REQUEST1a=1,
	TCP_REQUEST1b=2,
	TCP_REQUEST2a=3,
	TCP_REQUEST2b=4,
	TCP_REQUEST3a=5,
	TCP_REQUEST3b=6,
	TCP_ERROR=7,
	TCP_RETRY=8,
	TCP_IDLE=9,
	TCP_END=10
};
