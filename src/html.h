#ifndef _HTML_H_
#define _HTML_H_

#include "handle.h"
#include "fileop.h"

void DrawScroll(GuiWindow *mainWindow, GuiButton **btndown, GuiButton **btnup, GuiSound *btnSoundOver,
				GuiTrigger *trigA);
void Clear(GuiWindow *mainWindow, Indice Index, Indice *first, Indice *last, Indice ext);
void FreeMem(GuiWindow *mainWindow, ListaDiTesto text, ListaDiBottoni btn, ListaDiImg img, Indice Index);
void SetFont(GuiText *text, vector<string> mode);

enum
{
	THREAD_EXIT = 0,
	THREAD_SUSPEND,
	THREAD_RUN
};

enum
{
	UNKNOWN = 0,
	WEB,
	TEXT,
	IMAGE,
	VIDEO
};

#endif
