#ifndef _STRINGOP_H_
#define _STRINGOP_H_

#include "common.h"

extern "C" {
#include "entities.h"
}

typedef struct Size
{
    int width;
    int height;
} Size;

void downloadPath(struct block *html, char *url, char *path);
void correctPath(char *path, char *arg, int which);
int strtokcmp(const char *string, const char *compare, const char *separator);

struct Size imageSize(Tag *lista, GuiImage *img);

enum
{
    PAGE,
    IMAGES,
    SCREENSHOT,
    OTHER
};

#endif
