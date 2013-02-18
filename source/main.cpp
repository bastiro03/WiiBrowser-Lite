/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * main.cpp
 * Basic template/demonstration of libwiigui capabilities. For a
 * full-featured app using many more extensions, check out Snes9x GX.
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <dirent.h>

#include "video.h"
#include "audio.h"
#include "menu.h"
#include "gettext.h"
#include "liste.h"
#include "input.h"
#include "filelist.h"
#include "main.h"

#include "FreeTypeGX.h"
#include "utils/mem2_manager.h"
#include "mplayer/input/input.h"
#include "mplayer/osdep/gx_supp.h"

extern char MPLAYER_DATADIR[512];
extern char MPLAYER_CONFDIR[512];
extern char MPLAYER_LIBDIR[512];
extern char MPLAYER_CSSDIR[512];

// MPlayer threads
#define MPLAYER_STACKSIZE (512*1024)
#define CACHE_STACKSIZE (16*1024)
static lwp_t mthread = LWP_THREAD_NULL;
static lwp_t cthread = LWP_THREAD_NULL;
static u8 cachestack[CACHE_STACKSIZE] ATTRIBUTE_ALIGN (32);

static void *
mplayerthread (void *arg)
{
    mplayer_main();
    return NULL;
}

extern "C" {
    extern void *mplayercachethread(void *arg);

    void SuspendCacheThread()
    {
        LWP_SuspendThread(cthread);
    }
    void ResumeCacheThread()
    {
        LWP_ResumeThread(cthread);
    }
    bool CacheThreadSuspended()
    {
        if(LWP_ThreadIsSuspended(cthread))
            return true;
        return false;
    }
}

void show_mem()
{
    printf("m1(%.4f) m2(%.4f)\n",
           ((float)((char*)SYS_GetArena1Hi()-(char*)SYS_GetArena1Lo()))/0x100000,
           ((float)((char*)SYS_GetArena2Hi()-(char*)SYS_GetArena2Lo()))/0x100000);
}

bool InitMPlayer()
{
    u8 *mplayerstack;
    char *AppPath = Settings.AppPath;

    static bool init = false;
    if(init) return true;

    if(AppPath[0] == 0)
        return false;

    if(chdir(AppPath) != 0)
    {
        wchar_t msg[512];
        swprintf(msg, 512, L"%s %s", gettext("Unable to change path to"), AppPath);
        return false;
    }

    // check if subtitle font file exists
    struct stat st;
    char filepath[1024];
    sprintf(filepath, "%s/subfont.ttf", AppPath);

    bool subtitleFontFound;
    if(stat(filepath, &st) == 0)
        subtitleFontFound = true;

    sprintf(MPLAYER_DATADIR,"%s",AppPath);
    sprintf(MPLAYER_CONFDIR,"%s",AppPath);
    sprintf(MPLAYER_LIBDIR,"%s",AppPath);
    sprintf(MPLAYER_CSSDIR,"%s/css",AppPath);
    DIR *dir = opendir(MPLAYER_CSSDIR);

    if(!dir && mkdir(MPLAYER_CSSDIR, 0777) != 0)
        sprintf(MPLAYER_CSSDIR, "off");
    else
        closedir(dir);

    setenv("HOME", MPLAYER_DATADIR, 1);
    setenv("DVDCSS_CACHE", MPLAYER_CSSDIR, 1);
    setenv("DVDCSS_METHOD", "key", 1);
    setenv("DVDCSS_VERBOSE", "0", 1);
    setenv("DVDREAD_VERBOSE", "0", 1);
    setenv("DVDCSS_RAW_DEVICE", "/dev/di", 1);

    char agent[30];
    sprintf(agent, "%s/%s (IOS%d)", APPNAME, "rev40", IOS_GetVersion());
    char *network_useragent = mem2_strdup(agent, MEM2_OTHER);

    // create mplayer thread
    mplayerstack=(u8*)(memalign(32,MPLAYER_STACKSIZE*sizeof(u8)));
    memset(mplayerstack,0,MPLAYER_STACKSIZE*sizeof(u8));
    LWP_CreateThread (&mthread, mplayerthread, NULL, mplayerstack, MPLAYER_STACKSIZE, 68);

    init = true;
    return true;
}

void LoadMPlayerFile()
{
    // if(!WiiSettings.subtitleVisibility)
        // SuspendParseThread();

    /*settingsSet = false;
    nowPlayingSet = false;*/
	controlledbygui = 2; // signal any previous file to end

	// wait for previous file to end
	while(controlledbygui == 2)
		usleep(100);

    char *partitionlabel = NULL;
    char ext[7] = "mp4";
    char* loadedFile = "sd:/Naruto.mp4"; // "http://www.w3schools.com/html/movie.mp4";

    /*if(1)
    {
        if (strncmp(loadedFile, "dvd://", 6) == 0 || strncmp(loadedFile, "dvdnav://", 9) == 0)
            wiiSetDVDDevice(loadedDevice);
        partitionlabel = GetPartitionLabel(loadedFile);
    }*/

    // wait for directory parsing to finish (to find subtitles)
    /*while(WiiSettings.subtitleVisibility && !ParseDone())
        usleep(100);*/

    // set new file to load
    wiiLoadFile(loadedFile, partitionlabel);

    while(controlledbygui != 0)
        usleep(100);
}

/*void ResumeMPlayerFile()
{
    DisableRumble();
    SuspendDeviceThread();
    SuspendPictureThread();
    SuspendParseThread();
    settingsSet = false;
    nowPlayingSet = false;
    controlledbygui = 0;
}

void StopMPlayerFile()
{
    controlledbygui = 2; // signal any previous file to end
}*/

extern "C" {
    void SetMPlayerSettings()
    {
        /*static float aspectRatio=-2;
        if(settingsSet)
            return;

        settingsSet = true;

        GX_SetScreenPos(WiiSettings.videoXshift, WiiSettings.videoYshift,
                        WiiSettings.videoZoomHor, WiiSettings.videoZoomVert);
        wiiSetAutoResume(WiiSettings.autoResume);
        wiiSetVolume(WiiSettings.volume);
        wiiSetSeekBackward(WiiSettings.skipBackward);
        wiiSetSeekForward(WiiSettings.skipForward);*/

        wiiSetCacheFill(30);
        wiiSetOnlineCacheFill(20);

        /*if(strncmp(loadedFile, "dvd", 3) == 0) // always use framedropping for DVD
            wiiSetProperty(MP_CMD_FRAMEDROPPING, FRAMEDROPPING_AUTO);
        else
            wiiSetProperty(MP_CMD_FRAMEDROPPING, WiiSettings.frameDropping);

        if(aspectRatio!=WiiSettings.aspectRatio)
        {
            aspectRatio=WiiSettings.aspectRatio;
            wiiSetProperty(MP_CMD_SWITCH_RATIO, WiiSettings.aspectRatio);
        }

        wiiSetProperty(MP_CMD_AUDIO_DELAY, WiiSettings.audioDelay);*/

        // if(!subtitleFontFound)
            wiiSetProperty(MP_CMD_SUB_VISIBILITY, 0);
        /*else
            wiiSetProperty(MP_CMD_SUB_VISIBILITY, WiiSettings.subtitleVisibility);

        wiiSetProperty(MP_CMD_SUB_DELAY, WiiSettings.subtitleDelay);

        wiiSetCodepage(WiiSettings.subtitleCodepage);

        char audioLang[14] = { 0 };
        char subtitleLang[14] = { 0 };

        if(WiiSettings.audioLanguage[0] != 0)
            sprintf(audioLang, "%s,%s,en,eng",
                    WiiSettings.audioLanguage, languages[GetLangIndex(WiiSettings.audioLanguage)].abbrev2);
        if(WiiSettings.subtitleLanguage[0] != 0)
            sprintf(subtitleLang, "%s,%s,en,eng",
                    WiiSettings.subtitleLanguage, languages[GetLangIndex(WiiSettings.subtitleLanguage)].abbrev2);

        wiiSetAudioLanguage(audioLang);
        wiiSetSubtitleLanguage(subtitleLang);
        wiiSetSubtitleColor(WiiSettings.subtitleColor);
        wiiSetSubtitleSize(0.6+(WiiSettings.subtitleSize-1)*0.6); // 1.0-5.0 --> 0.6-3.0*/
    }
}

SSettings Settings;
int ExitRequested = 0,
                    GuiShutdown = 0;

void ExitApp()
{
    Settings.Save();
    ShutoffRumble();
    StopGX();
    ShutdownAudio();
    ClearFontData();
    FreeHistory(history);
    StopUpdateThread();
    if (HWButton)
        SYS_ResetSystem(HWButton, 0, 0);
    exit(0);
}

u8 HWButton;
void WiiResetPressed()
{
    HWButton = SYS_RETURNTOMENU;
}
void WiiPowerPressed()
{
    HWButton = SYS_POWEROFF_STANDBY;
}
void WiimotePowerPressed(s32 chan)
{
    HWButton = SYS_POWEROFF_STANDBY;
}

int main(int argc, char *argv[])
{
    SYS_SetResetCallback(WiiResetPressed);
    SYS_SetPowerCallback(WiiPowerPressed);
    WPAD_SetPowerButtonCallback(WiimotePowerPressed);

    InitVideo(); // Initialize video
    SetupPads(); // Initialize input
    InitAudio(); // Initialize audio
    fatInitDefault(); // Initialize file system
    InitGUIThreads(); // Initialize GUI

    u32 size = ( (1024*MAX_HEIGHT)+((MAX_WIDTH-1024)*MAX_HEIGHT) + (1024*(MAX_HEIGHT/2)*2) ) + // textures
               (vmode->fbWidth * vmode->efbHeight * 4) + //videoScreenshot
               (32*1024); // padding

    AddMem2Area (size, MEM2_VIDEO);
    __exception_setreload(10);

    GX_AllocTextureMemory();
    InitVideo2();
    InitFreeType(); // Initialize font system

    // mplayer cache thread
    memset(cachestack,0,CACHE_STACKSIZE*sizeof(u8));
    LWP_CreateThread(&cthread, mplayercachethread, NULL, cachestack, CACHE_STACKSIZE, 70);
    ResetVideo_Menu();

    Settings.Load();
    LoadLanguage();
    MainMenu(MENU_SPLASH);
    ExitApp();
}
