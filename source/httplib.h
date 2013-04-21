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

// -----------------------------------------------------------
// DEFINES
// -----------------------------------------------------------

#define MAX_LEN         256
#define TYPE            50
#define DEBUG_LEVEL
#define APPNAME         "WiiBrowser"

struct block
{
    char *data;
    int size;
    char type[TYPE];
};

extern const struct block emptyblock;
struct block downloadfile(CURL *curl_handle, const char *url, FILE *hfile);
void save(struct block *b, FILE *hfile);

void DebugInt(u32 msg);
void Debug(const char *msg);

#endif
