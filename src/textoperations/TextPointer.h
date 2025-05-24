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
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#ifndef TEXTPOINTER_H
#define TEXTPOINTER_H

#include "libwiigui/gui.h"

class TextPointer : public GuiButton
{
public:
	TextPointer(GuiText *parent, int linestodraw);
	TextPointer(GuiText *parent, int w, int h);
	virtual ~TextPointer();
	void PositionChanged(int chan, int x, int y);
	void SetLetterPosition(int LetterPos);
	void SetCurrentLine(int line);
	void UpdateWidth();
	int GetCurrentLine() { return currentline; };
	int GetCurrentLetter() { return LetterNumInLine; };
	int GetPointerPosX() { return Position_X; };
	int GetPointerPosY() { return Position_Y; };
	void Refresh(void) { PositionChanged(currentChan, Position_X, Position_Y); };
	bool IsPointerVisible() { return (Position_X >= 0 && Position_X < TextPtr->GetMaxWidth()); }
	void Draw();

protected:
	int fontsize;
	int lineLength;
	int currentline;
	int currentChan;
	int Position_X;
	int LetterNumInLine;
	int Position_Y;
	bool Marking;
	bool visibility;
	GuiImageData *TextPointerImgData;
	GuiImage *TextPointerImg;
	GuiImage *MarkImage;
	GuiText *TextPtr;
};

#endif
