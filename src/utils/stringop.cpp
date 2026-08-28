#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "stringop.h"

static void safe_strcat(char *dst, const char *src, size_t dstSize) {
    size_t dlen = strlen(dst);
    size_t slen = src ? strlen(src) : 0;
    if(dlen >= dstSize) return;
    size_t avail = dstSize - dlen - 1;
    if(slen > avail) slen = avail;
    if(slen) {
        memcpy(dst + dlen, src, slen);
        dst[dlen + slen] = '\0';
    }
}

void addname(struct block *html, char *url, char *path, const char *phold)
{
    char *c = NULL;
    if (url)
        c = strrchr(url, '/');

    /* content-disposition */
    if (html->data && strrchr(html->data, '.'))
        safe_strcat(path, html->data, 512);

    /* find in url */
    else if (c && strchr(c, '.'))
        safe_strcat(path, c+1, 512);

    /* content-type */
    else if ((c = (char *)mime2ext(html->type)))
    {
        safe_strcat(path, phold, 512);
        safe_strcat(path, c, 512);
    }
}

void addformat(struct block *html, char *url, char *path)
{
    char *c = NULL;
    if (url)
        c = strrchr(url, '/');
    char *p;

    /* content-disposition */
    if (html->data && (p = strrchr(html->data, '.')))
        safe_strcat(path, p, 512);

    /* find in url */
    else if (c && (c = strchr(c, '.')))
        safe_strcat(path, c, 512);

    /* content-type */
    else if ((c = (char *)mime2ext(html->type)))
        safe_strcat(path, c, 512);
}

void downloadPath(struct block *html, char *url, char *path)
{
    char *c = strrchr(path, '/');
    if(!c) return;

    if(c[1] == '\0')
        addname(html, url, path, "filename");
    else if(strchr(c, '.') == NULL)
        addformat(html, url, path);
}

void correctPath(char *path, char *arg, int which)
{
    char *c = strrchr(path, '/');
    if(!c) return;
    struct block html;
    char phold[20];

    html.data = NULL;
    if(arg) {
        strncpy(html.type, arg, sizeof(html.type)-1);
        html.type[sizeof(html.type)-1] = '\0';
    } else {
        html.type[0] = '\0';
    }

    switch(which)
    {
    case PAGE:
        strcpy(phold, "page");
        break;
    case IMAGES:
        strcpy(phold, "image");
        break;
    case SCREENSHOT:
        strcpy(phold, "screenshot");
        break;
    case OTHER:
        strcpy(phold, "filename");
        break;
    }

    if(c[1] == '\0')
        addname(&html, NULL, path, phold);
    else if(strchr(c, '.') == NULL)
        addformat(&html, NULL, path);
}

int strtokcmp(const char *string, const char *compare, const char *separator)
{
	if(!string || !compare)
		return -1;

	char TokCopy[512];
	strncpy(TokCopy, compare, sizeof(TokCopy));
	TokCopy[511] = '\0';

	char * strTok = strtok(TokCopy, separator);

	while (strTok != NULL)
	{
		if (strcasecmp(string, strTok) == 0)
		{
			return 0;
		}
		strTok = strtok(NULL,separator);
	}

	return -1;
}

struct Size imageSize(Tag *lista, GuiImage *img)
{
    int attrWidth = INT_MAX, attrHeight = INT_MAX;
    int listWidth, listHeight;

    if(img && img->GetImage())
    {
        attrWidth = img->GetWidth();
        attrHeight = img->GetHeight();

        if(lista->value[0].text.length())
        {
            listWidth = atoi(lista->value[0].text.c_str());

            if(!strchr(lista->value[0].text.c_str(), '%'))
                attrWidth = listWidth;
            else attrWidth = listWidth*attrWidth/100;
        }

        if(lista->value[1].text.length())
        {
            listHeight = atoi(lista->value[1].text.c_str());

            if(!strchr(lista->value[1].text.c_str(), '%'))
                attrHeight = listHeight;
            else attrHeight = listHeight*attrHeight/100;
        }
    }

    else
    {
        if(lista->value[0].text.length())
        {
            listWidth = atoi(lista->value[0].text.c_str());

            if(!strchr(lista->value[0].text.c_str(), '%'))
                attrWidth = listWidth;
        }

        if(lista->value[1].text.length())
        {
            listHeight = atoi(lista->value[1].text.c_str());

            if(!strchr(lista->value[1].text.c_str(), '%'))
                attrHeight = listHeight;
        }
    }

    return {attrWidth, attrHeight};
}
