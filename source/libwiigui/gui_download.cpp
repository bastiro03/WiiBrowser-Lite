/****************************************************************************
 * WiiBrowser
 *
 * gave92 2012
 *
 * gui_download.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "main.h"
#include "networkop.h"
#include "gui.h"

#define MAXB    5
#define OFF     55
#define BASE    140

/**
 * Constructor for the GuiDownloadManager class.
 */
GuiDownloadManager::GuiDownloadManager()
{
	width = 548;
	height = 388;
	focus = 0; // allow focus

	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;
	this->SetPosition(0, -10);

	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	btnOutline = new GuiImageData(remove_png);
	btnOutlineOver = new GuiImageData(remove_over_png);

	dialogBox = new GuiImageData(downloads_box_png);
	dialogBoxImg = new GuiImage(dialogBox);

	progressLeft = new GuiImageData(progressbar_left_png);
	progressMid = new GuiImageData(progressbar_mid_png);
	progressRight = new GuiImageData(progressbar_right_png);
	progressLine = new GuiImageData(progressbar_line_png);
	progressEmpty = new GuiImageData(progressbar_empty_png);

	baroffset = 548/2 - progressEmpty->GetWidth()/2 - btnOutline->GetWidth()/2;
	maxtile = (progressEmpty->GetWidth()-16)/4;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	titleTxt = new GuiText("Downloads", 26, (GXColor){0, 0, 0, 255});
	titleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt->SetPosition(0,40);

	this->Append(dialogBoxImg);
	this->Append(titleTxt);

    for(int i = 0; i<MAX_DOWNLOADS; i++)
    {
        progressEmptyImg[i] = new GuiImage(progressEmpty);
        progressEmptyImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        progressEmptyImg[i]->SetPosition(baroffset, 0);

        progressEmptyImg[i]->SetForce(true);
        progressEmptyImg[i]->SetVisible(false);

        progressLeftImg[i] = new GuiImage(progressLeft);
        progressLeftImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        progressLeftImg[i]->SetPosition(baroffset, 0);

        progressLeftImg[i]->SetForce(true);
        progressLeftImg[i]->SetVisible(false);

        progressMidImg[i] = new GuiImage(progressMid);
        progressMidImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        progressMidImg[i]->SetPosition(baroffset+8, 0);
        progressMidImg[i]->SetTile(0);

        progressMidImg[i]->SetForce(true);
        progressMidImg[i]->SetVisible(false);

        progressLineImg[i] = new GuiImage(progressLine);
        progressLineImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        progressLineImg[i]->SetPosition(baroffset+8, 0);

        progressLineImg[i]->SetForce(true);
        progressLineImg[i]->SetVisible(false);

        progressRightImg[i] = new GuiImage(progressRight);
        progressRightImg[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
        progressRightImg[i]->SetPosition(-baroffset - btnOutline->GetWidth(), 0);

        progressRightImg[i]->SetForce(true);
        progressRightImg[i]->SetVisible(false);

        downloads[i] = new GuiText("", 20, (GXColor){0, 0, 0, 255});
        downloads[i]->SetMaxWidth(270);
        downloads[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        downloads[i]->SetScroll(SCROLL_DOTTED);
        downloads[i]->SetVisible(false);

        progress[i] = new GuiText("", 20, (GXColor){0, 0, 0, 255});
        progress[i]->SetMaxWidth(100);
        progress[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
        progress[i]->SetVisible(false);

        cancelBtnImg[i] = new GuiImage(btnOutline);
        cancelBtnImgOver[i] = new GuiImage(btnOutlineOver);
        cancelBtn[i] = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
        cancelBtn[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
        cancelBtn[i]->SetScale(0.75);

        cancelBtn[i]->SetImage(cancelBtnImg[i]);
        cancelBtn[i]->SetImageOver(cancelBtnImgOver[i]);
        cancelBtn[i]->SetSoundOver(btnSoundOver);
        cancelBtn[i]->SetTrigger(trigA);
        cancelBtn[i]->SetEffectGrow();
        cancelBtn[i]->SetVisible(false);

        this->Append(downloads[i]);
        this->Append(progress[i]);
        this->Append(cancelBtn[i]);

        this->Append(progressEmptyImg[i]);
        this->Append(progressLeftImg[i]);
        this->Append(progressMidImg[i]);
        this->Append(progressLineImg[i]);
        this->Append(progressRightImg[i]);
    }
}

/**
 * Destructor for the GuiDownloadManager class.
 */
GuiDownloadManager::~GuiDownloadManager()
{
    delete(progressLeft);
    delete(progressMid);
    delete(progressRight);
    delete(progressEmpty);
    delete(progressLine);
    delete(btnOutline);
    delete(btnOutlineOver);
    delete(dialogBox);

    delete(dialogBoxImg);
    delete(titleTxt);
    delete(trigA);
    delete(btnSoundOver);

    for(int i = 0; i<MAX_DOWNLOADS; i++)
    {
        delete(progressEmptyImg);
        delete(progressLeftImg);
        delete(progressMidImg);
        delete(progressLineImg);
        delete(progressRightImg);

        delete(downloads[i]);
        delete(progress[i]);

        delete(cancelBtn[i]);
        delete(cancelBtnImg[i]);
        delete(cancelBtnImgOver[i]);
    }
}

void GuiDownloadManager::SetProgress(void *p, float t)
{
    char msg[50];
    char prg[10];
    Private *data = (Private *)p;

    char *name = data->save.name;
    snprintf(msg, 50, "%s", name);
    snprintf(prg, 10, "<%2.2f%%>", t);

    int tile = 0;
    int d = *(data->bar);

    if(t > 2.0)
    {
        progressLeftImg[d]->SetVisible(true);
        tile = (maxtile+2)*(t-2.0)/100;
        if(tile > maxtile) tile = maxtile;
        progressMidImg[d]->SetTile(tile);
        progressLineImg[d]->SetXPosition(baroffset+8 + tile*4);
        progressLineImg[d]->SetVisible(true);
    }

    if(tile == maxtile)
    {
        progressLineImg[d]->SetVisible(false);
        progressRightImg[d]->SetVisible(true);
    }

    if(downloads[d])
        downloads[d]->SetText(msg);
    if(progress[d])
        progress[d]->SetText(prg);
}

bool GuiDownloadManager::CancelDownload(int *d)
{
    if (!cancelBtn[*d])
        return false;

    return (cancelBtn[*d]->GetState() == STATE_CLICKED);
}

int* GuiDownloadManager::CreateBar()
{
    int *ret = (int *)malloc(sizeof(int));

    for (*ret = 0; *ret<MAXB; (*ret)++)
    {
        if(!downloads[*ret]->IsVisible())
            break;
    }

    if(*ret == MAXB)
    {
        free(ret);
        return NULL;
    }

    int top = 0;
    for (int j = 0; j<MAXB; j++)
    {
        if(downloads[j]->IsVisible())
            top += 1;
    }

    downloads[*ret]->SetPosition(20, (top-1)*OFF + BASE);
    downloads[*ret]->SetVisible(true);
    progress[*ret]->SetPosition(-20, (top-1)*OFF + BASE);
    progress[*ret]->SetVisible(true);

    cancelBtn[*ret]->SetPosition(-30, (top-1)*OFF + BASE+17);
    cancelBtn[*ret]->SetVisible(true);
    cancelBtn[*ret]->SetState(STATE_DEFAULT);

    progressLeftImg[*ret]->SetYPosition((top-1)*OFF + BASE+30);
    progressLineImg[*ret]->SetYPosition((top-1)*OFF + BASE+30);
    progressMidImg[*ret]->SetYPosition((top-1)*OFF + BASE+30);
    progressRightImg[*ret]->SetYPosition((top-1)*OFF + BASE+30);
    progressEmptyImg[*ret]->SetYPosition((top-1)*OFF + BASE+30);

    progressMidImg[*ret]->SetTile(0);
    progressMidImg[*ret]->SetVisible(true);
    progressEmptyImg[*ret]->SetVisible(true);
    return ret;
}

void GuiDownloadManager::RemoveBar(int *i)
{
    downloads[*i]->SetVisible(false);
    progress[*i]->SetVisible(false);
    cancelBtn[*i]->SetVisible(false);

    progressEmptyImg[*i]->SetVisible(false);
    progressLeftImg[*i]->SetVisible(false);
    progressMidImg[*i]->SetVisible(false);
    progressLineImg[*i]->SetVisible(false);
    progressRightImg[*i]->SetVisible(false);

    int to_d, to_b, to_p;
    int wait = -1;

    for (int j = 0; j<MAXB; j++)
    {
        if(downloads[j]->IsVisible() && downloads[j]->GetYPosition()>downloads[*i]->GetYPosition())
        {
            to_d = downloads[j]->GetYPosition()-OFF;
            to_b = cancelBtn[j]->GetYPosition()-OFF;
            to_p = progressEmptyImg[j]->GetYPosition()-OFF;
            wait = j;

            downloads[j]->SetEffect(EFFECT_SLIDE_TO, 1, to_d);
            progress[j]->SetEffect(EFFECT_SLIDE_TO, 1, to_d);
            cancelBtn[j]->SetEffect(EFFECT_SLIDE_TO, 1, to_b);

            progressEmptyImg[j]->SetEffect(EFFECT_SLIDE_TO, 1, to_p);
            progressLeftImg[j]->SetEffect(EFFECT_SLIDE_TO, 1, to_p);
            progressMidImg[j]->SetEffect(EFFECT_SLIDE_TO, 1, to_p);
            progressLineImg[j]->SetEffect(EFFECT_SLIDE_TO, 1, to_p);
            progressRightImg[j]->SetEffect(EFFECT_SLIDE_TO, 1, to_p);
        }
    }

    if(wait >= 0)
    {
        while (progressRightImg[wait]->GetEffect() > 0)
            usleep(100);
    }
    free(i);
}

void GuiDownloadManager::Update(GuiTrigger * t)
{
	if(_elements.size() == 0 || (state == STATE_DISABLED && parentElement))
		return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try	{ _elements.at(i)->Update(t); }
		catch (const std::exception& e) { }
	}

	this->ToggleFocus(t);

	if(focus) // only send actions to this window if it's in focus
	{
		// pad/joystick navigation
		if(t->Right())
			this->MoveSelectionHor(1);
		else if(t->Left())
			this->MoveSelectionHor(-1);
		else if(t->Down())
			this->MoveSelectionVert(1);
		else if(t->Up())
			this->MoveSelectionVert(-1);
	}
}
