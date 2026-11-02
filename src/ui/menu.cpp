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
#include <atomic>
#include <wiiuse/wpad.h>

#include "httplib.h"
#include "liste.h"
#include "main.h"
#include "input.h"
#include "transfer.h"
#include "networkop.h"

#include "filebrowser.h"
#include "fileop.h"
#include "filelist.h"

#include "FreeTypeGX.h"
#include "TextEditor.h"
#include "config.h"
#include "url_helper.h"
#include "safety.h"
#include "file_io.h"
#include "gui_lock.h"
#include "devmount.h"

extern "C" {
#include "url.h"
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
CURLSH *curl_share;

mutex_t m_mutex;
History history;

char new_page[wbl::MaxUrl + 1];
static char prev_page[wbl::MaxUrl + 1];
static char Message[MAXLEN];

static int fadeAnim;
static int prevMenu;
static int currPage;

static GuiSwitch * Right = NULL;
static GuiSwitch * Left = NULL;
GuiToolbar * App = NULL;
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

static volatile bool guiHalt = true;
static bool toggleManager = false;

static volatile int updateThreadHalt = 0;
static volatile int loadThreadHalt = 1;
static mutex_t guiMutex;
static bool guiMutexReady = false;
static std::atomic<lwp_t> guiOwner(LWP_THREAD_NULL);
static unsigned guiDepth = 0;
static std::atomic<bool> guiReady(false);
static std::atomic<bool> guiStop(false);
static bool curlReady = false;

// Non-blocking font error overlay (Q1/Q2: explicit but non-blocking, uses fallback fonts)
static bool fontErrorOverlayShown = false;
static u32 fontErrorOverlayStart = 0;
#define FONT_ERROR_OVERLAY_DURATION_MS 10000 // 10s auto-dismiss, but keeps logging

extern bool g_fontLoadFailed;
extern "C" const char* GetFontErrorMsg();
extern "C" bool IsFontLoadFailed();

static void DrawFontErrorOverlay()
{
    if(!IsFontLoadFailed()) return;

    // First time: log via OSReport already done in FreeTypeGX ctor
    if(!fontErrorOverlayShown)
    {
        fontErrorOverlayShown = true;
        fontErrorOverlayStart = ticks_to_millisecs(gettime());
        // Also ensure visible in console for Dolphin
        printf("Font overlay: %s\n", GetFontErrorMsg());
    }

    // Auto-dismiss after duration but keep g_fontLoadFailed for logging
    u32 now = ticks_to_millisecs(gettime());
    if(now - fontErrorOverlayStart > FONT_ERROR_OVERLAY_DURATION_MS)
        return;

    // Draw semi-transparent banner at top (non-blocking, does not halt GUI)
    // Use Menu_DrawRectangle which works without freetype
    Menu_DrawRectangle(0, 0, screenwidth, 28, (GXColor){0xCC, 0x00, 0x00, 0xDD}, 1);
    // Draw border
    Menu_DrawRectangle(0, 28, screenwidth, 2, (GXColor){0xFF, 0xFF, 0xFF, 0xFF}, 1);

    // Try to render text via fallback fontSystem (bold/italic) if available
    // Do not use GuiText to avoid recursion; use raw FreeTypeGX draw if valid
    FreeTypeGX* fallback = nullptr;
    for(int sz=14; sz<=24; sz++)
    {
        extern FreeTypeGX *fontSystem[];
        if(fontSystem[sz] && fontSystem[sz]->isValid()) { fallback = fontSystem[sz]; break; }
    }
    if(!fallback)
    {
        for(int i=0;i<=MAX_FONT_SIZE;i++)
        {
            extern FreeTypeGX *fontSystem[];
            if(fontSystem[i] && fontSystem[i]->isValid()) { fallback = fontSystem[i]; break; }
        }
    }
    if(fallback)
    {
        const char* msg = GetFontErrorMsg();
        char display[220];
        snprintf(display, sizeof(display), "Font issue: %s (using fallback)", msg);
        wchar_t* wmsg = charToWideChar(display);
        if(wmsg)
        {
            // Centered, white text, shadow via double draw not needed
            fallback->drawText(10, 18, wmsg, (GXColor){0xFF,0xFF,0xFF,0xFF});
            delete[] wmsg;
        }
    }
    // else: rectangle alone indicates error even without freetype (all fonts failed)
}

using namespace std;

// Sprint 3.3: URL helpers extracted to src/utils/url_helper.*
// (legacy getHost/adjustUrl wrappers now in url_helper.cpp)

/****************************************************************************
 * Perform downloads
 ***************************************************************************/
bool performDownload(file *hfile, char *url, struct block *html)
{
    int choice;
    bool select = false;

    char path[260];
    memset(path, 0, sizeof(path));

    choice = WindowPrompt("Download", "Do you want to download the file?", "Yes", "No");
    if (choice)
    {
#ifdef WIIFLOW
        select = AutoDownloader(path, sizeof(path));
        if(!select)
#endif
        select = GuiBrowser(NULL, mainWindow, path, sizeof(path), "Download!");
    }

    if (select)
    {
        select = downloadPath(html, url, path, sizeof(path));
        hfile->file = NULL;
        hfile->overwrite = select && CheckFile(path);
        if (hfile->overwrite)
            select = WindowPrompt("Replace file?", path, "Replace", "Cancel") != 0;
        if (select) select = wbl::copyString(hfile->name, sizeof(hfile->name), path);
        if (!select) WindowPrompt("Download cancelled", "Invalid or oversized destination; no file was changed.", "OK", NULL);
    }

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
    if (!guiMutexReady) return;
    if (guiOwner == LWP_GetSelf() && guiDepth) {
        if (--guiDepth == 0) guiOwner = LWP_THREAD_NULL;
        LWP_MutexUnlock(guiMutex);
    }
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
extern "C" bool HaltGuiEx(int timeoutMs)
{
    if (!guiMutexReady) return true;
    for (int elapsed = 0; elapsed <= timeoutMs; ++elapsed) {
        if (LWP_MutexTryLock(guiMutex) == 0) {
            guiOwner = LWP_GetSelf();
            ++guiDepth;
            return true;
        }
        usleep(1000);
    }
    return false;
}

extern "C" void HaltGui()
{
    if (!guiMutexReady) return;
    LWP_MutexLock(guiMutex);
    guiOwner = LWP_GetSelf();
    ++guiDepth;
}

static void ResumeGuiEx()
{
    ResumeGui();
}

extern "C" void DoMPlayerGuiDraw()
{
    mainWindow->Draw();
    mainWindow->DrawTooltip();

    DoRumble(0);
    mainWindow->Update(&userInput[0]);
}

void UpdatePointer()
{
    if(userInput[0].wpad && userInput[0].wpad->ir.valid)
        Menu_DrawImg(userInput[0].wpad->ir.x-48, userInput[0].wpad->ir.y-48,
                     96, 96, pointer[0]->GetImage(), userInput[0].wpad->ir.angle, 1, 1, 255, GX_TF_RGBA8);
}

/****************************************************************************
 * LoadYouTubeFile
 *
 * Creates youtube video url
 ***************************************************************************/
#ifdef MPLAYER
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
    msgTxt->SetPosition(0, 73);

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
    snprintf(Message, sizeof(Message), "%s", msg ? msg : "");

    loadThreadHalt = 0;
    LWP_ResumeThread (loadthread);
}

void SetMessage (const char *msg)
{
    if (!msgTxt)
        return;

    snprintf(Message, sizeof(Message), "%s", msg ? msg : "");

    msgTxt->SetText(msg);
}

void CancelAction()
{
    if (loadthread == LWP_THREAD_NULL) return;
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
    msgLongTxt.SetLinesToDraw(8);
    msgLongTxt.SetMaxWidth(400);

    GuiText btn1Txt(btn1Label, 22, (GXColor)
    {
        0, 0, 0, 255
    });
    GuiImage btn1Img(&btnOutline);
    GuiImage btn1ImgOver(&btnOutlineOver);
    GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

    if(msg)
        msgLongTxt.SetPosition(20,130);
    else msgLongTxt.SetPosition(20,90);

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

        if(App_Exiting())
            choice = 0;
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(!App_Exiting() && promptWindow.GetEffect() > 0)
        usleep(THREAD_SLEEP);

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
        snprintf(message, sizeof(message), "%s", upd.changelog);
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
        App->SaveTooltip->SetTimeout("Updating", 2);

        if(downloadUpdate(upd.appversion))
            ExitRequested = true;
    }
    return NULL;
}

void ResumeUpdateThread()
{
#if !WBL_ENABLE_UPDATER
    return;
#endif
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
    int i;

    while(!guiStop)
    {
        if(!guiReady)
        {
            usleep(1000);
        }
        else
        {
            GuiScopedLock frameLock;
            if (!mainWindow) continue;
            UpdatePads();
            if (FrameTimer % 30 == 0) MountManager_Poll();
            mainWindow->Draw();
            mainWindow->DrawTooltip();
            DrawFontErrorOverlay();

#ifdef HW_RVL
            for(i=3; i >= 0; i--) // so that player 1's cursor appears on top!
            {
                if(userInput[i].wpad && userInput[i].wpad->ir.valid)
                    Menu_DrawImg(userInput[i].wpad->ir.x-48, userInput[i].wpad->ir.y-48,
                                 96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255, GX_TF_RGBA8);
                DoRumble(i);
            }
#endif

            for(i=0; i < 4; i++)
            {
                mainWindow->Update(&userInput[i]);
                if(userInput[i].wpad && (userInput[i].wpad->btns_d & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)))
                    ExitRequested = true; // exit program
                if (userInput[i].pad.btns_d & PAD_BUTTON_MENU) ExitRequested = true;
            }

            if(HWButton)
                ExitRequested = true; // exit program
            Menu_Render();

            if (ExitRequested || HWButton) ExitAccepted = true;
        }
        usleep(1000); // Let lower-priority menu/worker mutex owners run.
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
    if (LWP_MutexInit(&guiMutex, true) != 0) { ExitAccepted = true; return; }
    guiMutexReady = true;
    guiStop = false;
    if(LWP_CreateThread(&guithread, UpdateGUI, NULL, guistack, GUITH_STACK, 70) != 0)
        guithread = LWP_THREAD_NULL;
#if WBL_ENABLE_UPDATER
    if(LWP_CreateThread(&updatethread, UpdateThread, NULL, updatestack, GUITH_STACK, 70) != 0)
        updatethread = LWP_THREAD_NULL;
#endif
    if(LWP_CreateThread(&loadthread, LoadingThread, NULL, loadstack, GUITH_STACK, 70) != 0)
        loadthread = LWP_THREAD_NULL;
    if(LWP_CreateThread(&downloadthread, DownloadThread, NULL, downloadstack, GUITH_STACK, 70) != 0)
        downloadthread = LWP_THREAD_NULL;
    if (guithread == LWP_THREAD_NULL || loadthread == LWP_THREAD_NULL || downloadthread == LWP_THREAD_NULL)
        ExitAccepted = true;
}

void
StopGUIThreads()
{
    if(downloadthread != LWP_THREAD_NULL)
    {
        StopDownload();
        LWP_JoinThread(downloadthread, NULL);
        downloadthread = LWP_THREAD_NULL;
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
        guiStop = true;
        if (LWP_GetSelf() != guithread) LWP_JoinThread(guithread, NULL);
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
int OnScreenKeyboard(GuiWindow *keyboardWindow, char *var, u16 maxlen)
{
    int save = -1;
    if(!keyboardWindow)
        keyboardWindow = mainWindow;

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
        else if(ExitRequested || HWButton)
            save = 0;
        else
        {
            // Allow B button / Home as cancel, and handle emulated IR edge: check both WPAD and GC
            for(int i=0;i<4;i++)
            {
                if(userInput[i].wpad && (userInput[i].wpad->btns_d & (WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B | WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)))
                {
                    save = 0;
                    break;
                }
                if(PAD_ButtonsDown(i) & (PAD_BUTTON_B | PAD_BUTTON_MENU))
                {
                    save = 0;
                    break;
                }
            }
        }
    }

    if(save)
    {
        GuiScopedLock snapshot;
        snprintf(var, maxlen, "%s", keyboard.kbtextstr);
    }

    keyboard.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
    int effectTries = 0;
    while(keyboard.GetEffect() > 0 && effectTries < 300) // ~30ms*50 = cap 1.5s to avoid infinite slide
    {
        usleep(THREAD_SLEEP);
        effectTries++;
        if(ExitRequested || HWButton) break;
    }

    HaltGui();
    keyboardWindow->Remove(&keyboard);
    keyboardWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    return save;
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
    currPage = 0;

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

    // Ensure critical font sizes are primed before any GuiText is created
    // on the download/GUI threads (prevents ISI 0x4E800020 from corrupted FT_Face)
    {
        extern FreeTypeGX *fontSystem[];
        const int criticalSizes[] = {10, 12, 14, 16, 18, 20, 24, 28};
        for(size_t i = 0; i < sizeof(criticalSizes)/sizeof(criticalSizes[0]); ++i)
        {
            int sz = criticalSizes[i];
            if(sz > MAX_FONT_SIZE) continue;
            if(!fontSystem[sz] || !fontSystem[sz]->isValid())
            {
                if(fontSystem[sz]) { delete fontSystem[sz]; fontSystem[sz] = nullptr; }
                fontSystem[sz] = new (std::nothrow) FreeTypeGX(sz);
                if(!fontSystem[sz] || !fontSystem[sz]->isValid())
                {
                    printf("SetupGui: Failed to prime font size %d\n", sz);
                    // Non-fatal: GuiText will gracefully fall back to width 0
                }
            }
        }
    }

    if (!InitTransfers()) { ExitAccepted = true; return; }
    LWP_ResumeThread(downloadthread);
    guiReady = true;
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
	prevMenu = MENU_BROWSE_DEVICE;

	// populate initial directory listing
	if(BrowseDevice() <= 0)
	{
		int choice = WindowPrompt(
		"Error",
		"Unable to display files on selected load device",
		"Retry",
		"Cancel");

		if(choice)
			return MENU_BROWSE_DEVICE;
		else
			return MENU_HOME;
	}

	int menu = MENU_NONE;

    snprintf(title, sizeof(title), "%s", "Browse files");
    memset(path, 0, sizeof(path));

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

	while(menu == MENU_NONE && !App_Exiting())
	{
		usleep(THREAD_SLEEP);

        if(InsertURL.GetState() == STATE_CLICKED)
        {
            URL.SetScroll(SCROLL_NONE);
            InsertURL.ResetState();
            OnScreenKeyboard(mainWindow, path, 256);
            URL.SetText(path);
            URL.SetScroll(SCROLL_HORIZONTAL);
            InsertURL.ResetState();
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
                        snprintf(path, sizeof(path), "%s", fullpath);
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
                    snprintf(path, sizeof(path), "%s", fullpath);
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
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "IFrames (unavailable)");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Lua (unavailable)");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Script rewriting (unavailable)");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Proxy (url:port)");
    options.length = i;

    snprintf(version, sizeof(version), "WiiBrowser %s", Settings.Revision);
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

    while(menu == MENU_NONE && !App_Exiting())
    {
        usleep(THREAD_SLEEP);
        ret = optionBrowser.GetClickedOption();
        if(ret >= 0)
            changed = true;

        switch (ret)
        {
        case 0:
            Settings.IFrame = false;
            break;
        case 1:
            Settings.ExecLua = false;
            break;
        case 2:
            Settings.DocWrite = false;
            break;
        case 3:
            OnScreenKeyboard(mainWindow, Settings.Proxy, 256);
            break;
        }

        if(ret >= 0 || firstRun)
        {
            firstRun = false;
            snprintf (options.value[3], 256, "%s", Settings.Proxy);

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
                WindowPrompt("Report an issue", "Use the GitHub issue link in README. No report was transmitted.", "OK", NULL);
            }

            memset(dataToSend, 0, sizeof(dataToSend));
            Content.SetText(dataToSend);
            sendBtn.ResetState();
        }

        if(postComment.GetState() == STATE_CLICKED)
        {
            snprintf(dataToSend, sizeof(dataToSend), "%s", Content.GetText());
            OnScreenKeyboard(mainWindow, dataToSend, 1024);
            Content.SetText(dataToSend);
            postComment.ResetState();
        }

        if(backBtn.GetState() == STATE_CLICKED)
        {
            if(changed && !Settings.Save(0))
                WindowPrompt("Settings not saved", "Check storage and preserved .new/.bak recovery files.", "OK", NULL);
            menu = MENU_HOME;
        }
    }

    prevMenu = MENU_DEVELOPER;
    w.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 30);
    while(!App_Exiting() && w.GetEffect() > 0)
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
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Homepage");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Download Folder");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Show Tooltips");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Show Thumbnails");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Autoupdate");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Language");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Restore Session");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Unzip Files");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "Click Sound");
    snprintf(options.name[i++], sizeof(options.name[0]), "%s", "User Agent");
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

    while(menu == MENU_NONE && !App_Exiting())
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
            Settings.Autoupdate = DISABLED;
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
            Settings.ZipFile = NEVER;
            break;

        case 8:
            Settings.MuteSound = !Settings.MuteSound;
            break;

        case 9:
            Settings.UserAgent++;
            if (Settings.UserAgent >= MAXAGENTS)
                Settings.UserAgent = 0;
            break;
        }

        if(ret >= 0 || firstRun)
        {
            firstRun = false;
            if (Settings.Language > LANG_GERMAN)
                Settings.Language = LANG_JAPANESE;

            snprintf (options.value[0], 256, "%s", Settings.Homepage);
            snprintf (options.value[1], 256, "%s/", Settings.UserFolder);
            snprintf (options.value[9], 256, "%s", AgentName[Settings.UserAgent]);

            if (Settings.ShowTooltip == 0) snprintf(options.value[2], sizeof(options.value[2]), "%s", "Hide");
            else if (Settings.ShowTooltip == 1) snprintf(options.value[2], sizeof(options.value[2]), "%s", "Show");
            if (Settings.ShowThumbs == 0) snprintf(options.value[3], sizeof(options.value[3]), "%s", "Hide");
            else if (Settings.ShowThumbs == 1) snprintf(options.value[3], sizeof(options.value[3]), "%s", "Show");

            if (Settings.Autoupdate == 0) snprintf(options.value[4], sizeof(options.value[4]), "%s", "Disabled");
            else if (Settings.Autoupdate == 1) snprintf(options.value[4], sizeof(options.value[4]), "%s", "Stable");
            else if (Settings.Autoupdate == 2) snprintf(options.value[4], sizeof(options.value[4]), "%s", "Nightly");

            if (Settings.Language == LANG_JAPANESE) snprintf(options.value[5], sizeof(options.value[5]), "%s", "Japanese");
            else if (Settings.Language == LANG_ENGLISH) snprintf(options.value[5], sizeof(options.value[5]), "%s", "English");
            else if (Settings.Language == LANG_GERMAN) snprintf(options.value[5], sizeof(options.value[5]), "%s", "German");

            if (Settings.Restore == 0) snprintf(options.value[6], sizeof(options.value[6]), "%s", "Start new");
            else if (Settings.Restore == 1) snprintf(options.value[6], sizeof(options.value[6]), "%s", "Restore");
            if (Settings.MuteSound == 0) snprintf(options.value[8], sizeof(options.value[8]), "%s", "On");
            else if (Settings.MuteSound == 1) snprintf(options.value[8], sizeof(options.value[8]), "%s", "Off");

            if (Settings.ZipFile == 0) snprintf(options.value[7], sizeof(options.value[7]), "%s", "Never");
            else if (Settings.ZipFile == 1) snprintf(options.value[7], sizeof(options.value[7]), "%s", "Always");
            else if (Settings.ZipFile == 2) snprintf(options.value[7], sizeof(options.value[7]), "%s", "Ask each time");

            optionBrowser.TriggerUpdate();
        }

        if(backBtn.GetState() == STATE_CLICKED)
        {
            menu = MENU_HOME;
        }
        if(devBtn.GetState() == STATE_CLICKED)
        {
            menu = MENU_DEVELOPER;
        }
    }

    if(changed && !Settings.Save(0))
        WindowPrompt("Settings not saved", "Check storage and preserved .new/.bak recovery files.", "OK", NULL);

    if(menu == MENU_HOME)
        w.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
    else w.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);

    prevMenu = MENU_SETTINGS;
    while(!App_Exiting() && w.GetEffect() > 0)
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
    prevMenu = MENU_SPLASH;

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

    InitNetwork();
    for (int wait = 0; !networkinit && !App_Exiting() && wait < 550; ++wait)
        usleep(20000);
    if (!networkinit) { menu = MENU_HOME; conn = NET_ERR; }
    Init.StopEffect(EFFECT_ROTATE);

    Init.SetEffect(EFFECT_FADE, -50);
    while(!App_Exiting() && Init.GetEffect() > 0) usleep(THREAD_SLEEP);
    usleep(200*1000);

    HaltGui();
    mainWindow->Remove(&Init);
    ResumeGui();

    if (conn == NET_ERR)
        WindowPrompt("Network unavailable", "Menus remain available. Check Wii network settings before browsing.", "OK", NULL);
    if (conn == IP_ERR)
        WindowPrompt("Error reading IP address", "Application exiting. Please check connection settings", "Ok", NULL);

    return menu;
}

void ShowDownloads()
{
    manager->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    toggleManager = false;

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(manager);
    mainWindow->ChangeFocus(manager);
    ResumeGui();

    int x, y;

    while(!App_Exiting())
    {
        if (userInput[0].wpad->ir.valid)
        {
            x = userInput[0].wpad->ir.x;
            y = userInput[0].wpad->ir.y;

            if ((userInput[0].wpad->btns_d & WPAD_BUTTON_A) && !manager->IsInside(x, y))
                break;
        }

        if (userInput[0].wpad->btns_d & WPAD_BUTTON_B)
            break;

        usleep(100);
    }

    manager->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(!App_Exiting() && manager->GetEffect() > 0)
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
#ifndef WIIFLOW
    if(Settings.Autoupdate)
        ResumeUpdateThread();
#endif

    App->ChangeButtons(HOMEPAGE);
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
    InsertURL.SetLabel(&URL, 2);
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

    if (prevMenu != MENU_TOPSITES)
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
    prevMenu = MENU_HOME;

    while(!choice && !App_Exiting())
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
            InsertURL.ResetState();
            App->btnWWW->ResetState();
            OnScreenKeyboard(mainWindow, new_page, MAXLEN);
            strcpy(prev_page,new_page);
            URL.SetText(new_page);
            URL.SetScroll(SCROLL_HORIZONTAL);
            InsertURL.ResetState();
            App->btnWWW->ResetState();
        }

        else if(App->btnSett->GetState() == STATE_CLICKED)
        {
            fadeAnim = EFFECT_FADE;
            choice = MENU_SETTINGS;
            App->btnSett->ResetState();
        }

        else if(App->btnSave->GetState() == STATE_CLICKED ||
                toggleManager == true)
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
                snprintf(new_page, sizeof(new_page), "%s", history->url.c_str());
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
                snprintf(new_page, sizeof(new_page), "%s", history->url.c_str());
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
            choice = MENU_TOPSITES;
        }

        else if(btnExit.GetState() == STATE_CLICKED)
        {
            if (WindowPrompt("Homepage", "Do you want to exit?", "Ok", "Cancel"))
            {
                fadeAnim = EFFECT_FADE;
                choice = MENU_EXIT;
            }
        }
    }

    btnGo.SetEffect(fadeAnim, -50);
    btnExit.SetEffect(fadeAnim, -50);
    Splash->SetEffect(fadeAnim, -50);
    InsertURL.SetEffect(fadeAnim, -50);
    while(!App_Exiting() && InsertURL.GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(Splash);
    mainWindow->Remove(&InsertURL);
    mainWindow->Remove(&btnGo);
    mainWindow->Remove(&btnExit);
    ResumeGui();

    if(choice != MENU_TOPSITES)
    {
        App->SetEffect(EFFECT_SLIDE_OUT | EFFECT_SLIDE_BOTTOM, 50);
        Right->SetEffect(EFFECT_FADE, -50);
        while(!App_Exiting() && App->GetEffect() > 0) usleep(THREAD_SLEEP);

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
    GuiScopedLock lock;
    if (!actionButton)
        return false;

    return (actionButton->GetState() == STATE_CLICKED);
}

char *omniBox()
{
    char *encode = url_encode(new_page);
    if (!encode) return NULL;
    std::string search = "https://www.google.com/search?gbv=1&q=";
    search += encode;
    free(encode);
    return search.size() <= wbl::MaxUrl ? strdup(search.c_str()) : NULL;
}

static int MenuBrowse()
{
    char *nurl = NULL;
    char *url = NULL;

    file hfile;
    hfile.file = NULL;

    url = (char*) malloc (sizeof(new_page)+10);
    if (!url) return MENU_HOME;
    memset(url, 0, sizeof(new_page)+10);
    bool searchWord = true;

    if (strncmp(new_page,"http",4))
        strcpy(url,"http://");
    strcat(url,new_page);

    GuiWindow promptWindow(448,288);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiImageData dialogBox(dialogue_box_png, dialogue_box_png_size);
    GuiImage dialogBoxImg(&dialogBox);
    prevMenu = MENU_BROWSE;

    GuiText title("WiiBrowser", 26, (GXColor)
    {
        0, 0, 0, 255
    });
    title.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    title.SetPosition(0,40);

    GuiText staticTxt("", 20, (GXColor)
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

    GuiButton cancelButton(btnOutline.GetWidth(), btnOutline.GetHeight());
    actionButton = &cancelButton;
    struct ActionBinding {
        ~ActionBinding() { GuiScopedLock lock; actionButton = NULL; }
    } actionBinding;
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
    promptWindow.Append(actionButton);

    GuiWindow childWindow(screenwidth, screenheight);
    childWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    childWindow.SetPosition(0,0);

jump:
    if (!url || App_Exiting()) { free(url); return MENU_HOME; }
    cancelButton.ResetState();
    decode_html_entities_utf8(url, NULL);
    save_mem(url);
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    promptWindow.Append(&staticTxt);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

#ifdef TIME
    u64 now, prev;
    prev = gettime();
#endif

    if(!CheckConnection())
        InitNetwork();

    int networkWait = 0;
    while(!networkinit && !App_Exiting() && networkWait++ < 11000)
    {
        staticTxt.SetText("Resuming connection...");

        usleep(1000);
    }

    staticTxt.SetText("Loading...please wait");
    struct block HTML;
    std::string postBody;
    const bool isPost = TakeNavigationPost(url, postBody);
    HTML = networkinit && !App_Exiting() ? (isPost ? downloadpost(curl_handle, url, postBody.c_str()) : downloadfile(curl_handle, url, NULL)) : emptyblock;
    const bool requestCancelled = CancelDownload();

    // Release browsing never substitutes an unrelated sd:/page.html file.

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
    while(!App_Exiting() && promptWindow.GetEffect() > 0)
        usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    if (requestCancelled || App_Exiting()) { free(url); free(HTML.data); return MENU_HOME; }

    if(HTML.size == -1)
    {
        if(performDownload(&hfile, url, &HTML))
        {
            if (!AddDownload(curl_multi, url, &hfile))
                WindowPrompt("Download unavailable", "The queue is full or the destination is invalid.", "OK", NULL);
#ifdef WIIFLOW
            toggleManager = true;
#else
            App->SaveTooltip->SetTimeout("Download added", 2);
#endif
        }

        free(url);
        return MENU_HOME;
    }

    if (CURLE_OK == curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &nurl) && nurl && wbl::httpUrl(nurl))
    {
        char *replacement = static_cast<char *>(realloc(url, strlen(nurl) + 1));
        if (!replacement) { free(url); free(HTML.data); return MENU_HOME; }
        url = replacement;
        strcpy(url, nurl);
    }

    if (!HTML.size)
    {
        free(HTML.data);
        free(url);

        if (!searchWord || App_Exiting())
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
    link = DisplayHTML(&HTML, mainWindow, &childWindow, url);

    memset(new_page, 0, sizeof(new_page));
    free(HTML.data);

    HaltGui();
    mainWindow->Remove(&childWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    if (link.length()>0)
    {
        link = parseUrl(link, url);
        if (!wbl::httpUrl(link)) { free(url); return MENU_HOME; }
        char *replacement = static_cast<char *>(realloc(url, link.size() + 1));
        if (!replacement) { free(url); return MENU_HOME; }
        url = replacement;
        strcpy(url,(char*)link.c_str());
        goto jump;
    }
    free(url);
    return MENU_HOME;
}

bool Held(vector<GuiFavorite>& Block)
{
    for (int i=0; i<N; i++)
    {
        if(Block[i].Block->GetState() == STATE_HELD)
            return true;
    }
    return false;
}

void SwapPos(vector<GuiFavorite>& Block, int i, int j)
{
    int xtemp = Block[i].xpos;
    int ytemp = Block[i].ypos;

    Block[i].SetInit(Block[j].xpos, Block[j].ypos);
    Block[i].SetPosition(Block[j].xpos, Block[j].ypos);
    Block[j].SetInit(xtemp, ytemp);
    Block[j].SetPosition(xtemp, ytemp);
}

void SwapImage(vector<GuiFavorite>& Block, int i, int j)
{
    u8 *itemp = Settings.Thumbnails[i];
    Settings.Thumbnails[i] = Settings.Thumbnails[j];
    Settings.Thumbnails[j] = itemp;
}

void SwapUrls(int i, int j)
{
    char utemp[512];
    strcpy(utemp, Settings.GetUrl(i));
    strcpy(Settings.TopSites[i], Settings.TopSites[j]);
    strcpy(Settings.TopSites[j], utemp);
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
 * Top Sites
 ***************************************************************************/
static int MenuTopSites()
{
    bool editing = false;
    App->ChangeButtons(editing ? EDITING : FAVORITES);
    strcpy(new_page,prev_page);

    fadeAnim = EFFECT_SLIDE_IN | EFFECT_SLIDE_RIGHT;
    Right->Button->SetState(STATE_SELECTED);

    vector<GuiFavorite> Block(N, GuiFavorite(TOPSITE));
    prevMenu = MENU_TOPSITES;

    for (int i = 0, xpos = 40, ypos = 40; i < N; i++)
    {
        Block[i].SetInit(xpos, ypos);
        Block[i].SetEffect(fadeAnim, 50);
        Block[i].SetPosition(xpos,ypos);

        Block[i].Block->SetUpdateCallback(DragCallback);
        Block[i].Label->SetText(Settings.GetUrl(i));

        if(Settings.ShowThumbs && Settings.Thumbnails[i])
        {
            Block[i].Thumb->SetImage(Settings.Thumbnails[i], vmode->fbWidth, vmode->efbHeight);
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
    mainWindow->Append(Right);
    ResumeGui();

    int i, choice = 0;
    while(!choice && !App_Exiting())
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
                strcpy(Settings.TopSites[i],prev_page);
                Block[i].Label->SetText(Settings.GetUrl(i));
            }
        }

        else if(Left->Button->GetState() == STATE_CLICKED ||
                App->btnWWW->GetState() == STATE_CLICKED || userInput[0].wpad->btns_d & WPAD_BUTTON_MINUS)
        {
            App->btnWWW->ResetState();
            fadeAnim = EFFECT_SLIDE_OUT | EFFECT_SLIDE_RIGHT;
            choice = MENU_HOME;
        }

        else if(Right->Button->GetState() == STATE_CLICKED ||
                userInput[0].wpad->btns_d & WPAD_BUTTON_PLUS)
        {
            fadeAnim = EFFECT_SLIDE_OUT | EFFECT_SLIDE_LEFT;
            choice = MENU_FAVORITES;
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
        while(!App_Exiting() && Block[N-1].Remove->GetEffect() > 0) usleep(THREAD_SLEEP);
    }

    for (i = 0; i < N; i++)
        Block[i].SetEffect(fadeAnim, 50);
    while(!App_Exiting() && Block[N-1].GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    for (i = 0; i < N; i++)
        mainWindow->Remove(&Block[i]);
    ResumeGui();

    if(choice != MENU_HOME)
    {
        App->SetEffect(EFFECT_SLIDE_OUT | EFFECT_SLIDE_BOTTOM, 50);
        Left->SetEffect(EFFECT_FADE, -50);
        while(!App_Exiting() && App->GetEffect() > 0) usleep(THREAD_SLEEP);

        HaltGui();
        mainWindow->Remove(App);
        mainWindow->Remove(Left);
        ResumeGui();
    }

    fadeAnim = EFFECT_SLIDE_IN | EFFECT_SLIDE_LEFT;
    return choice;
}

/****************************************************************************
 * MenuFavorites
 ***************************************************************************/
typedef struct page
{
    struct favorite *ret;
    int n_url;
} page;

struct page GetPage(int page)
{
    struct page str;
    struct favorite *elem, *ret = NULL;
    struct favorite temp;

    char *item;
    int i, j;

    for (i = 0, j = 0; i<N; j++)
    {
        if (page)
        {
            elem = Settings.GetFav(j+9*(page-1));
            if(!elem)
                break;

            strcpy(temp.url, elem->url);
            strcpy(temp.name, elem->name);
        }
        else
        {
            item = Settings.GetUrl(j);
            if(!item)
                break;

            strcpy(temp.url, item);
            strcpy(temp.name, item);
        }

        ret = (struct favorite*)realloc(ret, (i+1)*sizeof(struct favorite));
        ret[i++] = temp;
    }

    str.ret = ret;
    str.n_url = i;
    return str;
}

static int MenuFavorites()
{
    int choice = 0, n_rec = 0;
    int i, xpos, ypos;
    struct page Page;

    Page = GetPage(currPage);
    History hst = history;
    strcpy(new_page,prev_page);

    fadeAnim = EFFECT_SLIDE_IN | EFFECT_SLIDE_LEFT;
    Right->Button->SetState(STATE_SELECTED);
    Right->SetEffect(EFFECT_FADE, -50);

    vector<GuiFavorite> Block(N, GuiFavorite(FAVORITE));
    vector<GuiFavorite> Recent(N, GuiFavorite(FAVORITE));
    prevMenu = MENU_FAVORITES;

    // favorites
    for (i = 0; i < N; i++)
    {
        Block[i].SetEffect(EFFECT_FADE, 50);
        Block[i].Block->SetUpdateCallback(DragCallback);
        Block[i].Block->SetLabel(Block[i].Label);
    }

    for (i = 0, xpos = 40, ypos = 100; i < Page.n_url; i++)
    {
        if(strlen(Page.ret[i].name))
            Block[i].Label->SetText(Page.ret[i].name);
        else Block[i].Label->SetText(Page.ret[i].url);

        Block[i].SetPosition(xpos, ypos);
        Block[i].SetEffect(EFFECT_SLIDE_IN | EFFECT_SLIDE_RIGHT, 50);
        ypos += Block[i].GetDataHeight()+15;

        if(ypos > screenheight-90)
        {
            xpos += Block[i].GetDataWidth()+35;
            ypos = 100;
        }
    }

    GuiImageData btnImgData(button_next_png, button_next_png_size);
    GuiImageData btnImgDataOver(button_next_over_png, button_next_over_png_size);
    GuiImage btnImg(&btnImgData);
    GuiImage btnImgOver(&btnImgDataOver);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    btnImg.SetScaleX(0.83);
    btnImgOver.SetScaleX(0.83);

    GuiButton btnNext(btnImg.GetRealWidth(), btnImg.GetRealHeight());
    btnNext.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    btnNext.SetEffect(EFFECT_SLIDE_IN | EFFECT_SLIDE_RIGHT, 50);
    btnNext.SetPosition(xpos, ypos);
    btnNext.SetImage(&btnImg);
    btnNext.SetImageOver(&btnImgOver);
    btnNext.SetSoundOver(&btnSoundOver);
    btnNext.SetTrigger(trigA);
    btnNext.SetEffectGrow();

    // recent sites
    for (i = 0, xpos = 434, ypos = 100; i < 5; i++)
    {
        if (!hst || !(hst = hst->prec))
            break;

        Recent[i].SetInit(xpos, ypos);
        Recent[i].SetEffect(EFFECT_SLIDE_IN | EFFECT_SLIDE_RIGHT, 50);
        Recent[i].SetPosition(xpos,ypos);

        Recent[i].Block->SetUpdateCallback(DragCallback);
        Recent[i].Label->SetText(hst->url.c_str());
        Recent[i].Block->SetLabel(Recent[i].Label);

        ypos += Recent[i].GetDataHeight()+15;
        if(ypos > screenheight-90)
        {
            xpos += Recent[i].GetDataWidth()+35;
            ypos = 100;
        }
        n_rec = i+1;
    }

    HaltGui();
    for (int i = 0; i < Page.n_url; i++)
        mainWindow->Append(&Block[i]);
    for (int i = 0; i < n_rec; i++)
        mainWindow->Append(&Recent[i]);
    ResumeGui();

    GuiText bookTxt("", 28, (GXColor)
    {
        0, 0, 0, 255
    });
    bookTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    bookTxt.SetPosition(40,40);
    bookTxt.SetEffect(EFFECT_FADE, 30);

    if(!currPage)
        bookTxt.SetText("Top Sites");
    else bookTxt.SetText("Bookmarks");

    GuiText histTxt("Recent History", 28, (GXColor)
    {
        0, 0, 0, 255
    });
    histTxt.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    histTxt.SetPosition(-40,40);
    histTxt.SetEffect(EFFECT_FADE, 30);

    HaltGui();
    mainWindow->Append(&btnNext);
    ResumeGui();

    while(!App_Exiting() && btnNext.GetEffect() > 0)
        usleep(THREAD_SLEEP);

    Left->SetEffect(EFFECT_FADE, 50);
    HaltGui();
    mainWindow->Append(Left);
    mainWindow->Append(&bookTxt);
    mainWindow->Append(&histTxt);
    mainWindow->Remove(Right);
    ResumeGui();

    while(!choice && !App_Exiting())
    {
        usleep(THREAD_SLEEP);

        if(Left->Button->GetState() == STATE_CLICKED ||
                userInput[0].wpad->btns_d & WPAD_BUTTON_MINUS)
        {
            choice = MENU_HOME;
        }

        if(btnNext.GetState() == STATE_CLICKED ||
                userInput[0].wpad->btns_d & WPAD_BUTTON_PLUS)
        {
            currPage++;

            free(Page.ret);
            Page = GetPage(currPage);
            btnNext.ResetState();

            if(!Page.n_url)
            {
                bookTxt.SetText("Top Sites");
                Page = GetPage(0);
                currPage = 0;
            }
            else bookTxt.SetText("Bookmarks");

            for (i = 0, xpos = 40, ypos = 100; i < Page.n_url; i++)
            {
                Block[i].SetPosition(xpos, ypos);

                ypos += Block[i].GetDataHeight()+15;
                if(ypos > screenheight-90)
                {
                    xpos += Block[i].GetDataWidth()+35;
                    ypos = 100;
                }
            }

            btnNext.SetPosition(xpos, ypos);
            HaltGui();

            for (i = 0; i < Page.n_url; i++)
            {
                if(strlen(Page.ret[i].name))
                    Block[i].Label->SetText(Page.ret[i].name);
                else Block[i].Label->SetText(Page.ret[i].url);

                mainWindow->Append(&Block[i]);
            }

            for (i = Page.n_url; i < N; i++)
                mainWindow->Remove(&Block[i]);

            mainWindow->Append(Left);
            ResumeGui();
        }

        for (i = 0; i < Page.n_url; i++)
        {
            if(Block[i].Block->GetState() == STATE_SELECTED)
                Block[i].Label->SetScroll(SCROLL_HORIZONTAL);
            else Block[i].Label->SetScroll(SCROLL_NONE);

            if(Block[i].Block->GetState() == STATE_CLICKED &&
                    Left->Button->GetState() == STATE_DEFAULT)
            {
                strcpy(new_page,Page.ret[i].url);
                strcpy(prev_page,new_page);
                Block[i].Block->ResetState();
            }
        }

        for (i = 0; i < n_rec; i++)
        {
            if(Recent[i].Block->GetState() == STATE_SELECTED)
                Recent[i].Label->SetScroll(SCROLL_HORIZONTAL);
            else Recent[i].Label->SetScroll(SCROLL_NONE);

            if(Recent[i].Block->GetState() == STATE_CLICKED &&
                    Left->Button->GetState() == STATE_DEFAULT)
            {
                strcpy(new_page,Recent[i].Label->GetText());
                strcpy(prev_page,new_page);
                Recent[i].Block->ResetState();
            }
        }
    }

    for (i = 0; i < Page.n_url; i++)
        Block[i].SetEffect(EFFECT_SLIDE_OUT | EFFECT_SLIDE_RIGHT, 50);
    for (i = 0; i < n_rec; i++)
        Recent[i].SetEffect(EFFECT_SLIDE_OUT | EFFECT_SLIDE_RIGHT, 50);

    btnNext.SetEffect(EFFECT_SLIDE_OUT | EFFECT_SLIDE_RIGHT, 50);
    bookTxt.SetEffect(EFFECT_FADE, -30);
    histTxt.SetEffect(EFFECT_FADE, -30);

    while(!App_Exiting() && Page.n_url && Block[Page.n_url-1].GetEffect() > 0)
        usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&btnNext);
    mainWindow->Remove(&bookTxt);
    mainWindow->Remove(&histTxt);

    for (i = 0; i < Page.n_url; i++)
        mainWindow->Remove(&Block[i]);
    for (i = 0; i < n_rec; i++)
        mainWindow->Remove(&Recent[i]);
    ResumeGui();

    if(choice != MENU_HOME)
    {
        Left->SetEffect(EFFECT_FADE, -50);
        while(!App_Exiting() && Left->GetEffect() > 0)
            usleep(THREAD_SLEEP);

        HaltGui();
        mainWindow->Remove(Left);
        ResumeGui();
    }

    free(Page.ret);
    return choice;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
void LoadSession()
{
    history = LoadList();

    /* setup cookies engine */
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl_handle, CURLOPT_COOKIESESSION, 1L);
}

void DeleteSession()
{
    FreeHistory(history);
    history = InitHistory();

    /* setup cookies engine */
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "");
}

/* lock callback */
void Lock(CURL *handle, curl_lock_data data, curl_lock_access access,void *useptr )
{
    LWP_MutexLock(m_mutex);
}

/* unlock callback */
void Unlock(CURL *handle, curl_lock_data data, void *useptr )
{
    LWP_MutexUnlock(m_mutex);
}

void Init()
{
    if(curl_global_init(CURL_GLOBAL_ALL)) { ExitAccepted = true; return; }
    curlReady = true;

    curl_handle = curl_easy_init();
    if (!curl_handle) { ExitAccepted = true; return; }
    memset(prev_page, 0, sizeof(prev_page));

    if (!Settings.Restore && Settings.CleanExit)
    {
        Settings.Save(0);
        DeleteSession();
    }
    else LoadSession();
    SessionInitialized = true;

#ifdef MPLAYER
    if(!InitMPlayer())
    {
        ExitRequested = true;
        return;
    }
#endif

    SetupGui();
    if (!ExitAccepted && (!Settings.CanSave() || !Settings.CanSaveBookmarks()))
        WindowPrompt("Settings recovery", "An invalid or interrupted save was preserved. Recovered state is read-only; see README before restoring files.", "OK", NULL);
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

    char cookies[256];
    snprintf(cookies, sizeof(cookies), "%s/appdata/cookie.csv", Settings.AppPath);

    /* setup cookies engine */
#ifndef WIIFLOW
    // No unchecked plaintext cookie-jar writes during teardown.
#endif
    if(curl_handle)
    {
        curl_easy_cleanup(curl_handle);
        curl_handle = NULL;
    }

    if(curl_share)
    {
        curl_share_cleanup(curl_share);
        curl_share = NULL;
    }
    if (curlReady) curl_global_cleanup();
    curlReady = false;
    mainWindow = NULL;
}

void MainMenu(int menu)
{
    int currentMenu = menu;
    // bool exitwindow = false;

    Init();
    if (ExitAccepted) return;
#ifndef WIIFLOW
    if(!Settings.CleanExit)
        WindowPrompt("Oops... this is embarrassing", "The app didn't close correctly, the previous session has been automatically restored", "OK", NULL);
#endif

    while(currentMenu != MENU_EXIT && !App_Exiting())
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
        case MENU_TOPSITES:
            currentMenu = MenuTopSites();
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

    ExitAccepted = 1;
    ResumeGui();
}
