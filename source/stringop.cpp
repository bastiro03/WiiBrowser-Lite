#include <stdio.h>
#include <string.h>
#include "stringop.h"

void addname(struct block *html, char *url, char *path, const char *phold)
{
    char *c = NULL;
    if (url)
        c = strrchr(url, '/');

    /* content-disposition */
    if (html->data && strrchr(html->data, '.'))
        strcat(path, html->data);

    /* find in url */
    else if (c && strchr(c, '.'))
        strcat(path, c+1);

    /* content-type */
    else if ((c = (char *)mime2ext(html->type)))
    {
        strcat(path, phold);
        strcat(path, c);
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
        strcat(path, p);

    /* find in url */
    else if (c && (c = strchr(c, '.')))
        strcat(path, c);

    /* content-type */
    else if ((c = (char *)mime2ext(html->type)))
        strcat(path, c);
}

void downloadPath(struct block *html, char *url, char *path)
{
    char *c = strrchr(path, '/');

    if(c[1] == '\0')
        addname(html, url, path, "filename");
    else if(strchr(c, '.') == NULL)
        addformat(html, url, path);
}

void correctPath(char *path, char *arg, int which)
{
    char *c = strrchr(path, '/');
    struct block html;
    char phold[20];

    html.data = NULL;
    if(arg)
        strcpy(html.type, arg);

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
