#include <stdio.h>
#include <stdlib.h>

#include "fileop.h"
#include "filebrowser.h"
#include "archiveoperations/archive.h"

bool GuiBrowser(GuiWindow *mainWindow, GuiWindow *parentWindow, char *path, const char *label)
{
	char temp[256];
	char title[100];
	int i;

	ShutoffRumble();

	// populate initial directory listing
	if (BrowseDevice() <= 0)
	{
		WindowPrompt(
			"Error",
			"Unable to display files on selected load device",
			"Ok",
			nullptr);

		return false;
	}

	int menu = MENU_NONE;
	int dev = 0;
	char mount[2][5] = {"SD", "USB"};

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	if (OpenDefaultFolder() <= 0)
	{
		BrowseDevice();
		bzero(temp, sizeof(temp));
	}
	else
		sprintf(temp, "%s/", Settings.UserFolder);

	sprintf(title, "Browse files");
	bzero(path, sizeof(path));

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData Textbox(textbox_end_png);
	GuiImage TextboxImg(&Textbox);
	GuiButton InsertURL(TextboxImg.GetWidth(), TextboxImg.GetHeight());

	GuiImageData Device(textbox_begin_png);
	GuiImage DeviceImg(&Device);
	GuiButton InsertDEV(DeviceImg.GetWidth(), DeviceImg.GetHeight());

	GuiText URL(strchr(temp, '/'), 20, (GXColor){0, 0, 0, 255})
		GuiText DEV("SD", 20, (GXColor){0, 0, 0, 255})

			URL.SetMaxWidth(TextboxImg.GetWidth() - 20);
	URL.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	URL.SetPosition(5, 0);
	URL.SetScroll(SCROLL_HORIZONTAL);

	InsertURL.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	InsertURL.SetPosition(InsertDEV.GetWidth() / 2, 30);
	InsertURL.SetLabel(&URL);
	InsertURL.SetImage(&TextboxImg);
	InsertURL.SetSoundOver(&btnSoundOver);
	InsertURL.SetTrigger(&trigA);

	InsertDEV.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	InsertDEV.SetPosition(InsertURL.GetLeft() - InsertDEV.GetWidth() / 2, 30);
	InsertDEV.SetLabel(&DEV);
	InsertDEV.SetImage(&DeviceImg);
	InsertDEV.SetSoundOver(&btnSoundOver);
	InsertDEV.SetTrigger(&trigA);
	InsertDEV.SetEffectGrow();

	GuiText titleTxt(title, 28, (GXColor){0, 0, 0, 255})
		titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, 50);

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 108);
	fileBrowser.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 30);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiText okBtnTxt(label, 24, (GXColor){0, 0, 0, 255})
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

	GuiText cancelBtnTxt("Cancel", 24, (GXColor){0, 0, 0, 255})
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
		while (mainWindow->GetEffect() > 0)
			usleep(100);
	}

	HaltGui();
	if (mainWindow)
		parentWindow->Remove(mainWindow);
	parentWindow->Append(&buttonWindow);
	parentWindow->Append(&fileBrowser);
	ResumeGui();

	while (menu == MENU_NONE)
	{
		usleep(100);
		if (!strlen(URL.GetText()) || URL.GetText()[0] != '/')
		{
			sprintf(temp, "%s", rootdir);
			URL.SetText(strchr(temp, '/'));
		}

		if (InsertURL.GetState() == STATE_CLICKED)
		{
			URL.SetScroll(SCROLL_NONE);
			OnScreenKeyboard(parentWindow, strchr(temp, '/'), 256);
			URL.SetText(strchr(temp, '/'));
			URL.SetScroll(SCROLL_HORIZONTAL);

			InsertURL.ResetState();
		}

		if (InsertDEV.GetState() == STATE_CLICKED)
		{
			InsertDEV.ResetState();
			dev ^= 1;

			if (BrowseDevice(dev) <= 0)
			{
				BrowseDevice();
				dev = 0;
			}

			DEV.SetText(mount[dev]);
			URL.SetText("");
		}

		// update file browser based on arrow buttons
		// set MENU_EXIT if A button pressed on a file
		for (i = 0; i < FILE_PAGESIZE; i++)
		{
			if (fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();
				// check corresponding browser entry
				if (browserList[browser.selIndex].isdir)
				{
					if (BrowserChangeFolder())
					{
						fileBrowser.ResetState();
						fileBrowser.fileList[0]->SetState(STATE_SELECTED);
						fileBrowser.TriggerUpdate();

						if (strlen(browser.dir) > 1)
							sprintf(fullpath, "%s%s/", rootdir, browser.dir + 1); // print current path
						else
							sprintf(fullpath, "%s", rootdir); // print current path

						sprintf(temp, fullpath);
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
					if (strlen(browser.dir) > 1)
						sprintf(fullpath, "%s%s/%s", rootdir, browser.dir + 1, browserList[browser.selIndex].filename);
					// print current path
					else
						sprintf(fullpath, "%s%s", rootdir, browserList[browser.selIndex].filename);
					// print current path

					sprintf(temp, fullpath);
					URL.SetText(strchr(temp, '/'));
				}
			}
		}

		if (okBtn.GetState() == STATE_CLICKED)
		{
			sprintf(path, temp);
			menu = MENU_HOME;
		}
		if (cancelBtn.GetState() == STATE_CLICKED)
			menu = MENU_HOME;
	}

	fileBrowser.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
	buttonWindow.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
	while (buttonWindow.GetEffect() > 0)
		usleep(100);

	fileBrowser.SetVisible(false);
	buttonWindow.SetVisible(false);

	if (mainWindow)
	{
		mainWindow->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 30);
		parentWindow->Append(mainWindow);
		while (mainWindow->GetEffect() > 0)
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

bool AutoDownloader(char *path)
{
	int AbsolutePath = CheckFolder(Settings.DownloadFolder);

	if (AbsolutePath == RELATIVE)
		sprintf(path, "%s%s", Settings.BootDevice, Settings.DownloadFolder);
	else if (AbsolutePath == ABSOLUTE)
		sprintf(path, Settings.DownloadFolder);

	if (isValidPath(path))
		return true;
	return false;
}

bool UnzipArchive(char *origfile)
{
	char zipfilepath[512];
	strcpy(zipfilepath, origfile);

	ArchiveHandle archive(zipfilepath);

	char *unzipfolder = strrchr(zipfilepath, '/');
	if (unzipfolder)
		unzipfolder[1] = 0;

	return (archive.ExtractAll(zipfilepath) > 0);
}

bool isValidPath(char *name)
{
	bool ret = false;
	char *l, *path;
	if (!(l = strrchr(name, '/')))
		return ret;

	path = strndup(name, l + 1 - name);
	if (makedir(path)) // attempt to make dir
		ret = true;
	free(path);
	return ret;
}

int mymkdir(const char *dirname)
{
	int ret = 0;
	ret = mkdir(dirname, 0775);
	return ret;
}

int makedir(char *newdir)
{
	char *buffer;
	char *p;
	int len = static_cast<int>(strlen(newdir));

	if (len <= 0)
		return 0;

	buffer = static_cast<char *>(malloc(len + 1));
	strcpy(buffer, newdir);

	if (buffer[len - 1] == '/')
	{
		buffer[len - 1] = '\0';
	}
	if (mymkdir(buffer) == 0)
	{
		free(buffer);
		return 1;
	}

	p = buffer + 1;
	while (true)
	{
		char hold;

		while (*p && *p != '\\' && *p != '/')
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

void show_mem()
{
	FILE *file = fopen("debug.txt", "a");
	fprintf(file, "m1(%.4f) m2(%.4f)\r\n",
			static_cast<float>(static_cast<char *>(SYS_GetArena1Hi()) - static_cast<char *>(SYS_GetArena1Lo())) /
				0x100000,
			static_cast<float>(static_cast<char *>(SYS_GetArena2Hi()) - static_cast<char *>(SYS_GetArena2Lo())) /
				0x100000);
	fclose(file);
}

/****************************************************************************
 * CheckFile
 *
 * Check if file is existing
 ***************************************************************************/
bool CheckFile(const char *filepath)
{
	if (!filepath)
		return false;

	if (strchr(filepath, '/') == nullptr)
		return false;

	struct stat filestat;
	int length = strlen(filepath);

	auto dirnoslash = static_cast<char *>(malloc(length + 2));
	if (!dirnoslash)
		return false;

	strcpy(dirnoslash, filepath);

	while (dirnoslash[length - 1] == '/')
	{
		dirnoslash[length - 1] = '\0';
		--length;
	}

	char *notRoot = strrchr(dirnoslash, '/');
	if (!notRoot)
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
bool CreateSubfolder(const char *fullpath)
{
	if (!fullpath)
		return false;

	if (CheckFile(fullpath))
		return true;

	string dirpath(fullpath);

	int length = dirpath.size() - 1;
	while (dirpath[length] == '/')
	{
		dirpath.erase(length);
		--length;
	}

	string subpath(dirpath);
	size_t pos = subpath.rfind('/');
	if (pos == string::npos)
		return false;

	if (subpath.size() - 1 > pos)
		subpath.erase(pos + 1);

	bool result = CreateSubfolder(subpath.c_str());
	if (!result)
		return false;

	return (mkdir(dirpath.c_str(), 0777) != -1);
}
