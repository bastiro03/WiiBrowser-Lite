 /****************************************************************************
 * Copyright (C) 2009
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * TextPointer.cpp
 * for WiiXplorer 2009
 ***************************************************************************/
#include "libwiigui/gui.h"
#include "TextPointer.h"
#include "FreeTypeGX.h"

extern FreeTypeGX * fontSystem[];

/**
 * Constructor for the TextPointer class.
 */
TextPointer::TextPointer(GuiText *parent, int linestodraw)
	: GuiButton(343, 240)
{
	TextPtr = parent;
	fontsize = TextPtr->GetFontSize();
	MarkImage = NULL;
	LetterNumInLine = 0;
	currentline = 0;
	Position_X = 0;
	Position_Y = 0;
	height = (linestodraw+1)*(fontsize+5);
	visibility = true;

	TextPointerImgData = new GuiImageData(textpointer_img_png);
	TextPointerImg = new GuiImage(TextPointerImgData);
	TextPointerImg->SetVisible(visibility);

	SetLabel(TextPtr);
	SetImage(TextPointerImg);
	UpdateWidth();
}

TextPointer::TextPointer(GuiText *parent, int w, int h)
	: GuiButton(w, h)
{
	TextPtr = parent;
	fontsize = TextPtr->GetFontSize();
	MarkImage = NULL;
	LetterNumInLine = 0;
	currentline = 0;
	Position_X = 0;
	Position_Y = 0;
	width = w;
	height = h;
	visibility = true;

	TextPointerImgData = new GuiImageData(textpointer_img_png);
	TextPointerImg = new GuiImage(TextPointerImgData);
	TextPointerImg->SetVisible(visibility);

	SetLabel(TextPtr);
	SetImage(TextPointerImg);
}

/**
 * Destructor for the TextPointer class.
 */
TextPointer::~TextPointer()
{
	delete (TextPointerImgData);
	delete TextPointerImg;

	if(MarkImage)
		delete MarkImage;
}

void TextPointer::PositionChanged(int chan, int x, int y)
{
	if(MarkImage)
	{
		delete MarkImage;
		MarkImage = NULL;
	}

	currentChan = chan;
	currentline = -1;
	Position_X = 0;
	Position_Y = 0;
	LetterNumInLine = 0;
	Marking = false;

	int linenumber = 0;

	int differenz = 1000;

	int maxlines = TextPtr->GetLinesCount();
	if(maxlines < 1)
		maxlines = 1;

	for(int i = 0; i < maxlines; i++)
	{
		int linestart = i*(fontsize+5);
		int diff = abs(linestart-y + 10);
		if(differenz > diff)
		{
			differenz = diff;
			linenumber = i;
		}
	}
	differenz = 1000;
	const wchar_t * line = TextPtr->GetTextLine(linenumber);

	if(!line)
		return;

	int i = 0;
	int w = -TextPtr->GetStartWidth();

	while(line[i] != 0 && line[i] != '\n')
	{
		w += fontSystem[fontsize]->getCharWidth(line[i]);

		if(w > TextPtr->GetMaxWidth())
			break;

		int diff = abs(x-w);
		if(differenz > diff)
		{
			differenz = diff;
			Position_X = w;
			LetterNumInLine = i+1;
		}
		i++;
	}

	if(differenz > abs(x))
	{
		differenz = abs(x);
		Position_X = -TextPtr->GetStartWidth();
		LetterNumInLine = 0;
	}

	currentChan = chan;
	currentline = linenumber;
	Position_Y = linenumber*(fontsize+5);
	Marking = true;
}

void TextPointer::SetLetterPosition(int LetterPos)
{
	if(LetterPos <= 0)
	{
		Position_X = -TextPtr->GetStartWidth();
		LetterNumInLine = 0;
		Marking = false;
		return;
	}

	const wchar_t * line = TextPtr->GetTextLine(currentline);
	if(!line)
		return;

	lineLength = 0;

	while(line[lineLength] != 0 && line[lineLength] != '\n')
		lineLength++;

	if(LetterPos < 0)
		LetterPos = 0;
	else if(LetterPos > lineLength)
		LetterPos = lineLength;

	Position_X = -TextPtr->GetStartWidth();

	for(int i = 0; i < LetterPos; ++i)
		Position_X += fontSystem[fontsize]->getCharWidth(line[i]);

	LetterNumInLine = LetterPos;
	Marking = false;
}

void TextPointer::SetCurrentLine(int line)
{
	currentline = line;
	Position_Y = currentline*(fontsize+5);
}

void TextPointer::UpdateWidth()
{
	width = TextPtr->GetMaxWidth();
	if(width == 0xFFFF)
		width = TextPtr->GetTextWidth();
}

void TextPointer::Draw()
{
	if(FrameTimer % 30 == 0)
	{
		visibility = !visibility && IsPointerVisible();
		TextPointerImg->SetVisible(visibility);
	}
	else if(TextPointerImg->GetLeft() != Position_X || TextPointerImg->GetTop() != Position_Y)
		TextPointerImg->SetPosition(Position_X, Position_Y);

	GuiButton::Draw();
}
