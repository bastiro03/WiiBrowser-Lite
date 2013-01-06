/****************************************************************************
 * libwiigui
 *
 * gave92 2012
 *
 * gui_toolbar.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "gettext.h"

/**
 * Constructor for the GuiToolbar class.
 */
GuiToolbar::GuiToolbar(int set)
{
    buttons = set;
	focus = 0; // allow focus
	selectable = true;
    this->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    this->SetPosition(0, 60);

    imgToolbar = new GuiImageData(toolbar_png, toolbar_png_size);
    btnSound = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    Toolbar = new GuiImage(imgToolbar);
    trigA = new GuiTrigger();
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    if (buttons == NAVIGATION)
    {
        imgWWW = new GuiImageData(www_png, www_png_size);
        imgWWWOver = new GuiImageData(www_over_png, www_over_png_size);
        imgSave = new GuiImageData(btn_save_png, btn_save_png_size);
        imgSaveOver = new GuiImageData(btn_save_over_png, btn_save_over_png_size);

        imgBack = new GuiImageData(toolbar_return_png, toolbar_return_png_size);
        imgBackOver = new GuiImageData(toolbar_return_over_png, toolbar_return_over_png_size);
        imgForward = new GuiImageData(toolbar_advance_png, toolbar_advance_png_size);
        imgForwardOver = new GuiImageData(toolbar_advance_over_png, toolbar_advance_over_png_size);

        Back = new GuiImage(imgBack);
        BackOver = new GuiImage(imgBackOver);
        Forward = new GuiImage(imgForward);
        ForwardOver = new GuiImage(imgForwardOver);

        WWW = new GuiImage(imgWWW);
        WWWOver = new GuiImage(imgWWWOver);
        Save = new GuiImage(imgSave);
        SaveOver = new GuiImage(imgSaveOver);

        btnBack = new GuiButton(Back->GetWidth(), Back->GetHeight());
        btnForward = new GuiButton(Forward->GetWidth(), Forward->GetHeight());
        btnWWW = new GuiButton(WWW->GetWidth(), WWW->GetHeight());
        btnSave = new GuiButton(Save->GetWidth(), Save->GetHeight());

        BackTooltip = new GuiTooltip(gettext("Go back"));
        BackTooltip->SetOffset(-15,-50);

        ForwardTooltip = new GuiTooltip(gettext("Go forward"));
        WWWTooltip = new GuiTooltip(gettext("Homepage"));
        SaveTooltip = new GuiTooltip(gettext("Download"));

        btnBack->SetImage(Back);
        btnBack->SetImageOver(BackOver);
        btnBack->SetSoundOver(btnSound);
        btnBack->SetTrigger(trigA);
        btnBack->SetEffectGrow();
        btnBack->SetPosition(20,20);
        btnBack->SetTooltip(BackTooltip);

        btnForward->SetImage(Forward);
        btnForward->SetImageOver(ForwardOver);
        btnForward->SetSoundOver(btnSound);
        btnForward->SetTrigger(trigA);
        btnForward->SetEffectGrow();
        btnForward->SetPosition(20+Back->GetWidth(),20);
        btnForward->SetTooltip(ForwardTooltip);

        btnWWW->SetImage(WWW);
        btnWWW->SetImageOver(WWWOver);
        btnWWW->SetSoundOver(btnSound);
        btnWWW->SetState(STATE_SELECTED);
        btnWWW->SetTrigger(trigA);
        btnWWW->SetEffectGrow();
        btnWWW->SetPosition(screenwidth-WWW->GetWidth()-20,20);
        btnWWW->SetTooltip(WWWTooltip);

        btnSave->SetImage(Save);
        btnSave->SetImageOver(SaveOver);
        btnSave->SetSoundOver(btnSound);
        btnSave->SetTrigger(trigA);
        btnSave->SetEffectGrow();
        btnSave->SetPosition(WWW->GetLeft()-Save->GetWidth()-15,20);
        btnSave->SetTooltip(SaveTooltip);
    }

    width = screenwidth;
	height = Toolbar->GetHeight();
    Toolbar->SetPosition(0,0);

    this->Append(Toolbar);

    if (buttons == NAVIGATION)
    {
        this->Append(btnWWW);
        this->Append(btnSave);
        this->Append(btnBack);
        this->Append(btnForward);
    }
}

/**
 * Destructor for the GuiToolbar class.
 */
GuiToolbar::~GuiToolbar()
{
    delete(imgToolbar);
    delete(btnSound);
    delete(trigA);
    delete(Toolbar);

    if (buttons == NAVIGATION)
    {
        delete(imgWWW);
        delete(imgWWWOver);
        delete(imgSave);
        delete(imgSaveOver);

        delete(imgBack);
        delete(imgBackOver);
        delete(imgForward);
        delete(imgForwardOver);

        delete(Back);
        delete(BackOver);
        delete(Forward);
        delete(ForwardOver);

        delete(WWW);
        delete(WWWOver);
        delete(Save);
        delete(SaveOver);

        delete(btnBack);
        delete(btnForward);
        delete(btnWWW);
        delete(btnSave);

        delete(BackTooltip);
        delete(ForwardTooltip);
        delete(WWWTooltip);
    }
}

void GuiToolbar::Update(GuiTrigger * t)
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
