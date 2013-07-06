/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * modified by gave92
 *
 * WiiBrowser
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wiiuse/wpad.h>

#include "liste.h"
#include "main.h"
#include "input.h"
#include "networkop.h"

#include "filebrowser.h"
#include "fileop.h"
#include "filelist.h"
#include "config.h"

extern "C" {
#include "mplayer/stream/url.h"
#include "urlcode.h"
}

#define THREAD_SLEEP    100
#define MAXLEN          512
#define N               9

static u8 loadstack[GUITH_STACK] ATTRIBUTE_ALIGN (32);
static u8 updatestack[GUITH_STACK] ATTRIBUTE_ALIGN (32);
static u8 guistack[GUITH_STACK] ATTRIBUTE_ALIGN (32);

CURL *curl_handle;
CURLM *curl_multi;
History history;

char new_page[MAXLEN];
static char prev_page[MAXLEN];
static char Message[MAXLEN];

static int fadeAnim;
static int prevMenu;

static GuiSwitch * Right = NULL;
static GuiSwitch * Left = NULL;
static GuiToolbar * App = NULL;
static GuiTrigger * trigA = NULL;

static GuiImageData * pointerData[4];
static GuiImageData * pointerGrabData[4];
static GuiImageData * pointer[4];

static const u8 * pointerImg[4];
static const u8 * pointerGrabImg[4];

static GuiImage * bgImg = NULL;
static GuiButton * actionButton = NULL;

static GuiImageData * SplashImage = NULL;
static GuiImage * Splash = NULL;
static GuiImage * videoImg = NULL;

static GuiWindow * guiWindow = NULL;
static GuiWindow * videoWindow = NULL;
GuiWindow * mainWindow = NULL;

static lwp_t guithread = LWP_THREAD_NULL;
static lwp_t updatethread = LWP_THREAD_NULL;
static lwp_t loadthread = LWP_THREAD_NULL;

static bool guiHalt = true;
static int updateThreadHalt = 0;
static int loadThreadHalt = 1;

using namespace std;

/****************************************************************************
 * Adjust urls
 ***************************************************************************/
char *getHost(char *url)
{
    char *p=strchr (url, '/')+2;
    char *c=strchr (p, '/');
    if (c != NULL)
        return strndup(url,(c+1)-url);
    return url;
}

string getRoot(char *url)
{
    char *i=strchr (url, '/');
    char *p=strrchr (i+2, '/');
    string root(url);
    if (p != NULL)
        root.assign(url,(p+1)-url);
    else root.append("/");
    return root;
}

string adjustUrl(string link, const char* url)
{
    string result;
    if (link.find("http://")==0 || link.find("https://")==0)
        return link;
    else if (link.at(0)=='/' && link.at(1)=='/')
        result.assign("http:"); // https?
    else if (link.at(0)=='/')
    {
        result=getHost((char*)url);
        if (*result.rbegin()=='/')
            link.erase(link.begin());
    }
    else result=getRoot((char*)url);
    result.append(link);
    return result;
}

string jumpUrl(string link, const char* url)
{
    string res(url, strlen(url));
    res.append(link);
    return res;
}

string parseUrl(string link, const char* url)
{
    if (link.at(0)=='#')
        return jumpUrl(link, url);
    return adjustUrl(link, url);
}

/****************************************************************************
 * Perform downloads
 ***************************************************************************/
bool performDownload(FILE **hfile, char *url, struct block *html)
{
    int choice;
    bool select = false;
    char path[260];

    choice = WindowPrompt("Download", "Do you want to download the file?", "Yes", "No");
    if (choice)
        select = GuiBrowser(NULL, mainWindow, path, "Download!");

    if (select)
    {
        downloadPath(html, url, path);
        *hfile = fopen(path, "wb");
    }
    else *hfile = NULL;

    free(html->data);
    return select;
}

/****************************************************************************
 * Handle screenshots
 ***************************************************************************/
#ifdef MPLAYER
void DisableVideoImg()
{
    if(!videoImg)
        return;

    videoImg->SetVisible(false);
}

void EnableVideoImg()
{
    if(!videoImg)
        return;

    videoImg->SetImage(videoScreenshot, vmode->fbWidth, vmode->viHeight);
    videoImg->SetVisible(true);
}

bool VideoImgVisible()
{
    if(!videoImg)
        return false;

    return videoImg->IsVisible();
}
#endif

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
void ResumeGui()
{
    guiHalt = false;
    LWP_ResumeThread (guithread);
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
extern "C" void HaltGui()
{
    guiHalt = true;

    // wait for thread to finish
    while(!LWP_ThreadIsSuspended(guithread))
        usleep(THREAD_SLEEP);
}

extern "C" void DoMPlayerGuiDraw()
{
    mainWindow->Draw();
    mainWindow->DrawTooltip();

    DoRumble(0);
    mainWindow->Update(&userInput[0]);
}

#ifdef MPLAYER
void UpdatePointer()
{
    if(userInput[0].wpad->ir.valid)
        Menu_DrawImg(userInput[0].wpad->ir.x-48, userInput[0].wpad->ir.y-48,
                     96, 96, pointer[0]->GetImage(), userInput[0].wpad->ir.angle, 1, 1, 255, GX_TF_RGBA8);
}

/****************************************************************************
 * LoadYouTubeFile
 *
 * Creates youtube video url
 ***************************************************************************/
bool LoadYouTubeFile(char *newurl, char *data)
{
    if(!data)
        return false;
    char *str = strstr(data, "url_encoded_fmt_stream_map");

    if(str == NULL)
        return false;

    int fmt, chosenFormat = 0;
    char *urlc, *urlcod, *urlcend, *fmtc, *fmtcend;
    char format[5];

    // get start point
    urlc = str+30;

    // get end point
    char *strend = strstr(urlc, ";");

    if(strend == NULL)
        return false;
    strend[0] = 0; //terminate the string

    // work through the string looking for required format
    char *tempurl = (char *)mem2_malloc(2048, MEM2_OTHER);
    if(!tempurl)
        return false;

    while(chosenFormat != 35 && urlc < strend-10)
    {
        //find section end %2C and set pointer to next section
        char sep[5];
        strcpy(sep, ",");
        strncat(sep, urlc, 3);
        urlcend = strstr(urlc, sep);

        char *nexturl = urlcend + 1;
        if(!urlcend) urlcend = strend;

        //get and decode section
        snprintf(tempurl,urlcend-urlc+1,"%s",urlc);

        url_unescape_string(tempurl, tempurl); // %252526 = %2526
        url_unescape_string(tempurl, tempurl); // %2526 = %26
        url_unescape_string(tempurl, tempurl); // %26 = &

        //get format code of this section
        fmtc = strstr(tempurl, "itag=");
        if(!fmtc) break;
        fmtcend = strstr(fmtc+7,"&");
        if(!fmtcend) break;

        snprintf(format, fmtcend-fmtc-5+1, "%s", fmtc+5);
        fmt = atoi(format);

        if((fmt == 5 || fmt == 18 || fmt == 35) && fmt <= 35 && fmt > chosenFormat)
        {
            urlcod = strstr(tempurl,"url=");
            if(!urlcod) break;

            // swap front and back
            snprintf(urlc,urlcod-tempurl+1,"%s",tempurl);
            strcpy(tempurl,urlcod+4);
            strcat(tempurl,"&");
            strcat(tempurl,urlc);

            // expand signature
            char *sig = strstr(tempurl,"&sig=");
            int siglen = strlen(tempurl)-(sig-tempurl)-3;
            memmove(sig+10,sig+4,siglen);
            memmove(sig,"&signature=",11);

            // remove &type=
            sig = strstr(tempurl,"&type=");
            char *sigend = strstr(sig+6,"&");
            siglen = strlen(tempurl)-(sigend-tempurl)+1;
            memmove(sig,sigend,siglen);

            // remove &fallback_host=
            sig = strstr(tempurl,"&fallback_host=");
            sigend = strstr(sig+15,"&");
            siglen = strlen(tempurl)-(sigend-tempurl)+1;
            memmove(sig,sigend,siglen);

            // remove duplicate &itag=
            sig = strstr(tempurl,"&itag=");
            sigend = strstr(sig+6,"&");
            siglen = strlen(tempurl)-(sigend-tempurl)+1;
            memmove(sig,sigend,siglen);

            //remove last &
            siglen = strlen(tempurl);
            tempurl[siglen-1] = 0;

            chosenFormat = fmt;
            strcpy(newurl, tempurl);
        }
        urlc = nexturl; // do next section
    }

    mem2_free(tempurl, MEM2_OTHER);
    if(chosenFormat > 0)
        return true;

    return false;
}
#endif

/****************************************************************************
 * LoadingThread
 *
 * Opens a window, which displays progress to the user.
 * Can display a throbber that shows that an action is in progress.
 ***************************************************************************/
static int progsleep = 0;
static GuiImageData * throbber = NULL;
static GuiText * msgTxt = NULL;

static void
ProgressWindow(char *msg)
{
    GuiWindow promptWindow(556,244);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiImage throbberImg(throbber);
    throbberImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    throbberImg.SetPosition(0, 40);
    throbberImg.SetScale(0.60);

    msgTxt = new GuiText(msg, 20, (GXColor)
    {
        0, 0, 0, 255
    });
    msgTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    msgTxt->SetPosition(0, 80);

    promptWindow.Append(msgTxt);
    promptWindow.Append(&throbberImg);

    if(loadThreadHalt > 0)
        return;
    float angle = 0;

    HaltGui();
    int oldState = mainWindow->GetState();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    ResumeGui();

    while(loadThreadHalt == 0)
    {
        progsleep = 100*1000;

        while(progsleep > 0)
        {
            usleep(THREAD_SLEEP);
            progsleep -= THREAD_SLEEP;
        }

        angle += 90.0f;
        if(angle >= 360.0f)
            angle = 0;
        throbberImg.SetAngle(angle);
    }

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(oldState);
    ResumeGui();

    delete(msgTxt);
    msgTxt = NULL;
}

static void *LoadingThread (void *arg)
{
    while(1)
    {
        if (loadThreadHalt == 1)
            LWP_SuspendThread(loadthread);
        if (loadThreadHalt == 2)
            return NULL;

        ProgressWindow(Message);
        usleep(THREAD_SLEEP);
    }
    return NULL;
}

void ShowAction (const char *msg)
{
    snprintf(Message, sizeof(Message), msg);

    loadThreadHalt = 0;
    LWP_ResumeThread (loadthread);
}

void SetMessage (const char *msg)
{
    if (!msgTxt)
        return;

    snprintf(Message, sizeof(Message), msg);

    msgTxt->SetText(msg);
}

void CancelAction()
{
    loadThreadHalt = 1;

    // wait for thread to finish
    while(!LWP_ThreadIsSuspended(loadthread))
        usleep(THREAD_SLEEP);
}

void StopLoading()
{
    if(loadthread == LWP_THREAD_NULL)
        return;

    loadThreadHalt = 2;
    LWP_ResumeThread(loadthread);
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, const char *longText)
{
    int choice = -1;

    GuiWindow promptWindow(448,288);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);
    GuiImageData btnOutlineOver(button_over_png);

    GuiImageData dialogBox(dialogue_box_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiText titleTxt(title, 26, (GXColor)
    {
        0, 0, 0, 255
    });
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,40);

    GuiText msgTxt(msg, 22, (GXColor)
    {
        0, 0, 0, 255
    });
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-20);
    msgTxt.SetWrap(true, 400);

    GuiLongText msgLongTxt(longText, 22, (GXColor)
    {
        0, 0, 0, 255
    });
    msgLongTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    msgLongTxt.SetPosition(20,130);
    msgLongTxt.SetLinesToDraw(8);
    msgLongTxt.SetMaxWidth(400);

    GuiText btn1Txt(btn1Label, 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage btn1Img(&btnOutline);
    GuiImage btn1ImgOver(&btnOutlineOver);
    GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

    if(btn2Label)
    {
        btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1.SetPosition(20, -25);
    }
    else
    {
        btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn1.SetPosition(0, -25);
    }

    btn1.SetLabel(&btn1Txt);
    btn1.SetImage(&btn1Img);
    btn1.SetImageOver(&btn1ImgOver);
    btn1.SetSoundOver(&btnSoundOver);
    btn1.SetTrigger(trigA);
    btn1.SetState(STATE_SELECTED);
    btn1.SetEffectGrow();

    GuiText btn2Txt(btn2Label, 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage btn2Img(&btnOutline);
    GuiImage btn2ImgOver(&btnOutlineOver);
    GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-20, -25);
    btn2.SetLabel(&btn2Txt);
    btn2.SetImage(&btn2Img);
    btn2.SetImageOver(&btn2ImgOver);
    btn2.SetSoundOver(&btnSoundOver);
    btn2.SetTrigger(trigA);
    btn2.SetEffectGrow();

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);

    if(longText)
    {
        msgTxt.SetPosition(0,-52);
        promptWindow.Append(&msgLongTxt);
    }

    if(btn1Label)
        promptWindow.Append(&btn1);

    if(btn2Label)
        promptWindow.Append(&btn2);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(choice == -1)
    {
        usleep(THREAD_SLEEP);

        if(btn1.GetState() == STATE_CLICKED)
            choice = 1;
        else if(btn2.GetState() == STATE_CLICKED)
            choice = 0;
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    return choice;
}

/****************************************************************************
 * UpdateThread
 *
 * Thread for application auto-update
 ***************************************************************************/
static void *UpdateThread (void *arg)
{
    struct update upd;
    char message[512];
    char caption[50];
    char *longText = NULL;

    while(1)
    {
        LWP_SuspendThread(updatethread);
        if(updateThreadHalt)
            return NULL;

        upd = checkUpdate();
        if(!upd.appversion)
            return NULL;
        usleep(500*1000);

        if(strlen(upd.changelog) > 0)
        {
            sprintf(caption, "Changelog rev%d", upd.appversion);
            sprintf(message, upd.changelog);
            longText = message;
        }
        else sprintf(caption, "An update is available!");

        bool installUpdate = WindowPrompt(
                                 "Update Available",
                                 caption,
                                 "Update now",
                                 "Update later",
                                 longText);

        if(installUpdate)
        {
            if(downloadUpdate(upd.appversion))
                ExitRequested = true;
        }
    }
    return NULL;
}

void ResumeUpdateThread()
{
    if(updatethread == LWP_THREAD_NULL || ExitRequested)
        return;

    updateThreadHalt = 0;
    LWP_ResumeThread(updatethread);
}

void StopUpdateThread()
{
    if(updatethread == LWP_THREAD_NULL)
        return;

    updateThreadHalt = 1;
    LWP_ResumeThread(updatethread);
}

/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/
static void *UpdateGUI (void *arg)
{
    int i, guiRun = 1;

    while(guiRun)
    {
        if(guiHalt)
        {
            LWP_SuspendThread(guithread);
        }
        else
        {
            UpdatePads();
            mainWindow->Draw();
            mainWindow->DrawTooltip();

#ifdef HW_RVL
            for(i=3; i >= 0; i--) // so that player 1's cursor appears on top!
            {
                if(userInput[i].wpad->ir.valid)
                    Menu_DrawImg(userInput[i].wpad->ir.x-48, userInput[i].wpad->ir.y-48,
                                 96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255, GX_TF_RGBA8);
                DoRumble(i);
            }
#endif

            for(i=0; i < 4; i++)
            {
                mainWindow->Update(&userInput[i]);
                if(userInput[i].wpad->btns_d & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME))
                    ExitRequested = true; // exit program
            }

            if(HWButton)
                ExitRequested = true; // exit program
            Menu_Render();

            if(ExitRequested)
            {
                for(i = guiRun = 0; i <= 255; i += 15)
                {
                    mainWindow->Draw();
                    Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor)
                    {
                        0, 0, 0, (u8)i
                    },1);
                    Menu_Render();
                }
            }

            if (ExitRequested)
                ExitApp();
        }
    }
    return NULL;
}

/****************************************************************************
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
void
InitGUIThreads()
{
    LWP_CreateThread (&guithread, UpdateGUI, NULL, guistack, GUITH_STACK, 70);
    LWP_CreateThread (&updatethread, UpdateThread, NULL, updatestack, GUITH_STACK, 70);
    LWP_CreateThread (&loadthread, LoadingThread, NULL, loadstack, GUITH_STACK, 70);
    LWP_CreateThread (&networkthread, NetworkThread, NULL, networkstack, GUITH_STACK, 70);
}

void
StopGUIThreads()
{
    if(networkthread != LWP_THREAD_NULL)
    {
        StopNetwork();
        LWP_JoinThread(networkthread, NULL);
        networkthread = LWP_THREAD_NULL;
    }

    if(loadthread != LWP_THREAD_NULL)
    {
        StopLoading();
        LWP_JoinThread(loadthread, NULL);
        loadthread = LWP_THREAD_NULL;
    }

    if(updatethread != LWP_THREAD_NULL)
    {
        StopUpdateThread();
        LWP_JoinThread(updatethread, NULL);
        updatethread = LWP_THREAD_NULL;
    }

    if(guithread != LWP_THREAD_NULL)
    {
        ResumeGui();
        LWP_JoinThread(guithread, NULL);
        guithread = LWP_THREAD_NULL;
    }
}

void ToggleButtons(GuiToolbar *toolbar, bool checkState)
{
    if (!toolbar)
        return;

    if (toolbar->btnBack->GetState() != STATE_SELECTED
            || checkState)
    {
        if (history && history->prec)
            toolbar->btnBack->SetState(STATE_DEFAULT);
        else toolbar->btnBack->SetState(STATE_DISABLED);
    }

    if (toolbar->btnForward->GetState() != STATE_SELECTED
            || checkState)
    {
        if (history && history->prox)
            toolbar->btnForward->SetState(STATE_DEFAULT);
        else toolbar->btnForward->SetState(STATE_DISABLED);
    }
}

extern "C" void ShutdownGui()
{
    HaltGui();
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
void OnScreenKeyboard(GuiWindow *keyboardWindow, char *var, u16 maxlen)
{
    int save = -1;

    GuiKeyboard keyboard(var, maxlen);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);
    GuiImageData btnOutlineOver(button_over_png);

    GuiText okBtnTxt("OK", 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage okBtnImg(&btnOutline);
    GuiImage okBtnImgOver(&btnOutlineOver);
    GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

    okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    okBtn.SetPosition(25, -25);

    okBtn.SetLabel(&okBtnTxt);
    okBtn.SetImage(&okBtnImg);
    okBtn.SetImageOver(&okBtnImgOver);
    okBtn.SetSoundOver(&btnSoundOver);
    okBtn.SetTrigger(trigA);
    okBtn.SetEffectGrow();

    GuiText cancelBtnTxt("Cancel", 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage cancelBtnImg(&btnOutline);
    GuiImage cancelBtnImgOver(&btnOutlineOver);
    GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    cancelBtn.SetPosition(-25, -25);
    cancelBtn.SetLabel(&cancelBtnTxt);
    cancelBtn.SetImage(&cancelBtnImg);
    cancelBtn.SetImageOver(&cancelBtnImgOver);
    cancelBtn.SetSoundOver(&btnSoundOver);
    cancelBtn.SetTrigger(trigA);
    cancelBtn.SetEffectGrow();

    keyboard.Append(&okBtn);
    keyboard.Append(&cancelBtn);
    keyboard.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 30);

    HaltGui();
    keyboardWindow->SetState(STATE_DISABLED);
    keyboardWindow->Append(&keyboard);
    keyboardWindow->ChangeFocus(&keyboard);
    ResumeGui();

    while(save == -1)
    {
        usleep(THREAD_SLEEP);

        if(okBtn.GetState() == STATE_CLICKED)
            save = 1;
        else if(cancelBtn.GetState() == STATE_CLICKED)
            save = 0;
    }

    if(save)
    {
        snprintf(var, maxlen, "%s", keyboard.kbtextstr);
    }

    keyboard.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
    while(keyboard.GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    keyboardWindow->Remove(&keyboard);
    keyboardWindow->SetState(STATE_DEFAULT);
    ResumeGui();
}

/****************************************************************************
 * SetupGui
 *
 * Called once
 ***************************************************************************/
static void DragCallback(void *ptr)
{
    GuiButton *b = (GuiButton *) ptr;
    int chan = b->GetStateChan();
    if (chan < 0)
        return;

    if (b->GetState() == STATE_HELD)
    {
        SetPointer(true, chan);
    }
    else
    {
        SetPointer(false, chan);
    }
}

void SetPointer(bool drag, int chan)
{
    if (drag)
    {
        pointer[chan] = pointerGrabData[chan];
    }
    else
    {
        pointer[chan] = pointerData[chan];
    }
}

void SetupGui()
{
    bgImg = new GuiImage(screenwidth, screenheight, (GXColor)
    {
        225, 225, 225, 255
    });
    bgImg->SetEffect(EFFECT_FADE, 50);
    trigA = new GuiTrigger();
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    guiWindow = new GuiWindow(screenwidth, screenheight);
    guiWindow->Append(bgImg);
    mainWindow = guiWindow;

    #ifdef MPLAYER
	videoImg = new GuiImage();
    videoImg->SetImage(videoScreenshot, vmode->fbWidth, vmode->viHeight);
    videoImg->SetScaleX(screenwidth/(float)vmode->fbWidth);
    videoImg->SetScaleY(screenheight/(float)vmode->efbHeight);
    videoImg->SetVisible(false);
    mainWindow->Append(videoImg);
	#endif

    throbber = new GuiImageData(loading_png);
    fadeAnim = EFFECT_FADE;
    prevMenu = MENU_HOME;

    pointerImg[0] = player1_point_png;
    pointerImg[1] = player2_point_png;
    pointerImg[2] = player3_point_png;
    pointerImg[3] = player4_point_png;
    pointerGrabImg[0] = player1_grab_png;
    pointerGrabImg[1] = player2_grab_png;
    pointerGrabImg[2] = player3_grab_png;
    pointerGrabImg[3] = player4_grab_png;

#ifdef HW_RVL
    for (int i = 0; i < 4; i++)
    {
        pointerData[i] = new GuiImageData(pointerImg[i]);
        pointerGrabData[i] = new GuiImageData(pointerGrabImg[i]);
        pointer[i] = pointerData[i];
    }
#endif

    Right = new GuiSwitch(0);
    Left = new GuiSwitch(1);
    App = new GuiToolbar(HOMEPAGE);

    SplashImage = new GuiImageData(logo_png);
    Splash = new GuiImage(SplashImage);

    Splash->SetAlignment(2,5);
    Splash->SetPosition(10,-140);
    Splash->SetScale(0.4);

    Right->SetPosition(15,-30);
    Left->SetPosition(0,-30);

    videoWindow = new GuiWindow(screenwidth, screenheight);
    videoWindow->Append(App);

    LWP_ResumeThread(networkthread);
    ResumeGui();
}

/****************************************************************************
 * MenuBrowseDevice
 ***************************************************************************/
static int MenuBrowseDevice()
{
	char title[100];
	char path[256];
	int i;

	ShutoffRumble();

	// populate initial directory listing
	if(BrowseDevice() <= 0)
	{
		int choice = WindowPrompt(
		"Error",
		"Unable to display files on selected load device.",
		"Retry",
		"Cancel");

		if(choice)
			return MENU_BROWSE_DEVICE;
		else
			return MENU_HOME;
	}

	int menu = MENU_NONE;

	sprintf(title, "Browse files");
	bzero(path, sizeof(path));

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData Textbox(keyboard_textbox_png);
    GuiImage TextboxImg(&Textbox);
    GuiButton InsertURL(TextboxImg.GetWidth(), TextboxImg.GetHeight());

    GuiText URL("", 20, (GXColor)
    {
        0, 0, 0, 255
    });
    URL.SetMaxWidth(TextboxImg.GetWidth()-20);
    URL.SetScroll(SCROLL_HORIZONTAL);

    InsertURL.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    InsertURL.SetPosition(0,30);
    InsertURL.SetLabel(&URL);
    InsertURL.SetImage(&TextboxImg);
    InsertURL.SetSoundOver(&btnSoundOver);
    InsertURL.SetTrigger(trigA);
    InsertURL.SetEffectGrow();

    GuiText titleTxt(title, 28, (GXColor){0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 108);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiText backBtnTxt("Go Back", 24, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetTrigger(trigA);
	backBtn.SetEffectGrow();

	GuiWindow buttonWindow(screenwidth, screenheight);
	buttonWindow.Append(&backBtn);

	HaltGui();
	// mainWindow->Append(&titleTxt);
	mainWindow->Append(&fileBrowser);
	mainWindow->Append(&buttonWindow);
	mainWindow->Append(&InsertURL);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

        if(InsertURL.GetState() == STATE_CLICKED)
        {
            URL.SetScroll(SCROLL_NONE);
            OnScreenKeyboard(mainWindow, path, 256);
            URL.SetText(path);
            URL.SetScroll(SCROLL_HORIZONTAL);
        }

		// update file browser based on arrow buttons
		// set MENU_EXIT if A button pressed on a file
		for(i=0; i < FILE_PAGESIZE; i++)
		{
			if(fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();
				// check corresponding browser entry
				if(browserList[browser.selIndex].isdir)
				{
					if(BrowserChangeFolder())
					{
						fileBrowser.ResetState();
						fileBrowser.fileList[0]->SetState(STATE_SELECTED);
						fileBrowser.TriggerUpdate();

						sprintf(fullpath, "%s%s", rootdir, browser.dir+1); // print current path
						sprintf(path, fullpath);
						URL.SetText(fullpath);
					}
					else
					{
						menu = MENU_BROWSE_DEVICE;
						break;
					}
				}
				else
				{
					ShutoffRumble();
					// load file
					sprintf(fullpath, "%s%s/%s", rootdir, browser.dir+1, browserList[browser.selIndex].filename); // print current path
					sprintf(path, fullpath);
					URL.SetText(fullpath);
				}
			}
		}
		if(backBtn.GetState() == STATE_CLICKED)
			menu = MENU_HOME;
	}

	HaltGui();
	// mainWindow->Remove(&titleTxt);
	mainWindow->Remove(&buttonWindow);
	mainWindow->Remove(&fileBrowser);
	mainWindow->Remove(&InsertURL);
	ResumeGui();

	return menu;
}

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
static int MenuAdvanced()
{
    int menu = MENU_NONE;
    int ret, i = 0;
    char version[50];

    bool firstRun = true;
    bool changed = false;

    OptionList options;
    sprintf(options.name[i++], "Render IFrames");
    sprintf(options.name[i++], "Execute Lua scripts");
    sprintf(options.name[i++], "Document.write");
    options.length = i;

    sprintf(version, "WiiBrowser %s", Settings.Revision);
    GuiText titleTxt(version, 28, (GXColor)
    {
        0, 0, 0, 255
    });
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50,45);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);
    GuiImageData btnOutlineOver(button_over_png);

    GuiText backBtnTxt("Go Back", 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage backBtnImg(&btnOutline);
    GuiImage backBtnImgOver(&btnOutlineOver);
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    backBtn.SetPosition(50, -35);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetImageOver(&backBtnImgOver);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetTrigger(trigA);
    backBtn.SetEffectGrow();

    GuiText sendBtnTxt("Send report", 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage sendBtnImg(&btnOutline);
    GuiImage sendBtnImgOver(&btnOutlineOver);
    GuiButton sendBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    sendBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    sendBtn.SetPosition(-50, -35);
    sendBtn.SetLabel(&sendBtnTxt);
    sendBtn.SetImage(&sendBtnImg);
    sendBtn.SetImageOver(&sendBtnImgOver);
    sendBtn.SetSoundOver(&btnSoundOver);
    sendBtn.SetTrigger(trigA);
    sendBtn.SetEffectGrow();

    GuiImageData TextboxImgData(bg_options_png, bg_options_png_size);
    GuiImage TextboxImg(&TextboxImgData);
    TextboxImg.SetScaleX((float)552/TextboxImg.GetWidth());
    TextboxImg.SetScaleY((float)120/TextboxImg.GetHeight());
    GuiButton postComment(TextboxImg.GetRealWidth(), TextboxImg.GetRealHeight());

    GuiText Post("Report bug", 20, (GXColor)
    {
        0, 0, 0, 255
    });
    Post.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Post.SetPosition(0,-25);
    postComment.SetLabel(&Post);

    GuiLongText Content("", 20, (GXColor)
    {
        0, 0, 0, 255
    });
    Content.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Content.SetLinesToDraw(TextboxImg.GetRealHeight()/25);
    Content.SetMaxWidth(TextboxImg.GetRealWidth()-45);
    Content.SetPosition(20, 25);

    postComment.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    postComment.SetPosition(0,250);
    postComment.SetLabel(&Content, 1);
    postComment.SetImage(&TextboxImg);
    postComment.SetSoundOver(&btnSoundOver);
    postComment.SetTrigger(trigA);

    GuiOptionBrowser optionBrowser(552, 93, &options);
    optionBrowser.SetPosition(0, 103);
    optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    optionBrowser.SetCol2Position(185);

    GuiWindow w(screenwidth, screenheight);
    w.Append(&backBtn);
    w.Append(&sendBtn);
    w.Append(&optionBrowser);
    w.Append(&titleTxt);
    w.Append(&postComment);
    w.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 30);

    HaltGui();
    mainWindow->Append(&w);
    ResumeGui();

    char dataToSend[2048];
    memset(dataToSend, 0, sizeof(dataToSend));

    while(menu == MENU_NONE)
    {
        usleep(THREAD_SLEEP);
        ret = optionBrowser.GetClickedOption();
        if(ret >= 0)
            changed = true;

        switch (ret)
        {
        case 0:
            Settings.IFrame = !Settings.IFrame;
            break;
        case 1:
            Settings.ExecLua = !Settings.ExecLua;
            break;
        case 2:
            Settings.DocWrite = !Settings.DocWrite;
            break;
        }

        if(ret >= 0 || firstRun)
        {
            firstRun = false;

            if (Settings.IFrame == 0) sprintf (options.value[0], "Off");
            else if (Settings.IFrame == 1) sprintf (options.value[0], "On");
            if (Settings.ExecLua == 0) sprintf (options.value[1], "Off");
            else if (Settings.ExecLua == 1) sprintf (options.value[1], "On");
            if (Settings.DocWrite == 0) sprintf (options.value[2], "Disabled");
            else if (Settings.DocWrite == 1) sprintf (options.value[2], "Enabled");

            optionBrowser.TriggerUpdate();
        }

        if(sendBtn.GetState() == STATE_CLICKED)
        {
            if(strlen(dataToSend) > 0)
            {
                postcomment(curl_handle, "Anonymous", dataToSend);
                WindowPrompt("Send report", "Thanks, your comment was posted at: http://wiibrowser.altervista.org/", "Ok", NULL);
            }

            memset(dataToSend, 0, sizeof(dataToSend));
            Content.SetText(dataToSend);
            sendBtn.ResetState();
        }

        if(postComment.GetState() == STATE_CLICKED)
        {
            snprintf(dataToSend, 1024, Content.GetText());
            OnScreenKeyboard(mainWindow, dataToSend, 1024);
            Content.SetText(dataToSend);
            postComment.ResetState();
        }

        if(backBtn.GetState() == STATE_CLICKED)
        {
            if(changed)
                Settings.Save(0);
            menu = prevMenu;
        }
    }

    w.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 30);
    while(w.GetEffect() > 0)
        usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&w);
    ResumeGui();
    return menu;
}

static int MenuSettings()
{
    int menu = MENU_NONE;
    int ret, i = 0;
    bool firstRun = true;
    bool changed = false;

    OptionList options;
    sprintf(options.name[i++], "Homepage");
    sprintf(options.name[i++], "Save Folder");
    sprintf(options.name[i++], "Show Tooltips");
    sprintf(options.name[i++], "Show Thumbnails");
    sprintf(options.name[i++], "Autoupdate");
    sprintf(options.name[i++], "Language");
    sprintf(options.name[i++], "Restore Session");
    sprintf(options.name[i++], "UserAgent");
    sprintf(options.name[i++], "Proxy (url:port)");
    options.length = i;

    GuiText titleTxt("Settings", 28, (GXColor)
    {
        0, 0, 0, 255
    });
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50,45);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);
    GuiImageData btnOutlineOver(button_over_png);

    GuiText backBtnTxt("Go Back", 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage backBtnImg(&btnOutline);
    GuiImage backBtnImgOver(&btnOutlineOver);
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    backBtn.SetPosition(50, -35);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetImageOver(&backBtnImgOver);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetTrigger(trigA);
    backBtn.SetEffectGrow();

    GuiText devBtnTxt("Advanced", 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage devBtnImg(&btnOutline);
    GuiImage devBtnImgOver(&btnOutlineOver);
    GuiButton devBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    devBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    devBtn.SetPosition(-50, -35);
    devBtn.SetLabel(&devBtnTxt);
    devBtn.SetImage(&devBtnImg);
    devBtn.SetImageOver(&devBtnImgOver);
    devBtn.SetSoundOver(&btnSoundOver);
    devBtn.SetTrigger(trigA);
    devBtn.SetEffectGrow();

    GuiOptionBrowser optionBrowser(552, 248, &options);
    optionBrowser.SetPosition(0, 103);
    optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    optionBrowser.SetCol2Position(185);

    GuiWindow w(screenwidth, screenheight);
    w.Append(&backBtn);
    w.Append(&devBtn);
    w.Append(&optionBrowser);
    w.Append(&titleTxt);
    w.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 30);

    HaltGui();
    mainWindow->Append(&w);
    ResumeGui();

    while(menu == MENU_NONE)
    {
        usleep(THREAD_SLEEP);
        ret = optionBrowser.GetClickedOption();
        if(ret >= 0)
            changed = true;

        switch (ret)
        {
        case 0:
            OnScreenKeyboard(mainWindow, Settings.Homepage, 256);
            break;

        case 1:
            OnScreenKeyboard(mainWindow, Settings.DefaultFolder, 256);
            Settings.ChangeFolder();
            break;

        case 2:
            Settings.ShowTooltip = !Settings.ShowTooltip;
            break;

        case 3:
            Settings.ShowThumbs = !Settings.ShowThumbs;
            break;

        case 4:
            Settings.Autoupdate++;
             if (Settings.Autoupdate >= CHANNELS)
                Settings.Autoupdate = 0;
            break;

        case 5:
            Settings.Language++;
            if (Settings.Language >= LANG_LENGTH)
                Settings.Language = 0;
            break;

        case 6:
            Settings.Restore = !Settings.Restore;
            break;

        case 7:
            Settings.UserAgent++;
            if (Settings.UserAgent >= MAXAGENTS)
                Settings.UserAgent = 0;
            break;

        case 8:
            OnScreenKeyboard(mainWindow, Settings.Proxy, 256);
            break;
        }

        if(ret >= 0 || firstRun)
        {
            firstRun = false;
            if (Settings.Language > LANG_GERMAN)
                Settings.Language = LANG_JAPANESE;

            snprintf (options.value[0], 256, "%s", Settings.Homepage);
            snprintf (options.value[1], 256, "%s", Settings.DefaultFolder);
            snprintf (options.value[7], 256, "%s", AgentName[Settings.UserAgent]);
            snprintf (options.value[8], 256, "%s", Settings.Proxy);

            if (Settings.ShowTooltip == 0) sprintf (options.value[2], "Hide");
            else if (Settings.ShowTooltip == 1) sprintf (options.value[2], "Show");
            if (Settings.ShowThumbs == 0) sprintf (options.value[3], "Hide");
            else if (Settings.ShowThumbs == 1) sprintf (options.value[3], "Show");

            if (Settings.Autoupdate == 0) sprintf (options.value[4], "Disabled");
            else if (Settings.Autoupdate == 1) sprintf (options.value[4], "Stable");
            else if (Settings.Autoupdate == 2) sprintf (options.value[4], "Nightly");

            if (Settings.Language == LANG_JAPANESE) sprintf (options.value[5], "Japanese");
            else if (Settings.Language == LANG_ENGLISH) sprintf (options.value[5], "English");
            else if (Settings.Language == LANG_GERMAN) sprintf (options.value[5], "German");

            if (Settings.Restore == 0) sprintf (options.value[6], "Start new");
            else if (Settings.Restore == 1) sprintf (options.value[6], "Restore");

            optionBrowser.TriggerUpdate();
        }

        if(backBtn.GetState() == STATE_CLICKED)
        {
            menu = prevMenu;
        }
        if(devBtn.GetState() == STATE_CLICKED)
        {
            menu = MENU_DEVELOPER;
        }
    }

    if(changed)
        Settings.Save(0);

    if(menu == MENU_HOME)
        w.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
    else w.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);

    while(w.GetEffect() > 0)
        usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&w);
    ResumeGui();
    return menu;
}

/****************************************************************************
 * MenuSplash
 *
 * Splash screen function, runs while initiating network functions.
 ***************************************************************************/
static int MenuSplash()
{
    GuiImageData InitData(loading_png);
    GuiImage Init(&InitData);

    Init.SetAlignment(2,5);
    Init.SetScale(0.60);
    Init.SetEffect(EFFECT_FADE, 50);

    HaltGui();
    mainWindow->Append(&Init);
    ResumeGui();

    char myIP[16];
    int conn = 0;
    int menu = Settings.GetStartPage(new_page);

    s32 ip;
    Init.SetEffect(EFFECT_ROTATE, 100, 90);

#ifndef DEBUG
    while ((ip = net_init()) == -EAGAIN)
    {
        usleep(100 * 1000); // 100ms
    }
    if(ip < 0)
    {
        menu = MENU_EXIT;
        conn = NET_ERR;
    }
    if (if_config(myIP, NULL, NULL, true) < 0)
    {
        menu = MENU_EXIT;
        conn = IP_ERR;
    }

#else
    usleep(2000*1000);
#endif
    Init.StopEffect(EFFECT_ROTATE);

    Init.SetEffect(EFFECT_FADE, -50);
    while(Init.GetEffect() > 0) usleep(THREAD_SLEEP);
    usleep(200*1000);

    HaltGui();
    mainWindow->Remove(&Init);
    ResumeGui();

    if (conn == NET_ERR)
        WindowPrompt("Connection failure", "Please check network settings, exiting...", "Ok", NULL);
    if (conn == IP_ERR)
        WindowPrompt("Error reading IP address", "Application exiting. Please check connection settings", "Ok", NULL);

    if(Settings.Autoupdate)
        ResumeUpdateThread();
    return menu;
}

void ShowDownloads()
{
    manager->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(manager);
    mainWindow->ChangeFocus(manager);
    ResumeGui();

    int x, y;

    while(1)
    {
        if (userInput[0].wpad->ir.valid)
        {
            x = userInput[0].wpad->ir.x;
            y = userInput[0].wpad->ir.y;

            if ((userInput[0].wpad->btns_d & WPAD_BUTTON_A) && !manager->IsInside(x, y))
                break;
        }
        usleep(100);
    }

    manager->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(manager->GetEffect() > 0)
        usleep(100);

    HaltGui();
    mainWindow->Remove(manager);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
}

/****************************************************************************
 * MenuHome
 ***************************************************************************/
static int MenuHome()
{
    App->ChangeButtons(HOMEPAGE);
    prevMenu = MENU_HOME;
    strcpy(new_page,prev_page);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData Textbox(keyboard_textbox_png);
    GuiImage TextboxImg(&Textbox);
    GuiButton InsertURL(TextboxImg.GetWidth(), TextboxImg.GetHeight());

    GuiText URL(new_page, 20, (GXColor)
    {
        0, 0, 0, 255
    });
    URL.SetMaxWidth(TextboxImg.GetWidth()-20);
    URL.SetScroll(SCROLL_HORIZONTAL);

    InsertURL.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    InsertURL.SetPosition(0,-50);
    InsertURL.SetLabel(&URL);
    InsertURL.SetImage(&TextboxImg);
    InsertURL.SetSoundOver(&btnSoundOver);
    InsertURL.SetTrigger(trigA);
    InsertURL.SetEffectGrow();

    GuiImageData btnCheck(btn_check_png);
    GuiImageData btnCheckOver(btn_check_over_png);
    GuiImageData btnCanc(btn_cancel_png);
    GuiImageData btnCancOver(btn_cancel_over_png);

    GuiImage btnGoImg(&btnCheck);
    GuiImage btnGoImgOver(&btnCheckOver);
    GuiImage btnExitImg(&btnCanc);
    GuiImage btnExitImgOver(&btnCancOver);

    GuiButton btnGo(btnCheck.GetWidth(), btnCheck.GetHeight());
    btnGo.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    btnGo.SetPosition(-130,50);
    btnGo.SetImage(&btnGoImg);
    btnGo.SetImageOver(&btnGoImgOver);
    btnGo.SetSoundOver(&btnSoundOver);
    btnGo.SetTrigger(trigA);
    btnGo.SetEffectGrow();

    GuiButton btnExit(btnCanc.GetWidth(), btnCanc.GetHeight());
    btnExit.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    btnExit.SetPosition(130,50);
    btnExit.SetImage(&btnExitImg);
    btnExit.SetImageOver(&btnExitImgOver);
    btnExit.SetSoundOver(&btnSoundOver);
    btnExit.SetTrigger(trigA);
    btnExit.SetEffectGrow();

    InsertURL.SetEffect(fadeAnim, 50);
    btnGo.SetEffect(fadeAnim, 50);
    btnExit.SetEffect(fadeAnim, 50);

    if (fadeAnim == EFFECT_FADE)
        App->SetEffect(fadeAnim, 50);
    Splash->SetEffect(fadeAnim, 50);
    Right->SetEffect(EFFECT_FADE, 50);
    Left->Button->SetState(STATE_SELECTED);
    Left->SetEffect(EFFECT_FADE, -50);

    HaltGui();
    mainWindow->Append(Splash);
    mainWindow->Append(App);
    mainWindow->Append(Right);
    mainWindow->Remove(Left);
    mainWindow->Append(&InsertURL);
    mainWindow->Append(&btnGo);
    mainWindow->Append(&btnExit);
    ResumeGui();

    int choice = 0;
    while(!choice)
    {
        if (btnGo.GetState() != STATE_SELECTED)
        {
            if(strlen(new_page) > 0)
                btnGo.SetState(STATE_DEFAULT);
            else btnGo.SetState(STATE_DISABLED);
        }

        ToggleButtons(App);
        usleep(THREAD_SLEEP);

        if(InsertURL.GetState() == STATE_CLICKED ||
                App->btnWWW->GetState() == STATE_CLICKED)
        {
            URL.SetScroll(SCROLL_NONE);
            OnScreenKeyboard(mainWindow, new_page, MAXLEN);
            strcpy(prev_page,new_page);
            URL.SetText(new_page);
            URL.SetScroll(SCROLL_HORIZONTAL);
            App->btnWWW->ResetState();
        }

        else if(App->btnSett->GetState() == STATE_CLICKED)
        {
            fadeAnim = EFFECT_FADE;
            choice = MENU_SETTINGS;
            App->btnSett->ResetState();
        }

        else if(App->btnSave->GetState() == STATE_CLICKED)
        {
            ShowDownloads();
            App->btnSave->ResetState();
        }

        else if(App->btnHome->GetState() == STATE_CLICKED)
        {
            strcpy(new_page,Settings.Homepage);
            strcpy(prev_page,new_page);
            URL.SetText(new_page);
            App->btnHome->ResetState();
        }

        if (App->btnBack->GetState() == STATE_CLICKED)
        {
            App->btnBack->ResetState();
            if (history->prec)
            {
                history=history->prec;
                strncpy(new_page,history->url.c_str(),512);
                strcpy(prev_page,new_page);
                URL.SetText(new_page);
            }
        }

        if (App->btnForward->GetState() == STATE_CLICKED)
        {
            App->btnForward->ResetState();
            if (history->prox)
            {
                history=history->prox;
                strncpy(new_page,history->url.c_str(),512);
                strcpy(prev_page,new_page);
                URL.SetText(new_page);
            }
        }

        else if(btnGo.GetState() == STATE_CLICKED)
        {
            fadeAnim = EFFECT_FADE;
            choice = MENU_BROWSE;
        }

        else if(Right->Button->GetState() == STATE_CLICKED ||
                userInput[0].wpad->btns_d & WPAD_BUTTON_PLUS)
        {
            fadeAnim = EFFECT_SLIDE_OUT | EFFECT_SLIDE_LEFT;
            choice = MENU_FAVORITES;
        }

        else if(btnExit.GetState() == STATE_CLICKED)
        {
            int close = WindowPrompt("Homepage", "Do you want to exit?", "Ok", "Cancel");
            if (close)
                choice = MENU_EXIT;
        }
    }

    btnGo.SetEffect(fadeAnim, -50);
    btnExit.SetEffect(fadeAnim, -50);
    Splash->SetEffect(fadeAnim, -50);
    InsertURL.SetEffect(fadeAnim, -50);
    while(InsertURL.GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(Splash);
    mainWindow->Remove(&InsertURL);
    mainWindow->Remove(&btnGo);
    mainWindow->Remove(&btnExit);
    ResumeGui();

    if(choice != MENU_FAVORITES)
    {
        App->SetEffect(EFFECT_SLIDE_OUT | EFFECT_SLIDE_BOTTOM, 50);
        Right->SetEffect(EFFECT_FADE, -50);
        while(App->GetEffect() > 0) usleep(THREAD_SLEEP);

        HaltGui();
        mainWindow->Remove(App);
        mainWindow->Remove(Right);
        ResumeGui();
    }

    return choice;
}

/****************************************************************************
 * MenuBrowse
 ***************************************************************************/
bool CancelDownload()
{
    if (!actionButton)
        return false;

    return (actionButton->GetState() == STATE_CLICKED);
}

char *omniBox()
{
    char *url = (char*) malloc (sizeof(new_page)+100);
    char *encode = url_encode(new_page);

    sprintf(url, "http://www.google.com/search?hl=en&source=hp&biw=&bih=&q=%s&btnG=Google+Search&gbv=1", encode);
    free(encode);
    return url;
}

static int MenuBrowse()
{
    char *nurl = NULL;
    char *url = NULL;
    FILE *hfile = NULL;

    url = (char*) malloc (sizeof(new_page)+10);
    bzero(url, sizeof(new_page)+10);
    bool searchWord = true;

    if (strncmp(new_page,"http",4))
        strcpy(url,"http://");
    strcat(url,new_page);

    GuiWindow promptWindow(448,288);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiImageData dialogBox(dialogue_box_png, dialogue_box_png_size);
    GuiImage dialogBoxImg(&dialogBox);

    GuiText title("WiiBrowser", 26, (GXColor)
    {
        0, 0, 0, 255
    });
    title.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    title.SetPosition(0,40);

    GuiText staticTxt("Loading...please wait", 20, (GXColor)
    {
        0, 0, 0, 255
    });
    staticTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    staticTxt.SetPosition(0,-20);
    staticTxt.SetWrap(true, 400);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);
    GuiImageData btnOutlineOver(button_over_png);
    GuiImage btnImg(&btnOutline);
    GuiImage btnImgOver(&btnOutlineOver);

    GuiText btnTxt("Cancel", 22, (GXColor)
    {
        0, 0, 0, 255
    });

    actionButton = new GuiButton(btnOutline.GetWidth(), btnOutline.GetHeight());
    actionButton->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    actionButton->SetPosition(0, -25);

    actionButton->SetLabel(&btnTxt);
    actionButton->SetImage(&btnImg);
    actionButton->SetImageOver(&btnImgOver);
    actionButton->SetSoundOver(&btnSoundOver);
    actionButton->SetTrigger(trigA);
    actionButton->SetEffectGrow();

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&title);

    GuiWindow childWindow(screenwidth, screenheight);
    childWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    childWindow.SetPosition(0,0);

    msgTxt = new GuiText("Loading...please wait", 20, (GXColor)
    {
        0, 0, 0, 255
    });

    msgTxt->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt->SetPosition(0,-20);
    msgTxt->SetWrap(true, 400);

jump:
    decode_html_entities_utf8(url, NULL);
    save_mem(url);
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    if(hfile)
    {
        promptWindow.Remove(&staticTxt);
        promptWindow.Append(actionButton);
        promptWindow.Append(msgTxt);
    }
    else promptWindow.Append(&staticTxt);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

#ifdef TIME
    u64 now, prev;
    prev = gettime();
#endif

    struct block HTML;
    HTML = downloadfile(curl_handle, url, hfile);

#ifdef DEBUG
    FILE *pFile = fopen ("sd:/page.htm", "rb");
    fseek (pFile, 0, SEEK_END);
    int size = ftell(pFile);
    rewind (pFile);

    HTML.data = (char*) malloc (sizeof(char)*size);
    if (HTML.data == NULL) exit (2);
    fread (HTML.data, 1, size, pFile);

    HTML.size = size;
    strcpy(HTML.type, "text/html");
    fclose(pFile);
#endif

#ifdef TIME
    do
    {
        now = gettime();
        usleep(100);
    }
    while (diff_usec(prev, now) < 500*1000);
#endif

    usleep(500*1000);
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(promptWindow.GetEffect() > 0)
        usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    delete(msgTxt);
    delete(actionButton);

    msgTxt = NULL;
    actionButton = NULL;

    if(HTML.size == -1)
    {
        if(performDownload(&hfile, url, &HTML))
            AddDownload(curl_multi, url, hfile);

        free(url);
        return MENU_HOME;
    }

    if (CURLE_OK == curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &nurl))
    {
        url = (char*) realloc (url,strlen(nurl)+1);
        strcpy(url, nurl);
    }

    if (!HTML.size)
    {
        free(url);

        if (!searchWord)
        {
            WindowPrompt("WiiBrowser", "Failed", "Ok", NULL);
            return MENU_HOME;
        }

        searchWord = false;
        url = omniBox();
        goto jump;
    }

    if (!history || strcmp(history->url.c_str(),url))
    {
        history=InsUrl(history,url);
        DumpList(history);
    }
    usleep(500*1000);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&childWindow);
    mainWindow->ChangeFocus(&childWindow);
    ResumeGui();

    string link;
    if (!hfile)
        link = DisplayHTML(&HTML, mainWindow, &childWindow, url);

    bzero(new_page, sizeof(new_page));
    free(HTML.data);

    HaltGui();
    mainWindow->Remove(&childWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    if (link.length()>0)
    {
        link = parseUrl(link, url);
        url = (char*) realloc (url,strlen(link.c_str())+1);
        strcpy(url,(char*)link.c_str());
        goto jump;
    }
    return MENU_HOME;
}

bool Held(GuiFavorite *Block)
{
    for (int i=0; i<N; i++)
    {
        if(Block[i].Block->GetState() == STATE_HELD)
            return true;
    }
    return false;
}

void SwapPos(GuiFavorite *Block, int i, int j)
{
    int xtemp = Block[i].xpos;
    int ytemp = Block[i].ypos;

    Block[i].SetInit(Block[j].xpos, Block[j].ypos);
    Block[i].SetPosition(Block[j].xpos, Block[j].ypos);
    Block[j].SetInit(xtemp, ytemp);
    Block[j].SetPosition(xtemp, ytemp);
}

void SwapImage(GuiFavorite *Block, int i, int j)
{
    u8 *itemp = Settings.Thumbnails[i];
    Settings.Thumbnails[i] = Settings.Thumbnails[j];
    Settings.Thumbnails[j] = itemp;
}

void SwapUrls(int i, int j)
{
    char utemp[512];
    strcpy(utemp, Settings.GetUrl(i));
    strcpy(Settings.Favorites[i], Settings.Favorites[j]);
    strcpy(Settings.Favorites[j], utemp);
}

int findEmpty()
{
    for(int i = 0; i<N; i++)
    {
        if(strlen(Settings.GetUrl(i)) == 0)
            return i;
    }
    return -1;
}

/****************************************************************************
 * MenuFavorites
 ***************************************************************************/
static int MenuFavorites()
{
    bool editing = false;
    App->ChangeButtons(editing ? EDITING : FAVORITES);

    strcpy(new_page,prev_page);
    prevMenu = MENU_HOME;

    fadeAnim = EFFECT_SLIDE_IN | EFFECT_SLIDE_LEFT;
    Right->Button->SetState(STATE_SELECTED);
    Right->SetEffect(EFFECT_FADE, -50);

    GuiFavorite Block[N];
    prevMenu = MENU_FAVORITES;

    for (int i = 0, xpos = 40, ypos = 40; i < N; i++)
    {
        Block[i].SetInit(xpos, ypos);
        Block[i].SetEffect(EFFECT_SLIDE_IN | EFFECT_SLIDE_RIGHT, 50);
        Block[i].SetPosition(xpos,ypos);
        Block[i].Block->SetUpdateCallback(DragCallback);
        Block[i].Label->SetText(Settings.GetUrl(i));

        if(Settings.ShowThumbs && Settings.Thumbnails[i])
        {
            Block[i].Thumb->SetImage(Settings.Thumbnails[i], vmode->fbWidth, vmode->viHeight);
            Block[i].Block->SetIcon(Block[i].Thumb);
            Block[i].Block->SetLabelOver(Block[i].Label);
        }
        else Block[i].Block->SetLabel(Block[i].Label);

        xpos += Block[i].GetDataWidth()+35;
        if(xpos > screenwidth-90)
        {
            ypos += Block[i].GetDataHeight()+15;
            xpos = 40;
        }
    }

    HaltGui();
    for (int i = 0; i < N; i++)
        mainWindow->Append(&Block[i]);
    ResumeGui();

    App->btnBack->SetState(STATE_DISABLED);
    App->btnForward->SetState(STATE_DISABLED);

    Left->SetEffect(EFFECT_FADE, 50);
    HaltGui();
    mainWindow->Append(App);
    mainWindow->Append(Left);
    mainWindow->Remove(Right);
    ResumeGui();

    int i, choice = 0;
    while(!choice)
    {
        usleep(THREAD_SLEEP);

        if(App->btnSett->GetState() == STATE_CLICKED)
        {
            editing = !editing;
            for (i = 0; i<N; i++)
                Block[i].SetEditing(editing);

            App->ChangeButtons(editing ? EDITING : FAVORITES);
            App->btnSett->ResetState();
        }

        else if(App->btnHome->GetState() == STATE_CLICKED)
        {
            strcpy(new_page,Settings.Homepage);
            strcpy(prev_page,new_page);
            App->btnHome->ResetState();
        }

        else if(App->btnSave->GetState() == STATE_CLICKED)
        {
            App->btnSave->ResetState();

            if((i = findEmpty()) >= 0)
            {
                strcpy(Settings.Favorites[i],prev_page);
                Block[i].Label->SetText(Settings.GetUrl(i));
            }
        }

        else if(Left->Button->GetState() == STATE_CLICKED ||
                App->btnWWW->GetState() == STATE_CLICKED || userInput[0].wpad->btns_d & WPAD_BUTTON_MINUS)
        {
            App->btnWWW->ResetState();
            choice = MENU_HOME;
        }

        for (i = 0; i < N; i++)
        {
            if(Block[i].Block->GetState() == STATE_SELECTED)
                Block[i].Label->SetScroll(SCROLL_HORIZONTAL);
            else Block[i].Label->SetScroll(SCROLL_NONE);

            if(Block[i].Block->GetState() == STATE_CLICKED &&
                    Left->Button->GetState() == STATE_DEFAULT && !editing)
            {
                strcpy(new_page,Settings.GetUrl(i));
                strcpy(prev_page,new_page);
                Block[i].Block->ResetState();
            }

            else if(Block[i].Remove->GetState() == STATE_CLICKED)
            {
                Block[i].Block->SetIcon(NULL);
                Settings.Remove(i);
                Block[i].Label->SetText(Settings.GetUrl(i));
                Block[i].Remove->ResetState();
            }

            if(editing && !Held(Block))
            {
                for (int j = 0; j < N; j++)
                {
                    if (j == i)
                        continue;

                    if(userInput[0].wpad->ir.valid && Block[i].Block->IsInside(userInput[0].wpad->ir.x, userInput[0].wpad->ir.y) &&
                            Block[j].Block->IsInside(userInput[0].wpad->ir.x, userInput[0].wpad->ir.y))
                    {
                        SwapPos(Block, i, j);
                        SwapImage(Block, i, j);
                        SwapUrls(i, j);
                    }
                }
            }
        }
    }

    if(editing)
    {
        for (i = 0; i < N; i++)
            Block[i].Remove->SetEffect(EFFECT_FADE, -50);
        while(Block[N-1].Remove->GetEffect() > 0) usleep(THREAD_SLEEP);
    }

    for (i = 0; i < N; i++)
        Block[i].SetEffect(EFFECT_SLIDE_OUT | EFFECT_SLIDE_RIGHT, 50);
    while(Block[N-1].GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    for (i = 0; i < N; i++)
        mainWindow->Remove(&Block[i]);
    ResumeGui();

    if(choice != MENU_HOME)
    {
        App->SetEffect(EFFECT_SLIDE_OUT | EFFECT_SLIDE_BOTTOM, 50);
        Left->SetEffect(EFFECT_FADE, -50);
        while(App->GetEffect() > 0) usleep(THREAD_SLEEP);

        HaltGui();
        mainWindow->Remove(App);
        mainWindow->Remove(Left);
        ResumeGui();
    }

    return choice;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
void LoadSession()
{
    char cookies[256];
    sprintf(cookies, "%s/cookie.csv", Settings.AppPath);
    history = LoadList();

    /* setup cookies engine */
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, cookies);
    curl_easy_setopt(curl_handle, CURLOPT_COOKIESESSION, 1L);
}

void DeleteSession()
{
    char path[256];
    history = InitHistory();

    sprintf(path, "%s/cookie.csv", Settings.AppPath);
    remove(path);
    sprintf(path, "%s/history.txt", Settings.AppPath);
    remove(path);

    /* setup cookies engine */
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, path);
}

void Init()
{
    if(curl_global_init(CURL_GLOBAL_ALL))
        ExitRequested = 1;

    curl_handle = curl_easy_init();
    memset(prev_page, 0, sizeof(prev_page));

    if (!Settings.Restore && Settings.CleanExit)
    {
        Settings.Save(0);
        DeleteSession();
    }
    else LoadSession();

    #ifdef MPLAYER
    if(!InitMPlayer())
    {
        ExitRequested = true;
        return;
    }
    #endif

    SetupGui();
    remove("debug.txt");
}

void Cleanup()
{
    delete trigA;
    delete bgImg;

    delete SplashImage;
    delete Splash;
    delete mainWindow;

    delete Right;
    delete Left;
    delete App;

    for (int i = 0; i < 4; i++)
    {
        delete pointerData[i];
        delete pointerGrabData[i];
    }

    char cookies[30];
    sprintf(cookies, "%s/cookie.csv", Settings.AppPath);

    /* setup cookies engine */
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR, cookies);

    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    mainWindow = NULL;
}

void MainMenu(int menu)
{
    int currentMenu = menu;
    Init();
    if(!Settings.CleanExit)
        WindowPrompt("Oops.. this is embarrassing", "The app didn't close correctly, the previous session has been automatically restored", "OK", NULL);

    while(currentMenu != MENU_EXIT)
    {
        switch (currentMenu)
        {
        case MENU_SPLASH:
            currentMenu = MenuSplash();
            break;
        case MENU_HOME:
            currentMenu = MenuHome();
            break;
        case MENU_SETTINGS:
            currentMenu = MenuSettings();
            break;
        case MENU_DEVELOPER:
            currentMenu = MenuAdvanced();
            break;
        case MENU_FAVORITES:
            currentMenu = MenuFavorites();
            break;
        case MENU_BROWSE:
            currentMenu = MenuBrowse();
            break;
        case MENU_BROWSE_DEVICE:
            currentMenu = MenuBrowseDevice();
            break;
        default: // unrecognized menu
            currentMenu = MenuHome();
            break;
        }
    }

    ExitRequested = 1;
    ResumeGui();
}
