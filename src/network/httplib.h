/*
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
*/

#ifndef _HTTPLIB_H_
#define _HTTPLIB_H_

#include <network.h>
#include <string.h>
#include <curl/curl.h>
#include <string>

// -----------------------------------------------------------
// DEFINES
// -----------------------------------------------------------

#define MAX_LEN         256
#define TYPE            50
#define APPNAME         "WiiBrowser"

struct block
{
    char *data;
    int size;
    char type[TYPE];
};

extern const struct block emptyblock;
struct block downloadfile(CURL *curl_handle, const char *url, FILE *hfile,
                         bool (*cancelled)(void *) = NULL, void *context = NULL);
block downloadpost(CURL *handle, const char *url, const char *body);
void ConfigureHttp(CURL *handle, const char *url);
size_t WriteHttpFile(void *bytes, size_t size, size_t count, void *stream);
void SetNavigationPost(const std::string &url, const std::string &body);
bool TakeNavigationPost(const char *url, std::string &body);
bool postcomment(CURL *curl_handle, char *name, char *content);

bool save(struct block *b, FILE *hfile); // Takes ownership of a non-null stream.
bool validProxy();

// NOTE: these truncate the (mutable) input in place at the found char.
// Callers must pass a writable buffer (e.g. local copy), never a
// string literal or other const storage.
char *findChr (char *str, char chr);
char *findRchr (char *str, char chr);

void DebugInt(u32 msg);
void Debug(const char *msg);

#endif
