#ifndef __NETWORKOP_H__
#define __NETWORKOP_H__

#include <unistd.h>
#include <network.h>

#include "common.h"
#include "menu.h"

#define GUITH_STACK 	(16384)
#define MAXD            5

typedef struct data
{
    char *url;
    int *bar;
    FILE *file;
    int code;
} Private;

void *NetworkThread (void *arg);
Private *AddHandle(CURLM *cm, char *url, FILE *file);
void StopNetwork();

extern GuiWindow *mainWindow;
extern GuiDownloadManager *manager;
extern GuiText *downloads[5];

extern lwp_t networkthread;
extern u8 networkstack[GUITH_STACK];

#endif
