#ifndef _URL_H_
#define _URL_H_

/* URL escaping/unescaping routines (from mplayer stream/url.h) */
void url_escape_string(char *outbuf, const char *inbuf);
void url_unescape_string(char *outbuf, const char *inbuf);

#endif