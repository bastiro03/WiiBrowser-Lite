/****************************************************************************
 * Copyright (C) 2009-2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "gui_longtext.h"
#include "FreeTypeGX.h"

extern FreeTypeGX * fontSystem[];

GuiLongText::GuiLongText(const char * t, int s, GXColor c)
	:	GuiText(t, s, c)
{
	maxWidth = 400;
	linestodraw = 9;
	curLineStart = 0;
	FirstLineOffset = 0;
	startWidth = 0;
	maxLineWidth = 0;

	SetText(t);
}

GuiLongText::GuiLongText(const wchar_t * t, int s, GXColor c)
	:	GuiText((wchar_t *) NULL, s, c)
{
	maxWidth = 400;
	linestodraw = 9;
	curLineStart = 0;
	FirstLineOffset = 0;
	startWidth = 0;
	maxLineWidth = 0;

	SetText(t);
}

GuiLongText::~GuiLongText()
{
}

void GuiLongText::SetText(const char * t)
{
	GuiText::SetText(t);

	if(text) {
		CalcLineOffsets();
	}
}

void GuiLongText::SetText(const wchar_t * t)
{
	GuiText::SetWText((wchar_t *)t);

	if(text) {
		CalcLineOffsets();
	}
}

void GuiLongText::SetMaxWidth(int w)
{
	maxWidth = w;
}

void GuiLongText::SetStartWidth(int w)
{
	startWidth = w;
}

void GuiLongText::SetTextLine(int line)
{
	curLineStart = LIMIT(line, 0, (int) TextLines.size()-1);
}

void GuiLongText::SetTextPos(int pos)
{
	if(!text)
		return;

	int diff = 10000;
	int targetLine = 0;

	for(u32 i = 0; i < TextLines.size(); i++)
	{
		int curDiff = abs(TextLines[i] - pos);
		if(curDiff < diff)
		{
			diff = curDiff;
			targetLine = i;
		}
	}

	SetTextLine(targetLine);
}

bool GuiLongText::AddChar(int iPos, wchar_t charCode)
{
	if(!text)
		return false;

	wchar_t *newText = new (std::nothrow) wchar_t[wcslen(text)+2];
	if(!newText)
		return false;

	int i = 0, n = 0;

	TextLines.clear();
	TextLines.push_back(0);

	while(text[i])
	{
		if(iPos == n)
		{
			if(charCode == '\n')
				TextLines.push_back(n+1);

			newText[n] = charCode;
			n++;
			continue;
		}

		if(text[i] == '\n')
			TextLines.push_back(n+1);

		newText[n] = text[i];
		++i;
		++n;
	}

	// append a character
	if(iPos == n)
	{
		if(charCode == '\n')
			TextLines.push_back(n+1);

		newText[n] = charCode;
		n++;
	}

	newText[n] = 0;

	wchar_t *tmp = text;
	text = newText;
	delete [] tmp;

	return true;
}

void GuiLongText::RemoveText(int iPos, int iNumber)
{
	if(!text || iPos < 0)
		return;

	TextLines.clear();
	TextLines.push_back(0);

	int i;
	for(i = 0; i < iPos && text[i]; ++i)
	{
		if(text[i] == L'\n')
			TextLines.push_back(i+1);
	}

	while(text[iPos])
	{
		if(iPos - i < iNumber)
		{
			iPos++;
			continue;
		}

		text[i] = text[iPos];

		if(text[i] == L'\n')
			TextLines.push_back(i+1);

		++i;
		++iPos;
	}

	text[i] = 0;
}

void GuiLongText::CheckMaxLineWidth(int iPos)
{
	if(!text || iPos < 0)
		return;

	int ch = iPos;
	int lineWidth = 0;

	while(text[ch] && text[ch] != '\n')
	{
		lineWidth +=  fontSystem[currentSize]->getCharWidth(text[ch]);
		ch++;
	}

	if(maxLineWidth < lineWidth) {
		maxLineWidth = lineWidth;
	}
}

int GuiLongText::GetLineOffset(int ind)
{
	if(TextLines.size() == 0)
		return 0;

	if(ind < 0)
		return TextLines[0];

	if(ind >= (int) TextLines.size()-1)
		return TextLines[TextLines.size()-1];

	return TextLines[ind];
}

const wchar_t * GuiLongText::GetTextLine(int ind)
{
	if(!text || ((u32)(curLineStart + ind) >= TextLines.size()))
		return NULL;

	return &text[TextLines[curLineStart + ind]];
}

void GuiLongText::NextLine()
{
	if(!text || (curLineStart+1 > ((int) TextLines.size()-linestodraw)))
		return;

	++curLineStart;
}

void GuiLongText::PreviousLine()
{
	if(!text || curLineStart-1 < 0)
		return;

	--curLineStart;
}

void GuiLongText::CalcLineOffsets()
{
	TextLines.clear();
	TextLines.push_back(0);

	int ch = 0;
	int lineWidth = 0;
	maxLineWidth = 0;

	while(text[ch])
	{
		lineWidth +=  fontSystem[currentSize]->getCharWidth(text[ch]);

		if(text[ch] == '\n')
		{
			TextLines.push_back(ch+1);

			if(maxLineWidth < lineWidth) {
				maxLineWidth = lineWidth;
			}
			lineWidth = 0;
		}

		ch++;
	}

	if(maxLineWidth < lineWidth) {
		maxLineWidth = lineWidth;
	}
}

void GuiLongText::Draw()
{
	if((u32)curLineStart >= TextLines.size())
		return;

	if(!this->IsVisible())
		return;

	GXColor c = color;
	c.a = this->GetAlpha();

	currentSize = size*GetScale();

	(font ? font : fontSystem[currentSize])->drawLongText(this->GetLeft(), this->GetTop(),
			&text[TextLines[curLineStart]], c, alignmentHor | alignmentVert, currentSize + 5,
			linestodraw, startWidth, maxWidth);
}
