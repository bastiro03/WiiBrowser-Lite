#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "stringop.h"

void addname(struct block *html, char *url, char *path, const char *phold)
{
	char *c = nullptr;
	if (url)
		c = strrchr(url, '/');

	/* content-disposition */
	if (html->data && strrchr(html->data, '.'))
		strcat(path, html->data);

	/* find in url */
	else if (c && strchr(c, '.'))
		strcat(path, c + 1);

	/* content-type */
	else if ((c = (char *)mime2ext(html->type)))
	{
		strcat(path, phold);
		strcat(path, c);
	}
}

void addformat(struct block *html, char *url, char *path)
{
	char *c = nullptr;
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

	if (c[1] == '\0')
		addname(html, url, path, "filename");
	else if (strchr(c, '.') == nullptr)
		addformat(html, url, path);
}

void correctPath(char *path, char *arg, int which)
{
	char *c = strrchr(path, '/');
	struct block html;
	char phold[20];

	html.data = NULL;
	if (arg)
		strcpy(html.type, arg);

	switch (which)
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

	if (c[1] == '\0')
		addname(&html, nullptr, path, phold);
	else if (strchr(c, '.') == nullptr)
		addformat(&html, nullptr, path);
}

int strtokcmp(const char *string, const char *compare, const char *separator)
{
	if (!string || !compare)
		return -1;

	char TokCopy[512];
	strncpy(TokCopy, compare, sizeof(TokCopy));
	TokCopy[511] = '\0';

	char *strTok = strtok(TokCopy, separator);

	while (strTok != nullptr)
	{
		if (strcasecmp(string, strTok) == 0)
		{
			return 0;
		}
		strTok = strtok(nullptr, separator);
	}

	return -1;
}

struct Size imageSize(Tag *lista, GuiImage *img)
{
	int attrWidth = INT_MAX, attrHeight = INT_MAX;
	int listWidth, listHeight;

	if (img && img->GetImage())
	{
		attrWidth = img->GetWidth();
		attrHeight = img->GetHeight();

		if (lista->value[0].text.length())
		{
			listWidth = atoi(lista->value[0].text.c_str());

			if (!strchr(lista->value[0].text.c_str(), '%'))
				attrWidth = listWidth;
			else
				attrWidth = listWidth * attrWidth / 100;
		}

		if (lista->value[1].text.length())
		{
			listHeight = atoi(lista->value[1].text.c_str());

			if (!strchr(lista->value[1].text.c_str(), '%'))
				attrHeight = listHeight;
			else
				attrHeight = listHeight * attrHeight / 100;
		}
	}

	else
	{
		if (lista->value[0].text.length())
		{
			listWidth = atoi(lista->value[0].text.c_str());

			if (!strchr(lista->value[0].text.c_str(), '%'))
				attrWidth = listWidth;
		}

		if (lista->value[1].text.length())
		{
			listHeight = atoi(lista->value[1].text.c_str());

			if (!strchr(lista->value[1].text.c_str(), '%'))
				attrHeight = listHeight;
		}
	}

	return {attrWidth, attrHeight};
}
