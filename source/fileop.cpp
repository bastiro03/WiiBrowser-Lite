#include <stdio.h>
#include <stdlib.h>

#include "fileop.h"
#include "filebrowser.h"

bool GuiBrowser(GuiWindow *mainWindow, GuiWindow *parentWindow, char *path)
{
    char temp[256];
    char title[100];
	int i;

	ShutoffRumble();

	// populate initial directory listing
	if(BrowseDevice() <= 0)
	{
		WindowPrompt(
		"Error",
		"Unable to display files on selected load device.",
		"Ok",
		NULL);

        return false;
	}

	int menu = MENU_NONE;
    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	sprintf(title, "Browse files");
	bzero(temp, sizeof(temp));
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
    InsertURL.SetTrigger(&trigA);
    InsertURL.SetEffectGrow();

    GuiText titleTxt(title, 28, (GXColor){0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 108);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiText okBtnTxt("Take screenshot", 24, (GXColor){0, 0, 0, 255});
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
	buttonWindow.Append(&fileBrowser);
	buttonWindow.Append(&InsertURL);

    buttonWindow.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 30);
    mainWindow->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(mainWindow->GetEffect() > 0)
        usleep(100);

    HaltGui();
    parentWindow->Remove(mainWindow);
    parentWindow->Append(&buttonWindow);
    parentWindow->ChangeFocus(&buttonWindow);
    ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(100);

        if(InsertURL.GetState() == STATE_CLICKED)
        {
            URL.SetScroll(SCROLL_NONE);
            OnScreenKeyboard(parentWindow, temp, 256);
            URL.SetText(temp);
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
						sprintf(temp, fullpath);
						URL.SetText(fullpath);
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
					sprintf(fullpath, "%s%s/%s", rootdir, browser.dir+1, browserList[browser.selIndex].filename); // print current path
					sprintf(temp, fullpath);
					URL.SetText(fullpath);
				}
			}
		}

		if(okBtn.GetState() == STATE_CLICKED)
		{
		    sprintf(path, temp);
		    menu = MENU_HOME;
		}
        if(cancelBtn.GetState() == STATE_CLICKED)
		    menu = MENU_HOME;
	}

    buttonWindow.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
    mainWindow->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 30);
    while(buttonWindow.GetEffect() > 0)
        usleep(100);

    HaltGui();
    parentWindow->Remove(&buttonWindow);
    parentWindow->Append(mainWindow);
    parentWindow->ChangeFocus(mainWindow);
    ResumeGui();

    if (isValidPath(path))
        return true;
    return false;
}

FILE *SelectFile(GuiWindow *mainWindow, char *type)
{
    const char *c;
    char path[256];
    snprintf(path, 256, Settings.UserFolder);
    OnScreenKeyboard(mainWindow, path, 256);
    if ((c = mime2ext(type)))
        strcat(path, c);
    FILE *file = NULL;
    if (isValidPath(path))
        file = fopen(path, "wb");
    return file;
}

bool SelectPath(GuiWindow *mainWindow, char *path)
{
    snprintf(path, 250, Settings.UserFolder);
    OnScreenKeyboard(mainWindow, path, 250);
    strcat(path, ".png");
    if (isValidPath(path))
        return true;
    return false;
}

bool isValidPath(char *name)
{
    char *l, *path;
    l = strrchr(name, '/');
    if (strlen(name) > 0) {
        if (l == NULL) return true;
        path = strndup(name, l+1-name);
        if(!strchr("./", *(l+1))) {
            makedir(path); // attempt to make dir
            return true;
        }
    }
    return false;
}

static int mymkdir(const char* dirname)
{
    int ret=0;
    ret = mkdir (dirname,0775);
    return ret;
}

int makedir (char *newdir)
{
    char *buffer ;
    char *p;
    int  len = (int)strlen(newdir);

    if (len <= 0)
        return 0;

    buffer = (char*)malloc(len+1);
    strcpy(buffer,newdir);

    if (buffer[len-1] == '/') {
        buffer[len-1] = '\0';
    }
    if (mymkdir(buffer) == 0)
    {
        free(buffer);
        return 1;
    }

    p = buffer+1;
    while (1)
    {
        char hold;

        while(*p && *p != '\\' && *p != '/')
            p++;
        hold = *p;
        *p = 0;
        if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
            free(buffer);
            return 0;
        }
        if (hold == 0)
            break;
        *p++ = hold;
    }
    free(buffer);
    return 1;
}