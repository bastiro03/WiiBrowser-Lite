#ifndef _FILEOP_H_
#define _FILEOP_H_

#include <errno.h>
#include <fcntl.h>

#include "main.h"
#include "menu.h"

#include "liste.h"
#include "stringop.h"

#define ArchiveFiles ".7z,.zip,.rar"

bool GuiBrowser(GuiWindow *mainWindow, GuiWindow *parentWindow, char *path, size_t capacity, const char *label);
bool AutoDownloader(char *path, size_t capacity);
bool UnzipArchive(char *origfile);
bool CreateSubfolder(const char *fullpath);

bool isValidPath(char *path);
bool CheckFile(const char *filepath);
int makedir (char *newdir);
void show_mem();

#endif
