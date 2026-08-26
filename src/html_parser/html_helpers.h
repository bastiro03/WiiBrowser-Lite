#ifndef WIIBROWSER_HTML_HELPERS_H
#define WIIBROWSER_HTML_HELPERS_H

#include <string>
#include <vector>
#include "common.h"
#include "html.h"

class GuiText;
class GuiWindow;
class GuiButton;
class GuiSound;
class GuiTrigger;

// Sprint 5.1: extracted helpers from html.cpp (was 733 lines, DisplayHTML 445 lines)
// These are pure helpers; DisplayHTML remains in html.cpp but now delegates.

int knownType(char type[]);
bool nextItemIs(Lista::iterator list, Lista::iterator end, std::string item);
bool prevItemIs(Lista::iterator list, std::string item);

void Clear(GuiWindow* mainWindow, Indice Index, Indice *first, Indice *last, Indice ext);
void SetFont(GuiText *text, std::vector<std::string> mode);
void DrawScroll(GuiWindow * mainWindow, GuiButton **btndown, GuiButton **btnup, GuiSound *btnSoundOver, GuiTrigger *trigA);
void FreeMem(GuiWindow* mainWindow, ListaDiTesto text, ListaDiBottoni btn, ListaDiImg img, Indice Index);

// Pagination helpers
struct PageState {
    int XPos = 0;
    int YPos = 0;
    int Height = 0;
    int offset = 0;
};

#endif // WIIBROWSER_HTML_HELPERS_H
