#include <stdio.h>
#include <urlcode.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode_lite(char *str) {
  if(!str) return NULL;
  char *buf = (char*)malloc(strlen(str) * 3 + 1);
  if(!buf) return NULL;
  char *pstr = str, *pbuf = buf;
  while (*pstr) {
    if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = *pstr;
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

char *url_encode(char *str) {
  if(!str) return NULL;
  char *buf = (char*)malloc(strlen(str) * 3 + 1);
  if(!buf) return NULL;
  char *pstr = str, *pbuf = buf;
  while (*pstr) {
    if (isalnum((unsigned char)*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
 *pbuf = '\0';
  return buf;
}

void url_escape_string(char *outbuf, const char *inbuf)
{
	while (*inbuf) {
		if (isalnum((unsigned char)*inbuf) || *inbuf == '-' || *inbuf == '_' || *inbuf == '.' || *inbuf == '~')
			*outbuf++ = *inbuf;
		else {
			*outbuf++ = '%';
			*outbuf++ = to_hex(*inbuf >> 4);
			*outbuf++ = to_hex(*inbuf & 15);
		}
		inbuf++;
	}
	*outbuf = '\0';
}

void url_unescape_string(char *outbuf, const char *inbuf)
{
	unsigned char c, c1, c2;
	int i, len = strlen(inbuf);
	for (i = 0; i < len; i++) {
		c = inbuf[i];
		if (c == '%' && i < len - 2) { //must have 2 more chars
			c1 = toupper(inbuf[i + 1]); // we need uppercase characters
			c2 = toupper(inbuf[i + 2]);
			if (((c1 >= '0' && c1 <= '9') || (c1 >= 'A' && c1 <= 'F')) &&
				((c2 >= '0' && c2 <= '9') || (c2 >= 'A' && c2 <= 'F')) ) {
				if (c1 >= '0' && c1 <= '9') c1 -= '0';
				else c1 -= 'A' - 10;
				if (c2 >= '0' && c2 <= '9') c2 -= '0';
				else c2 -= 'A' - 10;
				*outbuf++ = (c1 << 4) | c2;
				i += 2;
			} else {
				*outbuf++ = c;
			}
		} else {
			*outbuf++ = c;
		}
	}
	*outbuf = '\0';
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
  if(!str) return NULL;
  char *buf = (char*)malloc(strlen(str) + 1);
  if(!buf) return NULL;
  char *pstr = str, *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2] && isxdigit((unsigned char)pstr[1]) && isxdigit((unsigned char)pstr[2])) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      } else {
        // Invalid hex — keep literal '%'
        *pbuf++ = *pstr;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}
