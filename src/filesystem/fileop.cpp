#include <stdio.h>
#include <stdlib.h>

#include "fileop.h"
#include "filebrowser.h"
#include "config.h"
#include "safety.h"
#include "gui_lock.h"

bool GuiBrowser(GuiWindow *mainWindow, GuiWindow *parentWindow, char *path, size_t capacity, const char *label)
{
    if (!path || !capacity) return false;
    path[0] = 0;
    char temp[512] = {};
    char title[100];
	int i;

	ShutoffRumble();

	// populate initial directory listing
	if(BrowseDevice(SD) < 0 && BrowseDevice(USB) < 0)
	{
		WindowPrompt(
		"Error",
		"Unable to display files on selected load device",
		"Ok",
		NULL);

        return false;
	}

	int menu = MENU_NONE;
	int dev = !strcmp(rootdir, "usb:/") ? USB : SD;
	char mount[2][5] = {"SD", "USB"};

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    if(OpenDefaultFolder() <= 0)
    {
        BrowseDevice(dev);
        snprintf(temp, sizeof(temp), "%s", rootdir);
    }
    else snprintf(temp, sizeof(temp), "%s/", Settings.UserFolder);
    dev = !strcmp(rootdir, "usb:/") ? USB : SD;

    snprintf(title, sizeof(title), "%s", "Browse files");
    path[0] = 0;

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData Textbox(textbox_end_png);
    GuiImage TextboxImg(&Textbox);
    GuiButton InsertURL(TextboxImg.GetWidth(), TextboxImg.GetHeight());

    GuiImageData Device(textbox_begin_png);
    GuiImage DeviceImg(&Device);
    GuiButton InsertDEV(DeviceImg.GetWidth(), DeviceImg.GetHeight());

    GuiText URL(strchr(temp, '/'), 20, (GXColor){0, 0, 0, 255});
    GuiText DEV(mount[dev], 20, (GXColor){0, 0, 0, 255});

    URL.SetMaxWidth(TextboxImg.GetWidth()-20);
    URL.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    URL.SetPosition(5,0);
    URL.SetScroll(SCROLL_HORIZONTAL);

    InsertURL.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    InsertURL.SetPosition(InsertDEV.GetWidth()/2,30);
    InsertURL.SetLabel(&URL);
    InsertURL.SetImage(&TextboxImg);
    InsertURL.SetSoundOver(&btnSoundOver);
    InsertURL.SetTrigger(&trigA);

    InsertDEV.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    InsertDEV.SetPosition(InsertURL.GetLeft()-InsertDEV.GetWidth()/2,30);
    InsertDEV.SetLabel(&DEV);
    InsertDEV.SetImage(&DeviceImg);
    InsertDEV.SetSoundOver(&btnSoundOver);
    InsertDEV.SetTrigger(&trigA);
    InsertDEV.SetEffectGrow();

    GuiText titleTxt(title, 28, (GXColor){0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 108);
	fileBrowser.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 30);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiText okBtnTxt(label, 24, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(50, -35);
	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

    GuiText cancelBtnTxt("Cancel", 24, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-50, -35);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetEffectGrow();

	GuiWindow buttonWindow(screenwidth, screenheight);
	buttonWindow.Append(&okBtn);
	buttonWindow.Append(&cancelBtn);
	buttonWindow.Append(&InsertURL);
	buttonWindow.Append(&InsertDEV);
    buttonWindow.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 30);

    if (mainWindow)
    {
        mainWindow->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
        while(!App_Exiting() && mainWindow->GetEffect() > 0)
            usleep(100);
    }

    HaltGui();
    if (mainWindow)
        parentWindow->Remove(mainWindow);
    parentWindow->Append(&buttonWindow);
    parentWindow->Append(&fileBrowser);
    ResumeGui();

	while(menu == MENU_NONE && !App_Exiting())
	{
		usleep(100);
		if(!strlen(URL.GetText()) || URL.GetText()[0] != '/')
        {
            sprintf(temp, "%s", rootdir);
            URL.SetText(strchr(temp, '/'));
        }

        if(InsertURL.GetState() == STATE_CLICKED)
        {
            URL.SetScroll(SCROLL_NONE);
            char *editable = strchr(temp, '/');
            if (editable) OnScreenKeyboard(parentWindow, editable, sizeof(temp) - (editable - temp));
            URL.SetText(strchr(temp, '/'));
            URL.SetScroll(SCROLL_HORIZONTAL);

            InsertURL.ResetState();
        }

        if(InsertDEV.GetState() == STATE_CLICKED)
        {
            InsertDEV.ResetState();
            dev ^= 1;

            if(BrowseDevice(dev) < 0)
            {
                BrowseDevice();
                dev = 0;
            }

            DEV.SetText(mount[dev]);
            URL.SetText("");
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

                        if(strlen(browser.dir) > 1)
                            sprintf(fullpath, "%s%s/", rootdir, browser.dir+1); // print current path
                        else sprintf(fullpath, "%s", rootdir); // print current path

                        if (!wbl::copyString(temp, sizeof(temp), fullpath)) temp[0] = 0;
                        URL.SetText(strchr(temp, '/'));
					}
					else
					{
						menu = MENU_HOME;
						break;
					}
				}
				else
				{
					ShutoffRumble();
					// load file
					if(strlen(browser.dir) > 1)
                        sprintf(fullpath, "%s%s/%s", rootdir, browser.dir+1, browserList[browser.selIndex].filename); // print current path
                    else sprintf(fullpath, "%s%s", rootdir, browserList[browser.selIndex].filename); // print current path

                    if (!wbl::copyString(temp, sizeof(temp), fullpath)) temp[0] = 0;
                    URL.SetText(strchr(temp, '/'));
				}
			}
		}

		if(okBtn.GetState() == STATE_CLICKED)
		{
		    if (!wbl::copyString(path, capacity, temp)) {
                WindowPrompt("Path too long", "Choose a shorter destination.", "OK", NULL);
                continue;
            }
		    menu = MENU_HOME;
		}
        if(cancelBtn.GetState() == STATE_CLICKED)
		    menu = MENU_HOME;
	}

    fileBrowser.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
    buttonWindow.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
    while(!App_Exiting() && buttonWindow.GetEffect() > 0)
        usleep(100);

    fileBrowser.SetVisible(false);
    buttonWindow.SetVisible(false);

    if(mainWindow)
    {
        mainWindow->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 30);
        parentWindow->Append(mainWindow);
        while(!App_Exiting() && mainWindow->GetEffect() > 0)
            usleep(100);
    }

    HaltGui();
    parentWindow->Remove(&buttonWindow);
    parentWindow->Remove(&fileBrowser);
    ResumeGui();

    if (isValidPath(path))
        return true;
    return false;
}

bool AutoDownloader(char *path, size_t capacity)
{
    if (!path || !capacity) return false;
    path[0] = 0;
    int AbsolutePath = CheckFolder(Settings.DownloadFolder);

    if(AbsolutePath == RELATIVE)
        snprintf(path, capacity, "%s%s", Settings.BootDevice, Settings.DownloadFolder);
    else if(AbsolutePath == ABSOLUTE)
        wbl::copyString(path, capacity, Settings.DownloadFolder);

    if (isValidPath(path))
        return true;
    return false;
}

bool UnzipArchive(char *origfile)
{
    (void)origfile;
    // Extraction is unavailable until member containment and all output/CRC
    // checks have been hardened. Never claim success for unsupported formats.
    return false;
}

bool isValidPath(char *name)
{
    if (!name || !wbl::storagePath(name)) return false;
    bool ret = false;
    char *l, *path;
    if (!(l = strrchr(name, '/')))
        return ret;

    path = strndup(name, l+1-name);
    if(path && makedir(path)) // attempt to make dir
        ret = true;
    free(path);
    return ret;
}

int mymkdir(const char* dirname)
{
    int ret=0;
    ret = mkdir (dirname,0775);
    return ret;
}

int makedir (char *newdir)
{
    return CreateSubfolder(newdir) ? 1 : 0;
}

void show_mem()
{
#ifdef WBL_DEBUG_DIAGNOSTICS
    FILE *file = fopen("debug.txt", "a");
    if (!file) return;
    fprintf(file, "m1(%.4f) m2(%.4f)\r\n",
            ((float)((char*)SYS_GetArena1Hi()-(char*)SYS_GetArena1Lo()))/0x100000,
            ((float)((char*)SYS_GetArena2Hi()-(char*)SYS_GetArena2Lo()))/0x100000);
    fclose(file);
#endif
}

/****************************************************************************
 * CheckFile
 *
 * Check if file is existing
 ***************************************************************************/
bool CheckFile(const char * filepath)
{
    if(!filepath)
        return false;

    if(strchr(filepath, '/') == NULL)
        return false;

    struct stat filestat;
    int length = strlen(filepath);

    char * dirnoslash = (char *) malloc(length+2);
    if(!dirnoslash) return false;

    strcpy(dirnoslash, filepath);

    while(length > 1 && dirnoslash[length-1] == '/')
    {
        dirnoslash[length-1] = '\0';
        --length;
    }

	char * notRoot = strrchr(dirnoslash, '/');
	if(!notRoot)
	    strcat(dirnoslash, "/");

    int ret = stat(dirnoslash, &filestat);

    free(dirnoslash);

    return (ret == 0);
}

/****************************************************************************
 * CreateSubfolder
 *
 * Create recursive all subfolders to the given path
 ***************************************************************************/
bool CreateSubfolder(const char * fullpath)
{
    if (!fullpath || !wbl::storagePath(fullpath)) return false;
    const std::string path(fullpath);
    const size_t rootLength = path.compare(0, 4, "sd:/") == 0 ? 4 : 5;
    struct stat status;
    if (stat(path.substr(0, rootLength).c_str(), &status) != 0 || !S_ISDIR(status.st_mode)) return false;
    size_t position = rootLength;
    while (position < path.size()) {
        const size_t end = path.find('/', position);
        const std::string component = path.substr(0, end);
        if (stat(component.c_str(), &status) != 0) {
            if (errno != ENOENT || (mkdir(component.c_str(), 0777) != 0 && errno != EEXIST)) return false;
            if (stat(component.c_str(), &status) != 0) return false;
        }
        if (!S_ISDIR(status.st_mode)) return false;
        if (end == std::string::npos) break;
        position = end + 1;
    }
    return true;
}
