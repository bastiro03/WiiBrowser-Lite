#ifndef __NETWORKOP_H__
#define __NETWORKOP_H__

#include <unistd.h>
#include <network.h>

#include "common.h"
#include "menu.h"

#define GUITH_STACK 	(16384)
#define MAXD            5

typedef struct file
{
    FILE *file;
    char name[512];
} file;

typedef struct data
{
    char *url;
    int *bar;
    file save;
    int code;
    bool keep;
} Private;

void *NetworkThread (void *arg);
void StopNetwork();

Private *AddUpdate(CURLM *cm, char *url, FILE *file);
Private *AddDownload(CURLM *cm, char *url, file *file);

extern GuiWindow *mainWindow;
extern GuiDownloadManager *manager;
extern GuiText *downloads[5];

extern lwp_t networkthread;
extern u8 networkstack[GUITH_STACK];

#endif
