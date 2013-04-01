/****************************************************************************
 * libwiigui
 * WiiBrowser
 *
 * Tantric 2009
 * modified by gave92
 *
 * gui_tooltip.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

static GuiImageData *tooltipLeft = NULL;
static GuiImageData *tooltipTile = NULL;
static GuiImageData *tooltipRight = NULL;

/**
 * Constructor for the GuiTooltip class.
 */
GuiTooltip::GuiTooltip(const char *t)
{
	if(tooltipLeft == NULL)
	{
		tooltipLeft = new GuiImageData(tooltip_left_png);
		tooltipTile = new GuiImageData(tooltip_tile_png);
		tooltipRight = new GuiImageData(tooltip_right_png);
	}

	leftImage.SetImage(tooltipLeft);
	tileImage.SetImage(tooltipTile);
	rightImage.SetImage(tooltipRight);

	height = leftImage.GetHeight();
    offsetHr = 0;
	offsetVr = -50;

	text = NULL;
	if(t)
		SetText(t);
    this->SetAlignment(ALIGN_CENTRE,ALIGN_TOP);

	leftImage.SetParent(this);
	tileImage.SetParent(this);
	rightImage.SetParent(this);
	text->SetParent(this);

	time1 = 0;
	time2 = 0;
}

/*
 * Destructor for the GuiTooltip class.
 */
GuiTooltip::~GuiTooltip()
{
	if(text)
		delete text;
}

float GuiTooltip::GetScale()
{
	return (xscale * scaleDyn);
}

/* !Sets the text of the GuiTooltip element
 * !\param t Text
 */
void GuiTooltip::SetText(const char * t)
{
	if(text)
	{
		delete text;
		text = NULL;
	}

	if(!t)
		return;

	text = new GuiText(t, 20, (GXColor){0, 0, 0, 255});
	text->SetMaxWidth(250);
	text->SetParent(this);
	text->SetPosition(offsetHr, offsetVr);
    text->SetScroll(SCROLL_HORIZONTAL);

	int bound = text->GetTextWidth() < text->GetMaxWidth() ? text->GetTextWidth() : text->GetMaxWidth();
	int tile = ((float)bound / (float)tileImage.GetWidth()) + 1;

	if(tile < 0)
		tile = 0;

	leftImage.SetPosition(offsetHr, offsetVr);
	tileImage.SetPosition(leftImage.GetWidth() + offsetHr, offsetVr);
	tileImage.SetTile(tile);
	rightImage.SetPosition(leftImage.GetWidth() + tile * tileImage.GetWidth() + offsetHr, offsetVr);
	width = leftImage.GetWidth() + tile * tileImage.GetWidth() + rightImage.GetWidth();
}

void GuiTooltip::SetOffset(int hOff, int vOff)
{
    offsetHr = hOff;
    offsetVr = vOff;
    text->SetPosition(offsetHr, offsetVr);

	int tile = ((float)text->GetTextWidth() / (float)tileImage.GetWidth()) + 1;
	if(tile < 0)
		tile = 0;

    leftImage.SetPosition(offsetHr, offsetVr);
	tileImage.SetPosition(leftImage.GetWidth() + offsetHr, offsetVr);
	rightImage.SetPosition(leftImage.GetWidth() + tile * tileImage.GetWidth() + offsetHr, offsetVr);
}

void GuiTooltip::ResetText()
{
	if(text)
		text->ResetText();

	int tile = ((float)text->GetTextWidth() / (float)tileImage.GetWidth()) + 1;

	if(tile < 0)
		tile = 0;

	tileImage.SetTile(tile);
	rightImage.SetPosition(leftImage.GetWidth() + tile * tileImage.GetWidth(), 0);
	width = leftImage.GetWidth() + tile * tileImage.GetWidth() + rightImage.GetWidth();
}

/*
 * Draws the tooltip
 */
void GuiTooltip::DrawTooltip()
{
	if(!text || !this->IsVisible() || !parentElement)
		return;

	if(parentElement->GetState() == STATE_SELECTED)
	{
		if (time2 == 0)
		{
			time(&time1);
			time2 = time1;
		}
		if(time1 != 0)
			time(&time1);

		if(time1 == 0 || difftime(time1, time2) >= 1.0)
		{
			if(time1 != 0)
				SetEffect(EFFECT_FADE, 20);
			time1 = 0;
			goto draw;
		}
	}
	else
	{
		if(time2 != 0 && time1 == 0)
			SetEffect(EFFECT_FADE, -20);
		time2 = 0;
	}

	if(GetEffect())
	{
		draw:
		leftImage.Draw();
		tileImage.Draw();
		rightImage.Draw();
		text->Draw();
	}

	this->UpdateEffects();
}
