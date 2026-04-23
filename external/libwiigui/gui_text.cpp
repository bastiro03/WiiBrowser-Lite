/****************************************************************************
 * libwiigui
 * WiiBrowser
 *
 * Tantric 2009
 * modified by gave92
 *
 * gui_text.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../gettext.h"

static GXColor presetColor = (GXColor){255, 255, 255, 255};
static int presetSize = 0;
static int presetMaxWidth = 0;
static int presetAlignmentHor = 0;
static int presetAlignmentVert = 0;
static u16 presetStyle = 0;

#define TEXT_SCROLL_DELAY			10
#define	TEXT_SCROLL_INITIAL_DELAY	8

/**
 * Constructor for the GuiText class.
 */
GuiText::GuiText(const char * t, int s, GXColor c)
{
	origText = NULL;
	text = NULL;
	font = NULL;
	size = s;
	currentSize = size;
	color = c;
	alpha = c.a;
	linestodraw = MAX_LINES;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	usespace = true;
	textModel = NORMAL;
	wrap = false;
	textDynNum = 0;
	textScroll = SCROLL_NONE;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;

    if(t)
	{
		origText = strdup(t);
		text = charToWideChar(gettext(t));

        if(currentSize > MAX_FONT_SIZE)
            currentSize = MAX_FONT_SIZE;

        if(!fontSystem[currentSize])
        {
            fontSystem[currentSize] = new FreeTypeGX(currentSize);
        }
		textWidth = fontSystem[currentSize]->getWidth(text);
	}

	for(int i=0; i < MAX_LINES; i++)
		textDyn[i] = NULL;
    selectable = true;
}

GuiText::GuiText(const wchar_t * t, int s, GXColor c)
{
    origText = NULL;
	text = NULL;
	font = NULL;
	size = s;
	currentSize = size;
	color = c;
	alpha = c.a;
    linestodraw = MAX_LINES;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	font = 0;
	usespace = true;
	wrap = false;
	textModel = NORMAL;
	textDynNum = 0;
	textScroll = SCROLL_NONE;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;

    if(t)
	{
		text = new (std::nothrow) wchar_t[wcslen(t)+1];
		if(!text)
            return;

		wcscpy(text, t);

        if(currentSize > MAX_FONT_SIZE)
            currentSize = MAX_FONT_SIZE;

        if(!fontSystem[currentSize])
        {
            fontSystem[currentSize] = new FreeTypeGX(currentSize);
        }
		textWidth = fontSystem[currentSize]->getWidth(text);
	}

    for(int i=0; i < MAX_LINES; i++)
		textDyn[i] = NULL;
	selectable = true;
}

/**
 * Constructor for the GuiText class, uses presets
 */
GuiText::GuiText(const char * t)
{
	origText = NULL;
	text = NULL;
	font = NULL;
	size = presetSize;
	currentSize = size;
	color = presetColor;
	alpha = presetColor.a;
	style = presetStyle;
	maxWidth = presetMaxWidth;
	wrap = false;
    usespace = true;
	textModel = NORMAL;
	textDynNum = 0;
	font = 0;
    linestodraw = MAX_LINES;
	textScroll = SCROLL_NONE;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignmentHor = presetAlignmentHor;
	alignmentVert = presetAlignmentVert;

    if(t)
	{
	    origText = strdup(t);
		text = charToWideChar(gettext(t));
		if(!text)
		    return;

        if(currentSize > MAX_FONT_SIZE)
            currentSize = MAX_FONT_SIZE;

        if(!fontSystem[currentSize])
        {
            fontSystem[currentSize] = new FreeTypeGX(currentSize);
        }
		textWidth = fontSystem[currentSize]->getWidth(text);
	}

	for(int i=0; i < MAX_LINES; i++)
		textDyn[i] = NULL;
    selectable = true;
}

/**
 * Destructor for the GuiText class.
 */
GuiText::~GuiText()
{
	if(origText)
		free(origText);
	if(text)
		delete[] text;

    if(font)
	{
        delete font;
        font = NULL;
    }

    ClearDynamicText();
}

void GuiText::ClearDynamicText()
{
    for(int i=0; i < textDynNum; i++)
    {
        if(textDyn[i])
            delete [] textDyn[i];
    }

    textDynNum = 0;
}

void GuiText::SetText(const char * t)
{
	if(origText)
		free(origText);
	if(text)
		delete[] text;

	ClearDynamicText();

	origText = NULL;
	text = NULL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

    if(t)
	{
		origText = strdup(t);
		text = charToWideChar(gettext(t));
		if(!text)
		    return;

		if(!fontSystem[currentSize])
            fontSystem[currentSize] = new FreeTypeGX(currentSize);

		textWidth = fontSystem[currentSize]->getWidth(text);
	}
}

void GuiText::SetWText(wchar_t * t)
{
	if(origText)
		free(origText);
	if(text)
		delete[] text;

	ClearDynamicText();

	origText = NULL;
	text = NULL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

    if(t)
	{
		text = wcsdup(t);

		if(!fontSystem[currentSize])
            fontSystem[currentSize] = new FreeTypeGX(currentSize);

		textWidth = fontSystem[currentSize]->getWidth(text);
	}
}

int GuiText::GetLength()
{
	if(!text)
		return 0;

	return wcslen(text);
}

void GuiText::SetPresets(int sz, GXColor c, int w, u16 s, int h, int v)
{
	presetSize = sz;
	presetColor = c;
	presetStyle = s;
	presetMaxWidth = w;
	presetAlignmentHor = h;
	presetAlignmentVert = v;
}

void GuiText::SetFontSize(int s)
{
	size = s;
}

void GuiText::SetSpace(bool space)
{
    usespace=space;
}

void GuiText::SetMaxWidth(int width)
{
	maxWidth = width;

	ClearDynamicText();
}

int GuiText::GetMaxWidth()
{
    return maxWidth;
}

char * GuiText::GetText()
{
    return origText;
}

int GuiText::GetTextWidth()
{
	if(!text)
		return 0;

	return (font ? font : fontSystem[currentSize])->getWidth(text);
}

void GuiText::SetWrap(bool w, int width)
{
	wrap = w;
	maxWidth = width;

	ClearDynamicText();
}

void GuiText::SetScroll(int s)
{
	if(textScroll == s)
		return;

	ClearDynamicText();

	textScroll = s;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;
}

void GuiText::SetColor(GXColor c)
{
	color = c;
	alpha = c.a;
}

void GuiText::SetStyle(u16 s)
{
	style |= s;
}

void GuiText::SetAlignment(int hor, int vert)
{
	style = 0;

	switch(hor)
	{
		case ALIGN_LEFT:
			style |= FTGX_JUSTIFY_LEFT;
			break;
		case ALIGN_RIGHT:
			style |= FTGX_JUSTIFY_RIGHT;
			break;
		default:
			style |= FTGX_JUSTIFY_CENTER;
			break;
	}
	switch(vert)
	{
		case ALIGN_TOP:
			style |= FTGX_ALIGN_TOP;
			break;
		case ALIGN_BOTTOM:
			style |= FTGX_ALIGN_BOTTOM;
			break;
		default:
			style |= FTGX_ALIGN_MIDDLE;
			break;
	}

	alignmentHor = hor;
	alignmentVert = vert;
}

/**
 * Change font
 */
bool GuiText::SetFont(const u8 *fontbuffer, const u32 filesize)
{
    if(!fontbuffer || !filesize)
        return false;

    if(font)
    {
        delete font;
        font = NULL;
    }
	font = new FreeTypeGX(currentSize, fontbuffer, filesize);
	textWidth = font->getWidth(text);

	return true;
}

void GuiText::ResetText()
{
	if(!origText)
		return;
	if(text)
		delete[] text;

	text = charToWideChar(gettext(origText));

	ClearDynamicText();
	// currentSize = 0;
}

int GuiText::GetLinesCount()
{
	if(!text)
		return 0;

	return textDynNum>0 ? textDynNum : 1;
}

void GuiText::SetModel(int model)
{
    textModel = model;
}

int GuiText::GetTextMaxWidth()
{
    return maxWidth;
}

/**
 * Update the text on screen
 */
bool GuiText::IsOver(int x, int y)
{
	if(unsigned(x - this->GetLeft())  < unsigned(textWidth)
	&& unsigned(y - this->GetTop())  < unsigned(size /**GetLinesCount()*/))
		return true;
	return false;
}

void GuiText::Select(GuiTrigger * t)
{
    if (textModel != ANCHOR)
        return;

    if(t->wpad->ir.valid && t->chan >= 0)
	{
        if(this->IsOver(t->wpad->ir.x, t->wpad->ir.y))
        {
            this->SetStyle(FTGX_STYLE_UNDERLINE);
        }
        else
        {
            style &= (~FTGX_STYLE_MASK);
        }
	}
}

const wchar_t * GuiText::GetDynText(int ind)
{
	if(ind < 0 || ind >= (int) textDynNum)
		return text;

	return textDyn[ind];
}

/**
 * Draw the text on screen
 */
void GuiText::Draw()
{
	if(!text)
		return;

	if(!this->IsVisible())
		return;

	GXColor c = color;
	c.a = this->GetAlpha();

	int newSize = size*this->GetScale();

	if(newSize > MAX_FONT_SIZE)
		newSize = MAX_FONT_SIZE;

    if(!fontSystem[newSize])
    {
        fontSystem[newSize] = new FreeTypeGX(newSize);
        if(text)
            textWidth = (font ? font : fontSystem[newSize])->getWidth(text);
    }

	if(newSize != currentSize)
	{
	    if(font)
	    {
	        font->ChangeFontSize(newSize);
	    }
        if(text)
            textWidth = (font ? font : fontSystem[newSize])->getWidth(text);
		currentSize = newSize;
	}

	if(maxWidth == 0)
	{
        (font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop(), text, c, style, textWidth);
		this->UpdateEffects();
		return;
	}

	u32 textlen = wcslen(text);

	if(wrap)
	{
		if(textDynNum == 0)
		{
			u32 n = 0, ch = 0;
			int linenum = 0;
			int lastSpace = -1;
			int lastSpaceIndex = -1;
			int currentWidth = 0;

			while(ch < textlen && linenum < linestodraw)
			{
				if(n == 0)
					textDyn[linenum] = new wchar_t[textlen + 1];

				textDyn[linenum][n] = text[ch];
				textDyn[linenum][n+1] = 0;

				// currentWidth += (font ? font : fontSystem[currentSize])->getCharWidth(text[ch]);
				currentWidth = (font ? font : fontSystem[currentSize])->getWidth(textDyn[linenum]);

                if(currentWidth > ((linenum == 0 || (style & FTGX_JUSTIFY_MASK) != FTGX_JUSTIFY_LEFT) ? maxWidth : screenwidth-80))
                {
                    if(lastSpace >= 0)
                    {
                        textDyn[linenum][lastSpaceIndex] = 0; // discard space, and everything after
                        ch = lastSpace; // go backwards to the last space
                        lastSpace = -1; // we have used this space
                        lastSpaceIndex = -1;
                    }
                    ++linenum;
                    n = -1;
                }
                else if(ch == textlen-1)
                {
                    ++linenum;
                }
				else if(usespace && text[ch] == ' ' && n >= 0)
				{
					lastSpace = ch;
					lastSpaceIndex = n;
				}
				++ch;
				++n;
			}
			textDynNum = linenum;
		}

		int lineheight = newSize + 5;
		int leftPoint, voffset = 0;

		if(alignmentVert == ALIGN_MIDDLE)
			voffset = (lineheight >> 1) * (1-textDynNum);
        leftPoint = ((style & FTGX_JUSTIFY_MASK) != FTGX_JUSTIFY_LEFT) ? this->GetLeft() : this->GetOffset();

		int left = this->GetLeft();
		int top  = this->GetTop() + voffset;

		for(int i=0; i < textDynNum; ++i) {
		    if (i == 0)
                (font ? font : fontSystem[currentSize])->drawText(left, top+i*lineheight, textDyn[i], c, style);
			else (font ? font : fontSystem[currentSize])->drawText(leftPoint, top+i*lineheight, textDyn[i], c, style);
		}
	}
	else
	{
		if(textDynNum == 0)
		{
            textDynNum = 1;
            textDyn[0] = wcsdup(text);
            int len = wcslen(textDyn[0]);

            while((font ? font : fontSystem[currentSize])->getWidth(textDyn[0]) > maxWidth)
                textDyn[0][--len] = 0;
		}

		if(textScroll == SCROLL_DOTTED)
		{
            int len = wcslen(textDyn[0]);

            if(len > 3)
                swprintf(&textDyn[0][len-3], 4, L"...");
		}

		if(textScroll == SCROLL_HORIZONTAL)
		{
			if((font ? font : fontSystem[currentSize])->getWidth(text) > maxWidth && (FrameTimer % textScrollDelay == 0))
			{
				if(textScrollInitialDelay)
				{
					--textScrollInitialDelay;
				}
				else
				{
					++textScrollPos;
					if((u32)textScrollPos > textlen-1)
					{
						textScrollPos = 0;
						textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
					}

                    wcscpy(textDyn[0], &text[textScrollPos]);
                    u32 dynlen = wcslen(textDyn[0]);

                    if(dynlen+2 < textlen)
                    {
                        textDyn[0][dynlen] = ' ';
                        textDyn[0][dynlen+1] = ' ';
                        textDyn[0][dynlen+2] = 0;
                        dynlen += 2;
                    }

                    if((font ? font : fontSystem[currentSize])->getWidth(textDyn[0]) > maxWidth)
                    {
                        while((font ? font : fontSystem[currentSize])->getWidth(textDyn[0]) > maxWidth)
                            textDyn[0][--dynlen] = 0;
                    }
                    else
                    {
                        int i = 0;
                        while((font ? font : fontSystem[currentSize])->getWidth(textDyn[0]) < maxWidth && dynlen+1 < textlen)
                        {
                            textDyn[0][dynlen] = text[i++];
                            textDyn[0][++dynlen] = 0;
                        }

                        if((font ? font : fontSystem[currentSize])->getWidth(textDyn[0]) > maxWidth)
                            textDyn[0][dynlen-2] = 0;
                        else textDyn[0][dynlen-1] = 0;
                    }
                }
			}
		}
		(font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop(), textDyn[0], c, style);
	}
	this->UpdateEffects();
}
