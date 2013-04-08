#ifndef _FILEOP_H_
#define _FILEOP_H_

#include <errno.h>
#include <fcntl.h>

#include "liste.h"
#include "main.h"
#include "menu.h"

FILE *SelectFile(GuiWindow *mainWindow, char *type);
bool SelectPath(GuiWindow *mainWindow, char *path);
bool GuiBrowser(GuiWindow *mainWindow, GuiWindow *parentWindow, char *path);

bool isValidPath(char *path);
int makedir (char *newdir);

#endif
