#ifndef _HTML_H_
#define _HTML_H_

#include <errno.h>
#include <fcntl.h>

#include "handle.h"
#include "main.h"
#include "menu.h"

void DrawScroll (GuiWindow *mainWindow, GuiButton **btndown, GuiButton **btnup, GuiSound *btnSoundOver, GuiTrigger *trigA);
void Clear(GuiWindow *mainWindow, Indice Index, Indice *first, Indice *last, Indice ext);
void FreeMem(GuiWindow *mainWindow, ListaDiTesto text, ListaDiBottoni btn, ListaDiImg img, Indice Index);
void SetFont(GuiText *text, vector<string> mode);

FILE *SelectFile(GuiWindow *mainWindow, char *type);
bool isValidPath(char *path);
int makedir (char *newdir);

enum
{
    UNKNOWN = 0,
    WEB,
    TEXT,
    IMAGE,
    VIDEO
};

FILE *SelectFile(GuiWindow *mainWindow, char *type)
{
    const char *c;
    char path[MAXLEN];
    snprintf(path, MAXLEN, Settings.UserFolder);
    OnScreenKeyboard(mainWindow, path, MAXLEN);
    if ((c = mime2ext(type)))
        strcat(path, c);
    FILE *file = NULL;
    if (isValidPath(path))
        file = fopen(path, "wb");
    return file;
}

bool isValidPath(char *name)
{
    char *l, *path;
    l = strrchr(name, '/');
    if (strlen(name) > 0) {
        if (l == NULL) return true;
        path = strndup(name, l+1-name);
        if(!strchr("./", *(l+1))) {
            makedir(path); // attempt to make dir
            return true;
        }
    }
    return false;
}

static int mymkdir(const char* dirname)
{
    int ret=0;
    ret = mkdir (dirname,0775);
    return ret;
}

int makedir (char *newdir)
{
    char *buffer ;
    char *p;
    int  len = (int)strlen(newdir);

    if (len <= 0)
        return 0;

    buffer = (char*)malloc(len+1);
    strcpy(buffer,newdir);

    if (buffer[len-1] == '/') {
        buffer[len-1] = '\0';
    }
    if (mymkdir(buffer) == 0)
    {
        free(buffer);
        return 1;
    }

    p = buffer+1;
    while (1)
    {
        char hold;

        while(*p && *p != '\\' && *p != '/')
            p++;
        hold = *p;
        *p = 0;
        if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
            free(buffer);
            return 0;
        }
        if (hold == 0)
            break;
        *p++ = hold;
    }
    free(buffer);
    return 1;
}

#endif
