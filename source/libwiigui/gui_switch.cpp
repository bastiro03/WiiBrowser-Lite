/****************************************************************************
 * WiiBrowser
 *
 * gave92 2012
 *
 * gui_switch.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "main.h"
#include "gui.h"

/**
 * Constructor for the GuiSwitch class.
 */
GuiSwitch::GuiSwitch(int d)
{
    direction = d;
    focus = 0; // allow focus
	selectable = true;

    btnSound = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    trigA = new GuiTrigger();
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	dataMore = new GuiImageData(more_png);
    imgMore = new GuiImage(dataMore);
    imgMore->SetAlpha(100);

    if(direction == 0)
    {
        dataDef = new GuiImageData(arrow_r_png);
        imgDef = new GuiImage(dataDef);
        dataSel = new GuiImageData(arrow_r_over_png);
        imgSel = new GuiImage(dataSel);

        imgMore->SetPosition(-dataDef->GetWidth()+11,-8);
        imgSel->SetPosition(-dataDef->GetWidth()+11,-8);
        Button = new GuiButton(dataDef->GetWidth(), dataDef->GetHeight());
        this->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
    }

    if(direction == 1)
    {
        dataDef = new GuiImageData(arrow_l_png);
        imgDef = new GuiImage(dataDef);
        dataSel = new GuiImageData(arrow_l_over_png);
        imgSel = new GuiImage(dataSel);

        imgMore->SetPosition(dataDef->GetWidth()-27,-8);
        imgSel->SetPosition(dataDef->GetWidth()-27,-8);
        Button = new GuiButton(dataDef->GetWidth(), dataDef->GetHeight());
        this->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    }

    Button->SetImage(imgDef);
    Button->SetImageOver(imgDef);
    Button->SetImageOver(imgSel,1);
    Button->SetImageClick(imgMore);
    Button->SetSoundOver(btnSound);
    Button->SetTrigger(trigA);
    Button->SetEffect(EFFECT_MOVE,1,1);
    Button->SetEffectGrow();

    width = dataSel->GetWidth();
	height = dataSel->GetHeight();
    this->Append(Button);
}

/**
 * Destructor for the GuiSwitch class.
 */
GuiSwitch::~GuiSwitch()
{
    delete(dataDef);
    delete(dataSel);
    delete(dataMore);

    delete(imgDef);
    delete(imgSel);
    delete(imgMore);

    delete(btnSound);
    delete(trigA);
    delete(Button);
}

void GuiSwitch::Update(GuiTrigger * t)
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
