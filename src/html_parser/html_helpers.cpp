#include "html_helpers.h"
#include "html.h"
#include "common.h"
#include "context.h"
#include <climits>

int knownType(char type[]) {
    if (!strcmp(type, "text/html") || strstr(type, "application/xhtml"))
        return WEB;
    if (strstr(type, "text"))
        return TEXT;
    if (strstr(type, "image"))
        return IMAGE;
    if (strstr(type, "video"))
        return VIDEO;
    return UNKNOWN;
}

bool nextItemIs(Lista::iterator list, Lista::iterator end, std::string item) {
    bool ret = false;
    if((++list) != end && list->name == item)
        ret = true;
    return ret;
}

bool prevItemIs(Lista::iterator list, std::string item) {
    bool ret = false;
    if((--list)->name == item)
        ret = true;
    return ret;
}

void Clear(GuiWindow* mainWindow, Indice Index, Indice *first, Indice *last, Indice ext) {
    auto ctx = GetAppContext();
    int sh = ctx.screenHeight ? *ctx.screenHeight : 480;
    if (NoIndex(*first)) *first=ext;
    if (NoIndex(*last)) *last=Index;
    if (*first && (*first)->elem->GetYPosition()+(*first)->screenSize <0) {
        (*first)->elem->SetVisible(false);
        *first=(*first)->prec;
    }
    if (*last && (*last)->elem->GetYPosition()> sh+25) {
        (*last)->elem->SetVisible(false);
        *last=(*last)->prox;
    }
}

void SetFont(GuiText *text, std::vector<std::string> mode) {
    if (checkTag(mode, "b") || checkTag(mode, "strong"))
        text->SetFont(font_bold_ttf, font_bold_ttf_size);
    if (checkTag(mode, "em") || checkTag(mode, "i"))
        text->SetFont(font_italic_ttf, font_italic_ttf_size);
}

void DrawScroll(GuiWindow * mainWindow, GuiButton **btndown, GuiButton **btnup, GuiSound *btnSoundOver, GuiTrigger *trigA) {
    GuiImageData *btnupOutline=new GuiImageData(scrollbar_arrowup_png, scrollbar_arrowup_png_size);
    GuiImageData *btnupOutlineOver=new GuiImageData(scrollbar_arrowup_over_png, scrollbar_arrowup_over_png_size);
    GuiImage *btnupImg=new GuiImage(btnupOutline);
    GuiImage *btnupImgOver=new GuiImage(btnupOutlineOver);

    *btnup=new GuiButton(btnupOutline->GetWidth(), btnupOutline->GetHeight());
    (*btnup)->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    (*btnup)->SetPosition(-40, 20);

    (*btnup)->SetImage(btnupImg);
    (*btnup)->SetImageOver(btnupImgOver);
    (*btnup)->SetSoundOver(btnSoundOver);
    (*btnup)->SetTrigger(trigA);
    (*btnup)->SetEffectGrow();

    GuiImageData *btnOutline=new GuiImageData(scrollbar_arrowdown_png, scrollbar_arrowdown_png_size);
    GuiImageData *btnOutlineOver=new GuiImageData(scrollbar_arrowdown_over_png, scrollbar_arrowdown_over_png_size);
    GuiImage *btndownImg=new GuiImage(btnOutline);
    GuiImage *btndownImgOver=new GuiImage(btnOutlineOver);

    *btndown=new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
    (*btndown)->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    (*btndown)->SetPosition(-40, -40);
    (*btndown)->SetImage(btndownImg);
    (*btndown)->SetImageOver(btndownImgOver);
    (*btndown)->SetSoundOver(btnSoundOver);
    (*btndown)->SetTrigger(trigA);
    (*btndown)->SetEffectGrow();

    HaltGui();
    mainWindow->Append(*btnup);
    mainWindow->Append(*btndown);
    ResumeGui();
}

void FreeMem(GuiWindow* mainWindow, ListaDiTesto text, ListaDiBottoni btn, ListaDiImg img, Indice Index) {
    Indice temp, effect=NULL;
    for (temp=Index; !NoIndex(temp); temp=temp->prox) {
        if (temp->elem->IsVisible()) {
            temp->elem->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
            if(temp->content!=null) effect=temp;
        }
    }
    if (effect) while (effect->elem->GetEffect()>0) usleep(100);

    HaltGui();
    for (temp=Index; !NoIndex(temp); temp=temp->prox)
        mainWindow->Remove(temp->elem);
    ResumeGui();

    DistruggiText(text);
    DistruggiButton(btn);
    DistruggiImg(img);
    DistruggiIndex(Index);
}
