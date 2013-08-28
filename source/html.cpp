#include <stdio.h>
#include "audio.h"
#include "html.h"
#include "config.h"

#define minwidth MIN(width, screenwidth-80)
#define LEN 15

enum html htm;
char tags [END][LEN] = {"html","head","body","base","meta","title","form","p","a","div",
                        "br","img","h1","h2","h3","h4","h5","h6","b","big","blockquote","pre","td","dd","dt",/*"center",*/"li","cite","font"
                       };

static int threadState;
bool isRunning = false;

static ListaDiImg img;
static ListaDiBottoni btn;

static GuiSound *btnSoundOver = NULL;
static GuiTrigger *trigA = NULL;
static GuiWindow * renderWindow = NULL;

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
    CURL *curl_img=curl_easy_init();

    int width, height;

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
                if (!lista->fetched && !lista->img->GetImage() && lista->tag)
                {
                    string tmp=adjustUrl(lista->tag->attribute, *(char**)arg);
                    struct block THREAD = downloadfile(curl_img, tmp.c_str(), NULL);
                    if(THREAD.size>0 && strstr(THREAD.type, "image"))
                    {
                        lista->imgdata=new GuiImageData ((u8*)THREAD.data, THREAD.size);
                        lista->img->SetImage(lista->imgdata);
                        width = MIN(imageSize(lista->tag, lista->img).width, screenwidth-80);
                        height = MIN(imageSize(lista->tag, lista->img).height, screenheight);
                        lista->img->SetScale(width, height);
                        lista->img->SetEffect(EFFECT_FADE, 50);
                    }
                    lista->fetched = true;
                    free(THREAD.data);
                }
            }
        }
    }

    curl_easy_cleanup(curl_img);
    isRunning=false;
    return NULL;
}

bool AddImage(Lista::iterator lista, char *url)
{
    bool ret = false;
    string tmp = adjustUrl(lista->attribute, url);

    if (imageSize(&(*lista), NULL).height < INT_MAX)    // height known
    {
        img = InsImg(img);
        img->img = new GuiImage();
        img->tag = &(*lista);

        return true;
    }

    struct block IMAGE = downloadfile(curl_handle, tmp.c_str(), NULL);
    if(IMAGE.size>0 && strstr(IMAGE.type, "image"))
    {
        img = InsImg(img);
        img->imgdata = new GuiImageData((u8*)IMAGE.data, IMAGE.size);
        img->img = new GuiImage(img->imgdata);
        img->img->SetEffect(EFFECT_FADE, 50);

        ret = true;
    }

    free(IMAGE.data);
    return ret;
}

void AddButton(Lista::iterator lista, int type, GuiImage *image, void *arg)
{
    btn=InsButton(btn);
    int width, height;

    switch (type)
    {
        case ANCHOR:
            btn->label=new GuiText((char*)lista->value[*(int*)arg].text.c_str(), 20, (GXColor)
            {
                0, 0, 255, 255
            });
            btn->label->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            btn->label->SetSpace(false);
            SetFont(btn->label, lista->value[*(int*)arg].mode);

            btn->label->SetPosition(0,0);
            btn->tooltip=new GuiTooltip(lista->attribute.c_str());
            btn->btn=new GuiButton(btn->label->GetTextWidth(), 20);
            btn->btn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            btn->btn->SetLabel(btn->label);
            btn->btn->SetTooltip(btn->tooltip);
            btn->btn->SetModel(ANCHOR);
            btn->url.assign(lista->attribute);
        break;

        case IMAGE:
            width = MIN(imageSize(&(*lista), image).width, screenwidth-80);
            height = MIN(imageSize(&(*lista), image).height, screenwidth-80);

            btn->btn=new GuiButton(width, height);
            btn->btn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            btn->btn->SetPosition(image->GetXPosition(), image->GetYPosition());
            image->SetPosition(0, 0);

            btn->btn->SetImage(image);
            btn->btn->SetModel(BACKGROUND);
            btn->url.assign(lista->value[2].text);
            btn->btn->SetEffectFade();
        break;

        case FORM:
            btn->refs=&(*lista);
            btn->btn=new GuiButton(image->GetWidth(), image->GetHeight());
            btn->btn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            btn->btn->SetImage(image);
            btn->btn->SetEffectGrow();
        break;
    }

    btn->btn->SetSoundOver(btnSoundOver);
    btn->btn->SetTrigger(trigA);
    btn->btn->SetEffect(EFFECT_FADE, 50);

    HaltGui();
    renderWindow->BInsert(btn->btn);
    ResumeGui();
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

bool nextItemIs(Lista::iterator list, Lista::iterator end, string item)
{
    bool ret = false;

    if((++list) != end && list->name == item)
        ret = true;

    return ret;
}

bool prevItemIs(Lista::iterator list, string item)
{
    bool ret = false;

    if((--list)->name == item)
        ret = true;

    return ret;
}

string DisplayHTML(struct block *HTML, GuiWindow *parentWindow, GuiWindow *mainWindow, char *url)
{
    static lwp_t thread = LWP_THREAD_NULL;
    char *title = NULL;
    renderWindow = mainWindow;

    Page Doc;
    Doc.XPos = Doc.YPos = 0;
    Doc.Height = 0;

    int type = knownType(HTML->type);
    int offset = 0, choice = 0;
    bool done = false;

    GuiWindow *scrollWindow = new GuiWindow(50, screenheight-80);
    scrollWindow->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    scrollWindow->SetPosition(-40, 40);

    GuiImageData Textbox (keyboard_textbox_png, keyboard_textbox_png_size);
    string link;
    GuiImage *image=NULL;

    GuiButton *btndown=NULL, *btnup=NULL;
    Lista l1;
    Lista::iterator lista;
    ListaDiTesto text=InitText();
    ListaDiBottoni bottone;
    Indice Index, first=NULL, last=NULL, ext=NULL;
    threadState=THREAD_RUN;

    Index=InitIndex();
    img=InitImg();
    btn=InitButton();

    btnSoundOver=new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    trigA=new GuiTrigger;
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
            InitAudio(); // Initialize audio
            ResumeGui();
        }
    }
    else
#endif

    if (type == WEB)
    {
        l1=getTag((char*)HTML->data, url);
        lista=l1.begin();
        LWP_CreateThread (&thread, DownloadImage, (void*)&url, NULL, 0, 70);

        unsigned int i;
        while (!choice)
        {
            Clear(renderWindow, Index, &first, &last, ext);
            if (!ext && Index)
                ext=Index;

            Doc.YPos = ext ? ext->elem->GetYPosition() : 40;

            if (first && first->prox)
            {
                if (first->prox->elem->GetYPosition()+first->prox->screenSize>=0)
                {
                    first->prox->elem->SetVisible(true);
                    first->prox->elem->SetEffect(EFFECT_FADE, 50);
                    first=first->prox;
                }
            }

            if (last && last->prec)
            {
                if (last->prec->elem->GetYPosition()<screenheight+25)
                {
                    last->prec->elem->SetVisible(true);
                    last->prec->elem->SetEffect(EFFECT_FADE, 50);
                    last=last->prec;
                }
            }

            else if (lista!=l1.end() && Doc.YPos+Doc.Height<screenheight+25)
            {
                if (lista->name=="title" && !lista->value.empty())
                {
                    text=InsText(text);
                    title = (char*)lista->value[0].text.c_str();
                    text->txt = new GuiText(title, 30, (GXColor)
                    {
                        0, 0, 0, 255
                    });
                    text->txt->SetOffset(&offset);
                    text->txt->SetAlignment(ALIGN_MIDDLE, ALIGN_TOP);
                    text->txt->SetPosition(offset+screenwidth/2, Doc.YPos+Doc.Height);
                    text->txt->SetWrap(true, 400);
                    text->txt->SetEffect(EFFECT_FADE, 50);
                    HaltGui();
                    renderWindow->BInsert(text->txt);
                    ResumeGui();
                    Index=InsIndex(Index);
                    Index->elem=text->txt;
                    Index->screenSize=10+30*text->txt->GetLinesCount();
                    Doc.Height += Index->screenSize;
                }

                else if (lista->name=="a")
                {
                    for(i=0; i<lista->value.size(); i++)
                    {
                        AddButton(lista, ANCHOR, NULL, &i);
                        btn->label->SetOffset(&offset);

                        if (Doc.XPos >= (screenwidth-80))
                            Doc.XPos=Doc.XPos % (screenwidth-80);

                        btn->label->SetWrap(true, screenwidth-80-Doc.XPos);
                        btn->btn->SetPosition(offset+40+Doc.XPos, Doc.YPos+Doc.Height);

                        Index=InsIndex(Index);
                        Index->elem=btn->btn;
                        Doc.XPos+=btn->label->GetTextWidth();
                        Index->screenSize=(Doc.XPos/(screenwidth-80))*25;
                        Doc.Height += Index->screenSize;
                    }
                }

                else if (lista->name=="img" && lista->attribute.length()>0)
                {
                    HaltThread(thread);
                    if(AddImage(lista, url))
                    {
                        int width = imageSize(&(*lista), img->tag ? NULL : img->img).width;
                        int height = MIN(imageSize(&(*lista), img->tag ? NULL : img->img).height, screenheight);

                        if(width < INT_MAX) // width known
                        {
                            if(Doc.XPos+minwidth > (screenwidth-80))
                            {
                                Doc.Height+=Index->screenSize;
                                Doc.XPos=0;
                            }

                            img->img->SetPosition(offset+40+Doc.XPos, Doc.YPos+Doc.Height);
                            img->img->SetScale(minwidth, height);
                            Doc.XPos+=width;
                        }
                        else
                        {
                            if(prevItemIs(lista, "img"))
                                Doc.Height+=Index->screenSize;

                            img->img->SetPosition(offset+40, Doc.YPos+Doc.Height);
                            img->img->SetScale(minwidth, height);
                            Doc.XPos=minwidth;
                        }

                        if(checkTag(lista->value[2].mode, "a"))
                        {
                            AddButton(lista, IMAGE, img->img, NULL);

                            Index=InsIndex(Index);
                            Index->elem=btn->btn;
                            Index->screenSize=height+5;
                            Index->content=null;
                        }
                        else
                        {
                            HaltGui();
                            renderWindow->BInsert(img->img);
                            ResumeGui();

                            Index=InsIndex(Index);
                            Index->elem=img->img;
                            Index->screenSize=height+5;
                            Index->content=null;
                        }

                        if(!nextItemIs(lista, l1.end(), "img"))
                        {
                            Doc.Height+=Index->screenSize;
                            Doc.XPos=0;
                        }
                    }
                    ResumeThread(thread);
                }

                else if (lista->name=="form")
                {
                    GuiImage *TextboxImg=new GuiImage(&Textbox);
                    AddButton(lista, FORM, TextboxImg, NULL);
                    btn->btn->SetPosition(offset, Doc.YPos+Doc.Height);

                    Index=InsIndex(Index);
                    Index->elem=btn->btn;
                    Index->screenSize=60;
                    Doc.Height+=Index->screenSize;
                }

                else if (lista->name=="meta")
                {
                    if (!lista->value.empty())
                        choice = HandleMeta(lista, &link, HTML);
                }

                else if (lista->name=="base")
                {
                    url = (char*)realloc(url, lista->attribute.size()+1);
                    strcpy(url, lista->attribute.c_str());
                }

                else if (lista->name=="return")
                {
                    if (!nextItemIs(lista, l1.end(), "p") && !nextItemIs(lista, l1.end(), "return"))
                    {
                        Doc.Height+=25;
                        Doc.XPos=0;
                    }
                }

                else if (lista->name=="p")
                {
                    if (!nextItemIs(lista, l1.end(), "p") && !nextItemIs(lista, l1.end(), "return"))
                    {
                        Doc.Height+=50;
                        Doc.XPos=0;
                    }
                }

                else
                {
                    for(i=0; i<lista->value.size(); i++)
                    {
                        text=InsText(text);
                        text->txt = new GuiText((char*)lista->value[i].text.c_str(), 20, (GXColor)
                        {
                            0, 0, 0, 255
                        });
                        text->txt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
                        text->txt->SetSpace(false);
                        text->txt->SetOffset(&offset);

                        if (Doc.XPos >= (screenwidth-80))
                            Doc.XPos=Doc.XPos % (screenwidth-80);

                        text->txt->SetPosition(offset+40+Doc.XPos, Doc.YPos+Doc.Height);
                        text->txt->SetWrap(true, screenwidth-80-Doc.XPos);
                        text->txt->SetEffect(EFFECT_FADE, 50);
                        SetFont(text->txt, lista->value[i].mode);
                        HaltGui();
                        renderWindow->BInsert(text->txt);
                        ResumeGui();
                        Doc.XPos+=text->txt->GetTextWidth();
                        Index=InsIndex(Index);
                        Index->elem=text->txt;
                        Index->screenSize=(Doc.XPos/(screenwidth-80))*25;
                        Doc.Height+=Index->screenSize;
                    }
                }
                lista++;
            }

            else if (!done)
            {
                HaltGui();
                DoMPlayerGuiDraw();
                u8 *video = TakeScreenshot(0);
                ResumeGui();

                int t = 0;
                if ((t = Settings.FindUrl(new_page)) >= 0
                        || (t = Settings.FindUrl(url)) >= 0)
                {
                    Settings.Remove(t, 1);
                    Settings.Thumbnails[t] = video;
                }
                else free(video);
                done = true;
            }

            if (!btnup)
            {
                DrawScroll (scrollWindow, &btndown, &btnup, btnSoundOver, trigA);
                HaltGui();
                renderWindow->Append(scrollWindow);
                ResumeGui();
            }

            HandleHtmlPad(&offset, btnup, btndown, ext, Index, renderWindow, parentWindow);
            HandleMenuBar(&link, url, new_page, title, &choice, 0, renderWindow, parentWindow);

            if (choice == 2)
            {
                char path[260];
                FILE *file;

                if (!hidden)
                    hideBar(renderWindow, parentWindow);

                if (GuiBrowser(renderWindow, parentWindow, path, "Save page"))
                {
                    correctPath(path, HTML->type, PAGE);
                    file = fopen(path, "wb");
                    save(HTML, file);
                }
                choice=0;
            }

            for (bottone=btn; !NoButton(bottone); bottone=bottone->prox)
            {
                if(bottone->btn->GetState() == STATE_CLICKED)
                {
                    bottone->btn->ResetState();
                    choice = 1;
                    if (bottone->refs)
                        choice=HandleForm(parentWindow, renderWindow, bottone);
                    if (choice) link.assign(bottone->url);
                }
            }
        }
        l1.clear();
    }

    else if (type == IMAGE)
    {
        GuiImageData image_data((u8*)HTML->data, HTML->size);
        image = new GuiImage(&image_data);
        image->SetEffect(EFFECT_FADE, 50);
        image->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        image->SetPosition(0,0);

        HaltGui();
        renderWindow->Append(image);
        ResumeGui();

        DrawScroll (scrollWindow, &btndown, &btnup, btnSoundOver, trigA);
        HaltGui();
        renderWindow->Append(scrollWindow);
        ResumeGui();

        while (!choice)
        {
            HandleImgPad(btnup, btndown, image);
            HandleMenuBar(&link, url, new_page, NULL, &choice, 1, renderWindow, parentWindow);

            if (choice == 2)
            {
                char path[260];
                FILE *file;

                if (!hidden)
                    hideBar(renderWindow, parentWindow);

                if (GuiBrowser(renderWindow, parentWindow, path, "Save image"))
                {
                    correctPath(path, HTML->type, IMAGES);
                    file = fopen(path, "wb");
                    save(HTML, file);
                }
                choice = 0;
            }
        }

        image->SetEffect(EFFECT_FADE, -50);
        while (image->GetEffect()>0)
            usleep(100);

        HaltGui();
        renderWindow->Remove(image);
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
        // DisableVideoImg();
        InitAudio(); // Initialize audio
        ResumeGui();
    }
#endif

    /* download displayed page or image */
    if (!knownType(HTML->type))
    {
        char path[260];
        FILE *file;
        choice = WindowPrompt("Download", "Do you want to save the page?", "Yes", "No");

        if (choice)
        {
            if (GuiBrowser(NULL, parentWindow, path, "Save page"))
            {
                correctPath(path, HTML->type, OTHER);
                file = fopen(path, "wb");
                save(HTML, file);
            }
        }
        usleep(100*1000);
    }

    threadState=THREAD_EXIT;
    while (isRunning)
        usleep(100);

    HaltGui();
    renderWindow->Remove(scrollWindow);
    ResumeGui();
    FreeMem(renderWindow, text, btn, img, Index);

    if (!hidden)
        hideBar(renderWindow, parentWindow);

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
    if (*first && (*first)->elem->GetYPosition()+(*first)->screenSize <0)
    {
        (*first)->elem->SetVisible(false);
        *first=(*first)->prec;
    }
    if (*last && (*last)->elem->GetYPosition()>screenheight+25)
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
