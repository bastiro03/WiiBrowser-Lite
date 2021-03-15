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
 * TextEditor.cpp
 * for WiiXplorer 2009
 ***************************************************************************/

#include <unistd.h>
#include "TextOperations/TextEditor.h"
#include "common.h"

#define FONTSIZE    18

void *LoadFile(char *filepath, int size);
off_t GetFileSize(const char *filename);

/**
 * Constructor for the TextEditor class.
 */
TextEditor::TextEditor(const wchar_t *intext, int LinesToDraw, const char *path)
{
	triggerupdate = true;
	ExitEditor = false;
	LineEditing = false;
	FileEdited = false;
	linestodraw = LinesToDraw;
	filesize = (u32) GetFileSize(path);

	filepath = new char[strlen(path)+1];
	snprintf(filepath, strlen(path)+1, "%s", path);

	char * filename = strrchr(filepath, '/')+1;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, PAD_BUTTON_X);

	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	bgTexteditorData = new GuiImageData(textreader_box_png, textreader_box_png_size);
	bgTexteditorImg = new GuiImage(bgTexteditorData);

	closeImgData = new GuiImageData(close_png, close_png_size);
	closeImgOverData = new GuiImageData(close_over_png, close_over_png_size);
    closeImg = new GuiImage(closeImgData);
    closeImgOver = new GuiImage(closeImgOverData);
	maximizeImgData = new GuiImageData(maximize_dis_png, maximize_dis_png_size);
    maximizeImg = new GuiImage(maximizeImgData);
	minimizeImgData = new GuiImageData(minimize_dis_png, minimize_dis_png_size);
    minimizeImg = new GuiImage(minimizeImgData);

    closeBtn = new GuiButton(closeImg->GetWidth(), closeImg->GetHeight());
    closeBtn->SetImage(closeImg);
    closeBtn->SetImageOver(closeImgOver);
    closeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    closeBtn->SetPosition(-30, 30);
    closeBtn->SetSoundOver(btnSoundOver);
    closeBtn->SetSoundClick(btnSoundClick);
    closeBtn->SetTrigger(trigA);
    closeBtn->SetTrigger(trigB);
    closeBtn->SetEffectGrow();
    closeBtn->Clicked.connect(this, &TextEditor::OnButtonClick);

    maximizeBtn = new GuiButton(maximizeImg->GetWidth(), maximizeImg->GetHeight());
    maximizeBtn->SetImage(maximizeImg);
    maximizeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    maximizeBtn->SetPosition(-60, 30);
    maximizeBtn->SetSoundClick(btnSoundClick);

    minimizeBtn = new GuiButton(minimizeImg->GetWidth(), minimizeImg->GetHeight());
    minimizeBtn->SetImage(minimizeImg);
    minimizeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    minimizeBtn->SetPosition(-90, 30);
    minimizeBtn->SetSoundClick(btnSoundClick);

    filenameTxt = new GuiText(filename, 22, (GXColor){0, 0, 0, 255});
    filenameTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    filenameTxt->SetPosition(-30,30);
	filenameTxt->SetWrap(true, 340);
    filenameTxt->SetScroll(SCROLL_DOTTED);

    MainFileTxt = new GuiLongText(intext, FONTSIZE, (GXColor){0, 0, 0, 255});
    MainFileTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    MainFileTxt->SetPosition(0, 15);
    MainFileTxt->SetLinesToDraw(linestodraw);
    MainFileTxt->SetMaxWidth(330);

    TextPointerBtn = new TextPointer(MainFileTxt, linestodraw);
    TextPointerBtn->SetPosition(43, 75);
    TextPointerBtn->SetHoldable(true);
    TextPointerBtn->SetTrigger(trigHeldA);
    TextPointerBtn->PositionChanged(0, 0, 0);
    TextPointerBtn->Held.connect(this, &TextEditor::OnPointerHeld);

    PlusBtn = new GuiButton(0, 0);
    PlusBtn->SetTrigger(trigPlus);
    PlusBtn->SetSoundClick(btnSoundClick);
    PlusBtn->Clicked.connect(this, &TextEditor::OnButtonClick);

	width = bgTexteditorImg->GetWidth();
	height = bgTexteditorImg->GetHeight();

    this->Append(PlusBtn);
    this->Append(bgTexteditorImg);
    this->Append(filenameTxt);
    this->Append(TextPointerBtn);
    this->Append(closeBtn);
    this->Append(maximizeBtn);
    this->Append(minimizeBtn);

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    SetPosition(0,0);
    SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
}

/**
 * Destructor for the TextEditor class.
 */
TextEditor::~TextEditor()
{
    this->RemoveAll();

    /** Buttons **/
	delete maximizeBtn;
	delete minimizeBtn;
	delete closeBtn;
	delete TextPointerBtn;

    /** Images **/
	delete bgTexteditorImg;
	delete closeImg;
	delete closeImgOver;
	delete maximizeImg;
	delete minimizeImg;

    /** ImageDatas **/
	delete(bgTexteditorData);
	delete(closeImgData);
	delete(closeImgOverData);
	delete(maximizeImgData);
	delete(minimizeImgData);

    /** Sounds **/
	delete(btnSoundOver);
	delete(btnSoundClick);

    /** Triggers **/
	delete trigHeldA;
	delete trigA;
	delete trigB;
	delete PlusBtn;

    /** Texts **/
    delete filenameTxt;
    delete MainFileTxt;
    delete [] filepath;
}

void TextEditor::SetText(const wchar_t *intext)
{
    if(TextPointerBtn)
    {
        delete TextPointerBtn;
        TextPointerBtn = NULL;
    }

    MainFileTxt->SetText(intext);

    TextPointerBtn = new TextPointer(MainFileTxt, 0);
    TextPointerBtn->SetPosition(43, 75);
    TextPointerBtn->SetHoldable(true);
    TextPointerBtn->SetTrigger(trigHeldA);
    TextPointerBtn->Held.connect(this, &TextEditor::OnPointerHeld);
}

void TextEditor::SetTriggerUpdate(bool set)
{
	triggerupdate = set;
}

void TextEditor::WriteTextFile(const char * path)
{
    FILE * f = fopen(path, "wb");
    if(!f)
        return;

    const wchar_t * FullWText = MainFileTxt->GetDynText(-1);
    char *FullText = wideCharToChar(FullWText, 0);

    fwrite(FullText, 1, strlen(FullText)+1, f);
    fclose(f);

    delete FullText;
}

void TextEditor::ResetState()
{
	state = STATE_DEFAULT;
	stateChan = -1;

	maximizeBtn->ResetState();
	minimizeBtn->ResetState();
	closeBtn->ResetState();
}

int TextEditor::GetState()
{
    if(LineEditing)
    {
        SetTriggerUpdate(false);
        if(EditLine() > 0)
        {
            FileEdited = true;
        }
        SetTriggerUpdate(true);

        this->SetState(STATE_DEFAULT);
        LineEditing = false;
    }

    if(state == STATE_CLOSED && FileEdited)
    {
        int choice = WindowPrompt("File was edited", "Do you want to save changes?", "Yes", "No");
        if(choice == 1)
        {
            WriteTextFile(filepath);
            state = STATE_CLOSED;
        }
        else if(choice == 0)
        {
            //to revert the state reset
            state = STATE_CLOSED;
        }
    }

    return GuiWindow::GetState();
}

wstring * toWString(const wchar_t * s)
{
    if(!s)
        return NULL;

    wstring * ws = new wstring();
	std::string::size_type size;
	size = wcslen(s);

	ws->resize(size);
	for (std::string::size_type i = 0; i < size; ++i)
		ws->at(i) = (unsigned char)s[i];

    return ws;
}

int TextEditor::EditLine()
{
    int currentline = TextPointerBtn->GetCurrentLine();

    if(currentline < 0 || currentline >= linestodraw)
        return -1;

    u32 LetterNumInLine = TextPointerBtn->GetCurrentLetter();

    wstring * wText = toWString(MainFileTxt->GetDynText(-1));
    if(!wText)
        return -1;

    const wchar_t * lineText = MainFileTxt->GetTextLine(currentline);
    if(!lineText)
        return -1;

    wchar_t temptxt[150];
    memset(temptxt, 0, sizeof(temptxt));

    int LineOffset = MainFileTxt->GetLineOffset(currentline+MainFileTxt->GetCurrPos());

    wcsncpy(temptxt, lineText, LetterNumInLine);
    temptxt[LetterNumInLine] = 0;

    char * tempstr = wideCharToChar(temptxt, 150);
    wchar_t * tempwstr = NULL;

    int result = OnScreenKeyboard(NULL, tempstr, 150);
    if(result == 1)
    {
        tempwstr = charToWideChar(tempstr);
        wText->replace(LineOffset, LetterNumInLine, tempwstr);
        MainFileTxt->SetText(wText->c_str());
        MainFileTxt->CalcLineOffsets();

        delete(tempwstr);
        delete(tempstr);
        delete(wText);
        return 1;
    }

    delete(tempstr);
    delete(wText);
    return -1;
}

void TextEditor::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == closeBtn)
        SetState(STATE_CLOSED);

    else if(sender == PlusBtn)
    {
        LineEditing = true;
    }
}

void TextEditor::OnPointerHeld(GuiElement *sender, int pointer, POINT p)
{
    if(!userInput[pointer].wpad->ir.valid)
        return;

    TextPointerBtn->PositionChanged(pointer, p.x, p.y);
}

void TextEditor::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t || !triggerupdate)
		return;

	maximizeBtn->Update(t);
	closeBtn->Update(t);
	minimizeBtn->Update(t);
	TextPointerBtn->Update(t);
	PlusBtn->Update(t);

	if(t->Right())
	{
	    for(int i = 0; i < linestodraw; i++)
            MainFileTxt->NextLine();
	}
	else if(t->Left())
	{
	    for(int i = 0; i < linestodraw; i++)
            MainFileTxt->PreviousLine();
	}
	else if(t->Down())
	{
        MainFileTxt->NextLine();
	}
	else if(t->Up())
	{
        MainFileTxt->PreviousLine();
	}
}

TextEditor * TextEditor::LoadFileEd(const char *filepath)
{
	if(!filepath)
		return NULL;

    char *string = (char*)LoadFile((char *)filepath, GetFileSize(filepath));
    wchar_t *wstring = charToWideChar(string);
    free(string);

    TextEditor * Editor = new TextEditor(wstring, 9, filepath);
	Editor->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	Editor->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	Editor->SetPosition(0, 0);

    delete(wstring);
	return Editor;
}
