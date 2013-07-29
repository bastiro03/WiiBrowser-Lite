#ifndef _FILEOP_H_
#define _FILEOP_H_

#include <errno.h>
#include <fcntl.h>

#include "main.h"
#include "menu.h"

#include "liste.h"
#include "stringop.h"

bool GuiBrowser(GuiWindow *mainWindow, GuiWindow *parentWindow, char *path, const char *label);
bool UnzipArchive(char *zipfilepath);

bool isValidPath(char *path);
int makedir (char *newdir);
void show_mem();

#endif
