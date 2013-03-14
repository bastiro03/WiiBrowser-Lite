#include <stdio.h>
#include "html.h"
#include "config.h"

#include "include/litehtml.h"
#include "litehtml/viewwnd.h"
#define LEN 15

enum html htm;
char tags [END][LEN]= {"html","head","body","base","meta","title","form","p","a","div",
                       "br","img","h1","h2","h3","h4","h5","h6","b","big","blockquote","td","dd","dt",/*"center",*/"li","cite","font"
                      };
enum { THREAD_EXIT=0, THREAD_SUSPEND, THREAD_RUN };

static int threadState;
bool isRunning=false;
static ListaDiImg img;

void ResumeThread(lwp_t thread)
{
    threadState=THREAD_RUN;
    LWP_ResumeThread (thread);
}

void HaltThread(lwp_t thread)
{
    threadState=THREAD_SUSPEND;
    while(!LWP_ThreadIsSuspended(thread))   // wait for thread to finish
        usleep(100);
}

static void *DownloadImage (void *arg)
{
    ListaDiImg lista;
    isRunning=true;
    while(threadState!=THREAD_EXIT)
    {
        if (threadState==THREAD_SUSPEND)
        {
            LWP_SuspendThread(LWP_GetSelf());
        }
        else
        {
            usleep(100);
            for (lista=img; !NoImg(lista); lista=lista->prox)
            {
                if (threadState==THREAD_EXIT || threadState==THREAD_SUSPEND) break;
                if (!lista->imgdata->GetImage() && lista->tag)
                {
                    string tmp=adjustUrl(lista->tag->value[0].text, (char*)arg);
                    struct block THREAD = downloadfile(curl_handle, tmp.c_str(), NULL);
                    if(THREAD.size>0 && strstr(THREAD.type, "image"))
                    {
                        lista->imgdata=new GuiImageData ((u8*)THREAD.data, THREAD.size);
                        lista->img->SetImage(lista->imgdata);
                        lista->img->SetScale(screenwidth-80, atoi(lista->tag->attribute.c_str()));
                        lista->img->SetEffect(EFFECT_FADE, 50);
                    }
                    free(THREAD.data);
                }
            }
        }
    }
    isRunning=false;
    return NULL;
}

int knownType(char type[])
{
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

string DisplayHTML(struct block *HTML, GuiWindow *parentWindow, GuiWindow *mainWindow, char *url)
{
    static lwp_t thread = LWP_THREAD_NULL;
    int type = knownType(HTML->type);
    int coordX = 0, coordY = 0, offset = 0, choice = 0;
    bool done = false;

    GuiWindow *scrollWindow = new GuiWindow(50, screenheight-80);
    scrollWindow->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    scrollWindow->SetPosition(-40, 40);

    GuiImageData Textbox (keyboard_textbox_png, keyboard_textbox_png_size);
    string link;
    GuiImage *image=NULL;

    GuiButton *btndown=NULL, *btnup=NULL;
    list<GuiElement *> l1;
    list<GuiElement *>::iterator lista;

    ListaDiTesto text=InitText();
    ListaDiBottoni bottone, btn=InitButton();
    Indice first=NULL, last=NULL, ext=NULL, Index=InitIndex();
    // threadState=THREAD_RUN;
    img=InitImg();

    GuiSound *btnSoundOver= new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiTrigger *trigA=new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    #ifdef MPLAYER
    if (!strncmp(url, "http://www.youtube.", 19)
            || !strncmp(url, "https://www.youtube.", 20))
    {
        char newurl[2048];

        if(LoadYouTubeFile(newurl, HTML->data))
        {
            LoadMPlayerFile(newurl);
            while(controlledbygui != 1)
                usleep(100);

            ResetVideo_Menu();
            DisableVideoImg();
            ResumeGui();
        }
    }
    else
    #endif

    if (type == WEB)
    {
        // LWP_CreateThread (&thread, DownloadImage, (void*)url, NULL, 0, 70);
        wchar_t* css = charToWideChar((char *)master_css);

        litehtml::context m_context;
        m_context.load_master_stylesheet(css);
        delete(css);

        wchar_t* wurl = charToWideChar(url);
        CHTMLViewWnd wii(&m_context);
        wii.open(wurl);
        delete(wurl);

        litehtml::position clip(0, 0, screenwidth, wii.m_doc->height());
        wii.SetClip(clip);
        wii.OnPaint();

        l1 = wii.m_list;
        lista = l1.begin();

        unsigned int i;
        while (!choice)
        {
            Clear(mainWindow, Index, &first, &last, ext);
            coordY=Index ? Index->elem->GetYPosition()+Index->elem->GetHeight() : 0;
            if (!ext && Index) ext=Index;

            if (first && first->prox)
            {
                if (first->prox->elem->GetYPosition()+first->prox->elem->GetHeight()>=0)
                {
                    first->prox->elem->SetVisible(true);
                    first->prox->elem->SetEffect(EFFECT_FADE, 50);
                    first=first->prox;
                }
            }

            if (last && last->prec)
            {
                if (last->prec->elem->GetYPosition()<screenheight)
                {
                    last->prec->elem->SetVisible(true);
                    last->prec->elem->SetEffect(EFFECT_FADE, 50);
                    last=last->prec;
                }
            }

            else if (lista!=l1.end() && coordY<screenheight)
            {
                HaltGui();
                mainWindow->Append(*lista);
                ResumeGui();
                Index=InsIndex(Index);
                Index->elem=*lista;
                lista++;
            }

            else if (!done)
            {
                HaltGui();
                DoMPlayerGuiDraw();
                u8 *video = TakeScreenshot();
                ResumeGui();

                int t = 0;
                if ((t = Settings.FindUrl(url)) >= 0)
                    Settings.Thumbnails[t] = video;
                done = true;
            }

            if (!btnup)
            {
                DrawScroll (scrollWindow, &btndown, &btnup, btnSoundOver, trigA);
                HaltGui();
                mainWindow->Append(scrollWindow);
                ResumeGui();
            }

            if (userInput[0].wpad->btns_d & WPAD_BUTTON_A) {
                if(userInput[0].wpad->ir.valid)
                {
                    wii.OnLButtonDown(userInput[0].wpad->ir.x, userInput[0].wpad->ir.y);
                    wchar_t *wlink = wii.OnLButtonUp(userInput[0].wpad->ir.x, userInput[0].wpad->ir.y);
                    if (wlink)
                    {
                        char *ascii = new char[wcslen(wlink) + 1];
                        int bt = wcstombs(ascii, wlink, wcslen(wlink));
                        ascii[bt] = 0;

                        link.assign(ascii);
                        choice = 1;
                    }
                }
            }

            HandleHtmlPad(&coordX, btnup, btndown, ext, Index, mainWindow);
            HandleMenuBar(&link, url, &choice, 0, mainWindow, parentWindow);
        }
        l1.clear();
    }

    else if (type == IMAGE)
    {
        GuiImageData image_data((u8*)HTML->data, HTML->size);
        image = new GuiImage(&image_data);
        image->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        image->SetPosition(0,0);

        HaltGui();
        mainWindow->Append(image);
        ResumeGui();

        DrawScroll (scrollWindow, &btndown, &btnup, btnSoundOver, trigA);
        HaltGui();
        mainWindow->Append(scrollWindow);
        ResumeGui();

        while (!choice && !(userInput[0].wpad->btns_d & WPAD_BUTTON_B))
        {
            HandleMenuBar(&link, url, &choice, 1, mainWindow, parentWindow);
            HandleImgPad(btnup, btndown, image);
        }

        HaltGui();
        mainWindow->Remove(image);
        ResumeGui();
        delete(image);
    }

    #ifdef MPLAYER
    else if (type == VIDEO)
    {
        LoadMPlayerFile(url);
        while(controlledbygui != 1)
            usleep(100);

        ResetVideo_Menu();
        DisableVideoImg();
        ResumeGui();
    }
    #endif

    if (!knownType(HTML->type) || type == VIDEO || choice == 2)
    {
        if (!choice)
            choice = WindowPrompt("Download", "Do you want to save the file?", "Yes", "No");
        if (choice)
        {
            FILE *file = SelectFile(parentWindow, HTML->type);
            if (file)
                save(HTML, file);
        }
        usleep(100*1000);
    }

    threadState=THREAD_EXIT;
    while (isRunning)
        usleep(100);

    HaltGui();
    mainWindow->Remove(scrollWindow);
    ResumeGui();
    FreeMem(mainWindow, text, btn, img, Index);

    if (!hidden)
        hideBar(mainWindow, parentWindow);

    /*if (statusBar) {
        delAllElements();
        statusBar=NULL;
    }*/

    if (btnup)
    {
        delete(btnup);
        delete(btndown);
    }

    delete(btnSoundOver);
    delete(scrollWindow);
    delete(trigA);
    return link;
}

void Clear(GuiWindow* mainWindow, Indice Index, Indice *first, Indice *last, Indice ext)
{
    if (NoIndex(*first)) *first=ext;
    if (NoIndex(*last)) *last=Index;
    if (*first && (*first)->elem->GetYPosition()+(*first)->elem->GetHeight() <0)
    {
        (*first)->elem->SetVisible(false);
        *first=(*first)->prec;
    }
    if (*last && (*last)->elem->GetYPosition()>screenheight)
    {
        (*last)->elem->SetVisible(false);
        *last=(*last)->prox;
    }
}

void SetFont(GuiText *text, vector<string> mode)
{
    if (checkTag(mode, "b") || checkTag(mode, "strong"))
        text->SetFont(font_bold_ttf, font_bold_ttf_size);
    if (checkTag(mode, "em") || checkTag(mode, "i"))
        text->SetFont(font_italic_ttf, font_italic_ttf_size);
}

void DrawScroll (GuiWindow * mainWindow, GuiButton **btndown, GuiButton **btnup, GuiSound *btnSoundOver, GuiTrigger *trigA)
{
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

void FreeMem(GuiWindow* mainWindow, ListaDiTesto text, ListaDiBottoni btn, ListaDiImg img, Indice Index)
{
    Indice temp, effect=NULL;
    for (temp=Index; !NoIndex(temp); temp=temp->prox)
    {
        if (temp->elem->IsVisible())
        {
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
