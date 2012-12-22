/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>

#include "libwiigui/gui.h"
#include "liste.h"
#include "menu.h"
#include "main.h"
#include "input.h"
#include "filelist.h"
#include "filebrowser.h"

extern "C" {
#include "entities.h"
}

#define THREAD_SLEEP 100
#define MAXLEN 100

CURL *curl_handle;
History history;

char homepage[]="www.google.it/";
char hmpg[MAXLEN];

static GuiImageData * pointer[4];
static GuiImage * bgImg = NULL;
static GuiSound * bgMusic = NULL;
static GuiWindow * mainWindow = NULL;
static lwp_t guithread = LWP_THREAD_NULL;
static lwp_t updatethread = LWP_THREAD_NULL;
static bool guiHalt = true;
static int updateThreadHalt=0;

using namespace std;

/****************************************************************************
 * Adjust urls
 ***************************************************************************/
char * getHost(char *url) {
    char *p=strchr (url, '/')+2;
    char *c=strchr (p, '/');
    return strndup(url,(c+1)-url);
}

string getRoot(char *url) {
    char *p=strrchr (url, '/');
    return strndup(url,(p+1)-url);
}

string adjustUrl(string link, const char* url) {
    string result;
    if (link.find("http://")==0 || link.find("https://")==0)
        return link;
    else if (link.at(0)=='/' && link.at(1)=='/')
        result.assign("http:"); // https?
    else if (link.at(0)=='/') {
        link.erase(link.begin());
        result=getHost((char*)url);
    }
    else result=getRoot((char*)url);
    result.append(link);
    return result;
}

string jumpUrl(string link, const char* url) {
    string res(url, strlen(url));
    res.append(link);
    return res;
}

string parseUrl(string link, const char* url) {
    if (link.at(0)=='#')
        return jumpUrl(link, url);
    return adjustUrl(link, url);
}

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
void HaltGui()
{
	guiHalt = true;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(guithread))
		usleep(THREAD_SLEEP);
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label)
{
	int choice = -1;

	GuiWindow promptWindow(448,288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,40);
	GuiText msgTxt(msg, 22, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-20);
	msgTxt.SetWrap(true, 400);

	GuiText btn1Txt(btn1Label, 22, (GXColor){0, 0, 0, 255});
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
	btn1.SetTrigger(&trigA);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	GuiText btn2Txt(btn2Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btn2.SetPosition(-20, -25);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetImageOver(&btn2ImgOver);
	btn2.SetSoundOver(&btnSoundOver);
	btn2.SetTrigger(&trigA);
	btn2.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);

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
    int appversion = 0;
	while(1)
	{
		LWP_SuspendThread(updatethread);
		if(updateThreadHalt || !(appversion=checkUpdate()))
			return NULL;
        usleep(500*1000);

		bool installUpdate = WindowPrompt(
			"Update Available",
			"An update is available!",
			"Update now",
			"Update later");

		if(installUpdate) {
			HaltGui();
			if(downloadUpdate(appversion))
				ExitRequested = true;
            ResumeGui();
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
	int i;

	while(1)
	{
		if(guiHalt)
		{
			LWP_SuspendThread(guithread);
		}
		else
		{
			UpdatePads();
			mainWindow->Draw();

			#ifdef HW_RVL
			for(i=3; i >= 0; i--) // so that player 1's cursor appears on top!
			{
				if(userInput[i].wpad->ir.valid)
					Menu_DrawImg(userInput[i].wpad->ir.x-48, userInput[i].wpad->ir.y-48,
						96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255);
				DoRumble(i);
			}
			#endif

            if(HWButton)
                ExitRequested = true; // exit program
			Menu_Render();

			for(i=0; i < 4; i++)
			{
				mainWindow->Update(&userInput[i]);
				if(userInput[i].wpad->btns_d & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME))
                    ExitRequested = true; // exit program
			}

			if(ExitRequested)
			{
				for(i = 0; i <= 255; i += 15)
				{
					mainWindow->Draw();
					Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, (u8)i},1);
					Menu_Render();
				}
				ExitApp();
			}
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
	LWP_CreateThread (&guithread, UpdateGUI, NULL, NULL, 0, 70);
	LWP_CreateThread (&updatethread, UpdateThread, NULL, NULL, 0, 60);
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
void OnScreenKeyboard(GuiWindow * keyboardWindow, char *var, u16 maxlen, bool autoComplete)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText okBtnTxt("OK", 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 22, (GXColor){0, 0, 0, 255});
	if (autoComplete)
        cancelBtnTxt.SetText("Homepage");
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetTrigger(&trigA);
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
    else
    {
		if (autoComplete)
            snprintf(var, maxlen, "%s", homepage);
    }

    keyboard.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
	while(keyboard.GetEffect() > 0) usleep(THREAD_SLEEP);

	HaltGui();
	keyboardWindow->Remove(&keyboard);
    keyboardWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}

// Splash screen function, runs while initiating network functions.
static int Splash()
{
    GuiImageData SplashImage(wiixplore_png);
    GuiImage Splash(&SplashImage);
    Splash.SetAlignment(2,5);
    Splash.SetPosition(0,0);
    GuiText init("Initialising...", 20, (GXColor){225, 225, 225, 255});
    init.SetAlignment(2,4);
    init.SetPosition(0,0);

    Splash.SetEffect(EFFECT_FADE, 50);
    init.SetEffect(EFFECT_FADE, 50);
    HaltGui();
    mainWindow->Append(&Splash);
    mainWindow->Append(&init);
    ResumeGui();

    char myIP[16]; s32 ip;
    int conn=0, menu=MENU_HOME;
    while ((ip = net_init()) == -EAGAIN) {
        usleep(100 * 1000); // 100ms
    }
    if(ip < 0) {
        menu = MENU_EXIT; conn = NET_ERR;
    }
    if (if_config(myIP, NULL, NULL, true) < 0) {
        menu = MENU_EXIT; conn = IP_ERR;
    }

    Splash.SetEffect(EFFECT_FADE, -50);
    init.SetEffect(EFFECT_FADE, -50);
    while(Splash.GetEffect() > 0) usleep(THREAD_SLEEP);
    while(init.GetEffect() > 0) usleep(THREAD_SLEEP);
    usleep(500*1000);

    bgImg = new GuiImage(screenwidth, screenheight, (GXColor){225, 225, 225, 255});
    mainWindow->Append(bgImg);
    remove("debug.txt");

    HaltGui();
        mainWindow->Remove(&Splash);
        mainWindow->Remove(&init);
    ResumeGui();

    if (conn == NET_ERR)
        WindowPrompt("Connection failure", "Please check network settings, exiting...", "Ok", NULL);
    if (conn == IP_ERR)
        WindowPrompt("Error reading IP address", "Application exiting. Please check connection settings", "Ok", NULL);

    // ResumeUpdateThread();
    return menu;
}

static int Home()
{
    char save[MAXLEN];
    strcpy(save,hmpg);
    OnScreenKeyboard(mainWindow, save, MAXLEN, 1);
    strcpy(hmpg,save);

    char *nurl=NULL;
    char *url=(char*) malloc (sizeof(save)+10);
    strcpy(url,"http://");
    strcat(url,save);

    jump:
    GuiWindow promptWindow(448,288);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiImageData dialogBox(dialogue_box_png, dialogue_box_png_size);
    GuiImage dialogBoxImg(&dialogBox);

    GuiText title("WiiXplore", 26, (GXColor){0, 0, 0, 255});
    title.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    title.SetPosition(0,40);
    GuiText msgTxt("Loading...please wait.", 22, (GXColor){0, 0, 0, 255});
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-20);
    msgTxt.SetWrap(true, 400);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&title);
    promptWindow.Append(&msgTxt);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    save_mem(url);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    #ifdef TIME
    struct timeval begin, end;
    gettimeofday(&begin, 0);
    #endif

    decode_html_entities_utf8(url, NULL);
    struct block HTML;
    HTML = downloadfile(curl_handle, url, NULL);

    #ifdef DEBUG
    FILE *pFile = fopen ("Google.htm", "rb");
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
        gettimeofday(&end, 0);
        usleep(100);
    } while ((end.tv_sec-begin.tv_sec)*1000.0+(end.tv_usec-begin.tv_usec)/1000.0 < 500);
    #endif

    usleep(500*1000);
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    if (CURLE_OK == curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &nurl))
    {
        url = (char*) realloc (url,strlen(nurl)+1);
        strcpy(url, nurl);
    }

    if (HTML.size == 0)
    {
        WindowPrompt("WiiXplore", "Failed", "Ok", NULL);
        return MENU_HOME;
    }

    if (!history || strcmp(history->url.c_str(),url))
        history=InsUrl(history,url);
    sleep(1);

    GuiWindow childWindow(screenwidth, screenheight);
    childWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    childWindow.SetPosition(0,0);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&childWindow);
    mainWindow->ChangeFocus(&childWindow);
    ResumeGui();

    string link;
	link = DisplayHTML(&HTML, mainWindow, &childWindow, url);
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
    free(url);

    int close = WindowPrompt("Homepage", "Do you want to exit?", "Ok", "Cancel");
    if (close)
        return MENU_EXIT;
    return MENU_HOME;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
void MainMenu(int menu)
{
	int currentMenu = menu;
    memset(hmpg, 0, sizeof(hmpg));
    history = InitHistory();

    if(curl_global_init(CURL_GLOBAL_ALL))
        ExitRequested=1;
    curl_handle = curl_easy_init();

	#ifdef HW_RVL
	pointer[0] = new GuiImageData(player1_point_png);
	pointer[1] = new GuiImageData(player2_point_png);
	pointer[2] = new GuiImageData(player3_point_png);
	pointer[3] = new GuiImageData(player4_point_png);
	#endif

	mainWindow = new GuiWindow(screenwidth, screenheight);
    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	ResumeGui();

	bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(50);
	bgMusic->SetLoop(true);
	bgMusic->Play(); // startup music

    while(currentMenu != MENU_EXIT)
	{
		switch (currentMenu)
		{
			case MENU_SPLASH:
				currentMenu = Splash();
				break;
			case MENU_HOME:
				currentMenu = Home();
				break;
			default: // unrecognized menu
				currentMenu = Home();
				break;
		}
	}

	ResumeGui();
	ExitRequested = 1;
	while(1) usleep(THREAD_SLEEP);

	HaltGui();

	bgMusic->Stop();
	delete bgMusic;
	delete bgImg;
	delete mainWindow;

	delete pointer[0];
	delete pointer[1];
	delete pointer[2];
	delete pointer[3];

	curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
	mainWindow = NULL;
}
