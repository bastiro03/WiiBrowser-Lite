#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "html.h"
#include "html_helpers.h"
#include "config.h"
#include "httplib.h"
#include "gui_lock.h"
#include "safety.h"
#include "file_io.h"
#include "url_helper.h"
#include <atomic>

#define minwidth MIN(width, screenwidth-80)
#define LEN 15

enum html htm;
char tags [END][LEN] = {"html","head","body","base","meta","title","form","p","a","div",
                        "br","img","h1","h2","h3","h4","h5","h6","b","big","blockquote","pre","td","dd","dt",/*"center",*/"li","cite","font"
                       };

static std::atomic<int> threadState(THREAD_RUN);

static ListaDiImg img;
static ListaDiBottoni btn;

static GuiSound *btnSoundOver = NULL;
static GuiTrigger *trigA = NULL;
static GuiWindow * renderWindow = NULL;
static bool imageCancelled(void *) { return threadState == THREAD_EXIT || App_Exiting(); }

void ResumeThread(lwp_t thread)
{
    (void)thread;
    ResumeGui();
}

void HaltThread(lwp_t thread)
{
    (void)thread;
    HaltGui();
}

static void *DownloadImage (void *arg)
{
    CURL *curl_img=curl_easy_init();

    // Take ownership of heap base-URL copy (strdup'd by creator).
    // Copy to std::string up-front so worker never touches caller stack.
    char *baseUrl = (char *)arg;
    std::string base = baseUrl ? baseUrl : "";
    free(baseUrl);

    if (!curl_img) return NULL;
    while (threadState != THREAD_EXIT && !App_Exiting()) {
        Image *item = NULL;
        std::string source;
        {
            GuiScopedLock lock;
            for (Image *candidate = img; candidate; candidate = candidate->prox) {
                if (!candidate->fetched && candidate->tag) {
                    candidate->fetched = true;
                    item = candidate;
                    source = adjustUrl(candidate->tag->attribute, base.c_str());
                    break;
                }
            }
        }
        if (!item) { usleep(10000); continue; }
        block response = downloadfile(curl_img, source.c_str(), NULL, imageCancelled, NULL);
        if (response.size > 0 && strstr(response.type, "image")) {
            GuiImageData *decoded = new (std::nothrow) GuiImageData(reinterpret_cast<u8 *>(response.data), response.size);
            GuiScopedLock lock;
            if (threadState != THREAD_EXIT && decoded && decoded->GetImage()) {
                item->imgdata = decoded;
                item->img->SetImage(decoded);
                const int width = MIN(imageSize(item->tag, item->img).width, screenwidth - 80);
                const int height = MIN(imageSize(item->tag, item->img).height, screenheight);
                item->img->SetScale(width, height);
                if (item->linkButton) item->linkButton->SetSize(width, height);
                item->img->SetEffect(EFFECT_FADE, 50);
            } else delete decoded;
        }
        free(response.data);
    }
    curl_easy_cleanup(curl_img);
    return NULL;
}

bool AddImage(Lista::iterator lista, char *url)
{
    (void)url;
    img = InsImg(img);
    img->img = new GuiImage();
    img->tag = &(*lista);
    return true;
}

void AddButton(Lista::iterator lista, int type, GuiImage *image, void *arg)
{
    btn=InsButton(btn);
    int width, height;

    switch (type)
    {
        case ANCHOR: {
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
        }

        case IMAGE:
            width = MIN(imageSize(&(*lista), image).width, screenwidth-80);
            height = MIN(imageSize(&(*lista), image).height, screenwidth-80);

            btn->btn=new GuiButton(width, height);
            if (img) img->linkButton = btn->btn;
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
            btn->ownedImage = image;
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

string DisplayHTML(struct block *HTML, GuiWindow *parentWindow, GuiWindow *mainWindow, char *url)
{
    const std::string navigationUrl = url ? url : "";
    std::string documentBase = navigationUrl;
    static lwp_t thread = LWP_THREAD_NULL;
    char *title = NULL;
    renderWindow = mainWindow;

    Page Doc;
    Doc.XPos = Doc.YPos = 0;
    Doc.Height = 0;

    int type = knownType(HTML->type);
    int offset = 0, choice = 0;
    bool done = false;
    u64 refreshAt = 0;
    std::string refreshUrl;

    GuiWindow *scrollWindow = new GuiWindow(50, screenheight-80);
    scrollWindow->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    scrollWindow->SetPosition(-40, 40);

    GuiImageData Textbox (keyboard_textbox_png, keyboard_textbox_png_size);
    string link;
    GuiImage *image=NULL;

    GuiButton *btndown=NULL, *btnup=NULL;
    ScrollResources scrollResources;
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
        for (const Tag &tag : l1) {
            if (tag.name == "base") {
                const std::string resolved = parseUrl(tag.attribute, navigationUrl.c_str());
                if (wbl::httpUrl(resolved)) documentBase = resolved;
                break;
            }
        }
        url = const_cast<char *>(documentBase.c_str()); // Borrowed, never resized.
        lista=l1.begin();
        // Pass a heap copy of the base URL: never hand the worker the
        // address of this function's stack slot (&url).
        char *baseCopy = strdup(url ? url : "");
        if(baseCopy)
        {
            if(thread != LWP_THREAD_NULL)
            {
                // Previous page's worker already exited (THREAD_EXIT joins
                // via isRunning below); reap handle before overwriting.
                LWP_JoinThread(thread, NULL);
                thread = LWP_THREAD_NULL;
            }
            if(LWP_CreateThread(&thread, DownloadImage, (void*)baseCopy, NULL, 0, 70) != 0)
                free(baseCopy);
        }

        unsigned int i;
        while (!choice && !App_Exiting())
        {
            if (refreshAt && gettime() >= refreshAt) { link = refreshUrl; choice = 1; }
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
                            if(prevItemIs(lista, l1.begin(), "img"))
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
                    if (!lista->value.empty() && lista->value[0].text == "refresh") {
                        const int delay = getTime(lista->attribute);
                        int valid = 0;
                        refreshUrl = getUrl(&valid, lista->attribute);
                        if (delay >= 0 && valid)
                            refreshAt = gettime() + secs_to_ticks(delay ? delay : 1);
                    }
                }

                else if (lista->name=="base")
                {
                    // Already resolved before worker creation; caller owns URL.
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

            else if (!done && Settings.ShowThumbs)
            {
                HaltGui();
                DoMPlayerGuiDraw();
                u8 *video = TakeScreenshot(0);
                ResumeGui();

                int t = 0;
                if(video && ((t = Settings.FindUrl(new_page)) >= 0
                        || (t = Settings.FindUrl(url)) >= 0))
                {
                    Settings.Remove(t, 1);
                    Settings.Thumbnails[t] = video;
                }
                else free(video);
                done = true;
            }

            if (!btnup)
            {
                DrawScroll (scrollWindow, &btndown, &btnup, btnSoundOver, trigA, scrollResources);
                HaltGui();
                renderWindow->Append(scrollWindow);
                ResumeGui();
            }

            HandleHtmlPad(&offset, btnup, btndown, ext, Index, renderWindow, parentWindow);
            HandleMenuBar(&link, const_cast<char *>(navigationUrl.c_str()), new_page, title, &choice, 0, renderWindow, parentWindow);

            if (choice == 2)
            {
                char path[260];
                FILE *file;

                if (!hidden)
                    hideBar(renderWindow, parentWindow);

                if (GuiBrowser(renderWindow, parentWindow, path, sizeof(path), "Save page"))
                {
                    if (correctPath(path, sizeof(path), HTML->type, PAGE) &&
                        (!CheckFile(path) || WindowPrompt("Replace file?", path, "Replace", "Cancel"))) {
                        if (!wbl::writeUserFile(path, HTML->data, HTML->size, true))
                            WindowPrompt("Save failed", "No existing backup was overwritten; check .new/.bak files.", "OK", NULL);
                    }
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
                    if (choice) {
                        if (bottone->url.empty() && !bottone->refs) { choice = 0; continue; }
                        link = parseUrl(bottone->url, url);
                        if (bottone->post)
                            SetNavigationPost(parseUrl(link, url), bottone->postBody);
                    }
                }
            }
        }
        // Tags and their form inputs stay alive until the image worker joins.
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

        DrawScroll (scrollWindow, &btndown, &btnup, btnSoundOver, trigA, scrollResources);
        HaltGui();
        renderWindow->Append(scrollWindow);
        ResumeGui();

        while (!choice && !App_Exiting())
        {
            HandleImgPad(btnup, btndown, image);
            HandleMenuBar(&link, url, new_page, NULL, &choice, 1, renderWindow, parentWindow);

            if (choice == 2)
            {
                char path[260];
                FILE *file;

                if (!hidden)
                    hideBar(renderWindow, parentWindow);

                if (GuiBrowser(renderWindow, parentWindow, path, sizeof(path), "Save image"))
                {
                    if (correctPath(path, sizeof(path), HTML->type, IMAGES) &&
                        (!CheckFile(path) || WindowPrompt("Replace file?", path, "Replace", "Cancel"))) {
                        if (!wbl::writeUserFile(path, HTML->data, HTML->size, true))
                            WindowPrompt("Save failed", "No existing backup was overwritten; check .new/.bak files.", "OK", NULL);
                    }
                }
                choice = 0;
            }
        }

        image->SetEffect(EFFECT_FADE, -50);
        while (!App_Exiting() && image->GetEffect()>0)
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
            if (GuiBrowser(NULL, parentWindow, path, sizeof(path), "Save page"))
            {
                if (correctPath(path, sizeof(path), HTML->type, OTHER) &&
                    (!CheckFile(path) || WindowPrompt("Replace file?", path, "Replace", "Cancel"))) {
                    if (!wbl::writeUserFile(path, HTML->data, HTML->size, true))
                        WindowPrompt("Save failed", "No existing backup was overwritten; check .new/.bak files.", "OK", NULL);
                }
            }
        }
        usleep(100*1000);
    }

    threadState=THREAD_EXIT;
    if(thread != LWP_THREAD_NULL)
    {
        LWP_JoinThread(thread, NULL);
        thread = LWP_THREAD_NULL;
    }

    HaltGui();
    renderWindow->Remove(scrollWindow);
    ResumeGui();
    FreeMem(renderWindow, text, btn, img, Index);
    for (Tag &tag : l1) DistruggiInput(tag.form.input);
    l1.clear();

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
    return link.empty() ? link : parseUrl(link, documentBase.c_str());
}
