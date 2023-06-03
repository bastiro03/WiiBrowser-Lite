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
#ifndef _LONGTEXT_HPP_
#define _LONGTEXT_HPP_

#include "libwiigui/gui.h"

#define LIMIT(x, min, max)																	\
	({																						\
		typeof( x ) _x = x;																	\
		typeof( min ) _min = min;															\
		typeof( max ) _max = max;															\
		( ( ( _x ) < ( _min ) ) ? ( _min ) : ( ( _x ) > ( _max ) ) ? (( _min ) > ( _max ) ? ( _min ) : ( _max )) : ( _x ) );	\
	})

class GuiLongText : public GuiText
{
public:
	//!Constructor
	//!\param t Text
	//!\param s Font size
	//!\param c Font color
	GuiLongText(const char* t, int s, GXColor c);
	GuiLongText(const wchar_t* t, int s, GXColor c);
	virtual ~GuiLongText();
	//!Sets the text of the GuiText element
	//!\param t Text
	void SetText(const char* t);
	void SetText(const wchar_t* t);
	//!Set the max text width
	void SetMaxWidth(int width);
	//!Set the start width of the text
	void SetStartWidth(int width);
	//!Go to next line
	void NextLine();
	//!Go to previous line
	void PreviousLine();
	//!Set the text line
	void SetTextLine(int line);
	//!Set to the char pos in text
	void SetTextPos(int pos);
	//!Refresh the rows to draw
	int GetCurrPos() { return curLineStart; };
	//!Get  the count of loaded lines
	int GetLinesCount() { return LIMIT(linestodraw, 0, (int)(TextLines.size() - curLineStart)); };
	//!Get the total count of lines
	int GetTotalLinesCount() { return TextLines.size(); };
	//!Get the maximum width of a line in the text
	int GetStartWidth() { return startWidth; };
	//!Get the maximum width of a line in the text
	int GetMaxLineWidth() { return maxLineWidth; };
	//!Get a Textline
	const wchar_t* GetTextLine(int ind);
	//!Get the offset in the text of a drawn Line
	int GetLineOffset(int ind);
	// calculate offsets to linebreaks
	void CalcLineOffsets();
	//!Constantly called to draw the text
	void Draw();

	bool AddChar(int iPos, wchar_t charCode);
	void RemoveText(int iPos, int iNumber);
	void CheckMaxLineWidth(int iPos);

protected:
	std::vector<u32> TextLines;
	int curLineStart;
	int FirstLineOffset;
	int startWidth;
	int maxLineWidth;
};

#endif
