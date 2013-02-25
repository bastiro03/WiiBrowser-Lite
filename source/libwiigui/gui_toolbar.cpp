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

    imgToolbar = new GuiImageData(toolbar_png, toolbar_png_size);
    this->SetPosition(0,60);
    this->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);

    btnSound = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    Toolbar = new GuiImage(imgToolbar);
    trigA = new GuiTrigger();
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    imgFavorites = new GuiImageData(favorites_png);
    imgFavoritesOver = new GuiImageData(favorites_over_png);
    imgEdit = new GuiImageData(edit_png);
    imgEditOver = new GuiImageData(edit_over_png);

    if (buttons == NAVIGATION || buttons == HOMEPAGE)
    {
        imgWWW = new GuiImageData(www_png);
        imgWWWOver = new GuiImageData(www_over_png);
        imgSave = new GuiImageData(btn_save_png);
        imgSaveOver = new GuiImageData(btn_save_over_png);
        imgSaveFlat = new GuiImageData(btn_save_flat_png);

        imgSett = new GuiImageData(settings_png);
        imgSettOver = new GuiImageData(settings_over_png);
        imgSettFlat = new GuiImageData(settings_flat_png);

        imgHome = new GuiImageData(btn_home_png);
        imgHomeOver = new GuiImageData(btn_home_over_png);
        imgReload = new GuiImageData(reload_png);
        imgReloadOver = new GuiImageData(reload_over_png);
        imgReloadFlat = new GuiImageData(reload_flat_png);

        imgBack = new GuiImageData(toolbar_return_png);
        imgBackFlat = new GuiImageData(toolbar_return_flat_png);
        imgBackOver = new GuiImageData(toolbar_return_over_png);
        imgForward = new GuiImageData(toolbar_advance_png);
        imgForwardFlat = new GuiImageData(toolbar_advance_flat_png);
        imgForwardOver = new GuiImageData(toolbar_advance_over_png);

        Back = new GuiImage(imgBack);
        BackFlat = new GuiImage(imgBackFlat);
        BackOver = new GuiImage(imgBackOver);
        Forward = new GuiImage(imgForward);
        ForwardFlat = new GuiImage(imgForwardFlat);
        ForwardOver = new GuiImage(imgForwardOver);

        WWW = new GuiImage(imgWWW);
        WWWOver = new GuiImage(imgWWWOver);
        Save = new GuiImage(imgSave);
        SaveOver = new GuiImage(imgSaveOver);
        SaveFlat = new GuiImage(imgSaveFlat);

        Sett = new GuiImage(imgSett);
        SettOver = new GuiImage(imgSettOver);
        SettFlat = new GuiImage(imgSettFlat);

        Home = new GuiImage(imgHome);
        HomeOver = new GuiImage(imgHomeOver);
        Reload = new GuiImage(imgReload);
        ReloadOver = new GuiImage(imgReloadOver);
        ReloadFlat = new GuiImage(imgReloadFlat);

        btnBack = new GuiButton(Back->GetWidth(), Back->GetHeight());
        btnForward = new GuiButton(Forward->GetWidth(), Forward->GetHeight());
        btnWWW = new GuiButton(WWW->GetWidth(), WWW->GetHeight());
        btnSave = new GuiButton(Save->GetWidth(), Save->GetHeight());
        btnHome = new GuiButton(Home->GetWidth(), Home->GetHeight());
        btnReload = new GuiButton(Reload->GetWidth(), Reload->GetHeight());
        btnSett = new GuiButton(Sett->GetWidth(), Sett->GetHeight());

        BackTooltip = new GuiTooltip(gettext("Go back"));
        BackTooltip->SetOffset(15,-50);
        WWWTooltip = new GuiTooltip(gettext("Browse"));
        WWWTooltip->SetOffset(-15,-50);

        ForwardTooltip = new GuiTooltip(gettext("Go forward"));
        SaveTooltip = new GuiTooltip(gettext("Download"));
        HomeTooltip = new GuiTooltip(gettext("Homepage"));
        ReloadTooltip = new GuiTooltip(gettext("Refresh"));
        SettTooltip = new GuiTooltip(gettext("Settings"));

        btnBack->SetImage(Back);
        btnBack->SetImageDisabled(BackFlat);
        btnBack->SetImageOver(BackOver);
        btnBack->SetSoundOver(btnSound);
        btnBack->SetTrigger(trigA);
        btnBack->SetEffectGrow();
        btnBack->SetPosition(40,20);
        btnBack->SetTooltip(BackTooltip);

        btnForward->SetImage(Forward);
        btnForward->SetImageDisabled(ForwardFlat);
        btnForward->SetImageOver(ForwardOver);
        btnForward->SetSoundOver(btnSound);
        btnForward->SetTrigger(trigA);
        btnForward->SetEffectGrow();
        btnForward->SetPosition(40+Back->GetWidth(),20);
        btnForward->SetTooltip(ForwardTooltip);

        btnWWW->SetImage(WWW);
        btnWWW->SetImageOver(WWWOver);
        btnWWW->SetSoundOver(btnSound);
        btnWWW->SetState(STATE_SELECTED);
        btnWWW->SetTrigger(trigA);
        btnWWW->SetEffectGrow();
        btnWWW->SetPosition(screenwidth-WWW->GetWidth()-40,20);
        btnWWW->SetTooltip(WWWTooltip);

        btnSett->SetImage(Sett);
        btnSett->SetImageOver(SettOver);
        btnSett->SetImageDisabled(SettFlat);
        btnSett->SetSoundOver(btnSound);
        btnSett->SetTrigger(trigA);
        btnSett->SetEffectGrow();
        btnSett->SetPosition(WWW->GetLeft()-Sett->GetWidth()-10,20);
        btnSett->SetTooltip(SettTooltip);

        btnHome->SetImage(Home);
        btnHome->SetImageOver(HomeOver);
        btnHome->SetSoundOver(btnSound);
        btnHome->SetTrigger(trigA);
        btnHome->SetEffectGrow();
        btnHome->SetPosition(Sett->GetLeft()-Home->GetWidth()-10,20);
        btnHome->SetTooltip(HomeTooltip);

        btnSave->SetImage(Save);
        btnSave->SetImageDisabled(SaveFlat);
        btnSave->SetImageOver(SaveOver);
        btnSave->SetSoundOver(btnSound);
        btnSave->SetTrigger(trigA);
        btnSave->SetEffectGrow();
        btnSave->SetPosition(Home->GetLeft()-Save->GetWidth()-10,20);
        btnSave->SetTooltip(SaveTooltip);

        btnReload->SetImage(Reload);
        btnReload->SetImageDisabled(ReloadFlat);
        btnReload->SetImageOver(ReloadOver);
        btnReload->SetSoundOver(btnSound);
        btnReload->SetTrigger(trigA);
        btnReload->SetEffectGrow();
        btnReload->SetPosition(Save->GetLeft()-Reload->GetWidth()-10,20);
        btnReload->SetTooltip(ReloadTooltip);
    }

    if (buttons == HOMEPAGE)
    {
        btnReload->SetState(STATE_DISABLED);
        btnSave->SetState(STATE_DISABLED);
    }

    if (buttons == NAVIGATION)
    {
        btnSett->SetState(STATE_DISABLED);
    }

    width = screenwidth;
	height = Toolbar->GetHeight();
    Toolbar->SetPosition(0,0);

    this->Append(Toolbar);

    if (buttons == NAVIGATION || buttons == HOMEPAGE)
    {
        this->Append(btnWWW);
        this->Append(btnSett);
        this->Append(btnSave);
        this->Append(btnBack);
        this->Append(btnForward);
        this->Append(btnHome);
        this->Append(btnReload);
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

    delete(imgFavorites);
    delete(imgFavoritesOver);
    delete(imgEdit);
    delete(imgEditOver);

    delete(imgWWW);
    delete(imgWWWOver);
    delete(imgSave);
    delete(imgSaveFlat);
    delete(imgSaveOver);

    delete(WWW);
    delete(WWWOver);
    delete(Save);
    delete(SaveFlat);
    delete(SaveOver);

    delete(btnWWW);
    delete(btnSave);

    delete(imgBack);
    delete(imgBackFlat);
    delete(imgBackOver);
    delete(imgForward);
    delete(imgForwardFlat);
    delete(imgForwardOver);

    delete(imgHome);
    delete(imgHomeOver);
    delete(imgReload);
    delete(imgReloadFlat);
    delete(imgReloadOver);

    delete(imgSett);
    delete(imgSettFlat);
    delete(imgSettOver);

    delete(Back);
    delete(BackFlat);
    delete(BackOver);
    delete(Forward);
    delete(ForwardFlat);
    delete(ForwardOver);

    delete(Sett);
    delete(SettFlat);
    delete(SettOver);

    delete(Home);
    delete(HomeOver);
    delete(Reload);
    delete(ReloadFlat);
    delete(ReloadOver);

    delete(btnBack);
    delete(btnForward);
    delete(btnHome);
    delete(btnReload);
    delete(btnSett);

    delete(HomeTooltip);
    delete(ReloadTooltip);
    delete(SettTooltip);
    delete(BackTooltip);
    delete(ForwardTooltip);
    delete(WWWTooltip);
}

void GuiToolbar::ChangeButtons(int set)
{
    if (buttons == set)
        return;
    buttons = set;

    if (buttons == HOMEPAGE)
    {
        Save->SetImage(imgSave);
        SaveOver->SetImage(imgSaveOver);
        // Reload->SetImage(imgReload);
        // ReloadOver->SetImage(imgReloadOver);

        SaveTooltip->SetText(gettext("Download"));
        // ReloadTooltip->SetText(gettext("Refresh"));

        btnSave->SetState(STATE_DISABLED);
        // btnReload->SetState(STATE_DISABLED);
    }

    else if (buttons == FAVORITES)
    {
        Save->SetImage(imgFavorites);
        SaveOver->SetImage(imgFavoritesOver);
        // Reload->SetImage(imgEdit);
        // ReloadOver->SetImage(imgEditOver);

        SaveTooltip->SetText(gettext("Add bookmark"));
        // ReloadTooltip->SetText(gettext("Edit"));

        btnSave->SetState(STATE_DEFAULT);
        // btnReload->SetState(STATE_DEFAULT);
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
