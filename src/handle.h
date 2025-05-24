#include <unistd.h>
#include "video.h"
#include "fileop.h"

#define MAXLEN 512

extern "C"
{
#include "urlcode.h"
}

int HandleForm(GuiWindow *parentWindow, GuiWindow *mainWindow, ListaDiBottoni btn);
int HandleMeta(Lista::iterator lista, string *link, struct block *html);

void HandleImgPad(GuiButton *btnup, GuiButton *btndown, GuiImage *image);
void HandleHtmlPad(int *offset, GuiButton *btnup, GuiButton *btndown, Indice ext, Indice Index, GuiWindow *mainWindow,
                   GuiWindow *parentWindow);
void HandleMenuBar(string *link, char *url, char *orig, char *title, int *choice, int img, GuiWindow *mainWindow,
                   GuiWindow *parentWindow);

void showBar(GuiWindow *mainWindow, GuiWindow *parentWindow, char *url);
void hideBar(GuiWindow *mainWindow, GuiWindow *parentWindow);

extern GuiWindow *statusBar;
extern bool hidden;

int getTime(string);
string getUrl(int *, string);
