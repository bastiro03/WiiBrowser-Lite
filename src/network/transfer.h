#ifndef __TRANSFER_H__
#define __TRANSFER_H__

#include <unistd.h>
#include <network.h>

#include "common.h"
#include "menu.h"

#define GUITH_STACK (16384)
#define MAXD 5

using file = struct file
{
	FILE *file;
	char name[512];
};

using Private = struct data
{
	char *url;
	int *bar;
	file save;
	int code;
	double bytes;
	bool keep;
};

void *DownloadThread(void *arg);
void StopDownload();

Private *AddUpdate(CURLM *cm, char *url, FILE *file);
Private *AddDownload(CURLM *cm, char *url, file *file);

extern GuiWindow *mainWindow;
extern GuiDownloadManager *manager;
extern GuiText *downloads[5];

extern lwp_t downloadthread;
extern u8 downloadstack[GUITH_STACK];

#endif
