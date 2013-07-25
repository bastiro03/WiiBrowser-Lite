/****************************************************************************
 * libwiigui
 * WiiBrowser
 *
 * Tantric 2009
 * modified by gave92
 *
 * gui_keyboard.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include <string>

#include "gui.h"
#include "wiikeyboard/usbkeyboard.h"

#define MAX_KEYBOARD_DISPLAY	390

static char tmptxt[MAX_KEYBOARD_DISPLAY];
bool GuiKeyboard::bInitUSBKeyboard = true;

int CalcMaxLine(char * t)
{
    int len = strlen(t);
    wchar_t *wt = charToWideChar(t);
    int w, startPos = 0;

    for(int i = 0; i < len; i++)
    {
        w = fontSystem[20]->getWidth(&wt[i]);
        if(w < MAX_KEYBOARD_DISPLAY)
            break;

        startPos++;
    }

    delete wt;
    return startPos;
}

char * GuiKeyboard::GetDisplayText(char * t)
{
	if(!t)
		return NULL;

	int len = strlen(t);
    int n = 0;
    int startPos = CalcMaxLine(t);

    for(int i = startPos; i < len; ++i)
    {
        tmptxt[n] = t[i];
        ++n;
    }

    tmptxt[n] = 0;
    CurrentFirstLetter = startPos;

	return tmptxt;
}

/**
 * Constructor for the GuiKeyboard class.
 */

GuiKeyboard::GuiKeyboard(char * t, u32 max)
{
    if(bInitUSBKeyboard) {
		bInitUSBKeyboard = false;
        KEYBOARD_Init(0);
    }
	memset(&keyboardEvent, 0, sizeof(keyboardEvent));
	DeleteDelay = 0;
	BackDelay = 0;
	CurrentFirstLetter = 0;

	if(t)
	{
        CurrentFirstLetter = CalcMaxLine(t);
        if(CurrentFirstLetter < 0)
            CurrentFirstLetter = 0;
	}

	width = 540;
	height = 400;
	shift = 0;
	caps = 0;
	selectable = true;
	focus = 0; // allow focus
	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;
	strncpy(kbtextstr, t, max);
	kbtextstr[max] = 0;
	kbtextmaxlen = max;

	Key thekeys[4][11] = {
	{
		{'1','!'},
		{'2','@'},
		{'3','#'},
		{'4','='},
		{'5','%'},
		{'6','^'},
		{'7','&'},
		{'8','*'},
		{'9','('},
		{'0',')'},
		{'\0','\0'}
	},
	{
		{'q','Q'},
		{'w','W'},
		{'e','E'},
		{'r','R'},
		{'t','T'},
		{'y','Y'},
		{'u','U'},
		{'i','I'},
		{'o','O'},
		{'p','P'},
		{'-','_'}
	},
	{
		{'a','A'},
		{'s','S'},
		{'d','D'},
		{'f','F'},
		{'g','G'},
		{'h','H'},
		{'j','J'},
		{'k','K'},
		{'l','L'},
		{';',':'},
		{'\'','"'}
	},

	{
		{'z','Z'},
		{'x','X'},
		{'c','C'},
		{'v','V'},
		{'b','B'},
		{'n','N'},
		{'m','M'},
		{',','<'},
		{'.','>'},
		{'/','?'},
		{'\0','\0'}
	}
	};
	memcpy(keys, thekeys, sizeof(thekeys));

	keyTextbox = new GuiImageData(keyboard_textbox_png);
	keyTextboxImg = new GuiImage(keyTextbox);
	keyTextboxImg->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	keyTextboxImg->SetPosition(0, 0);
	this->Append(keyTextboxImg);

	kbText = new GuiLongText(GetDisplayText(kbtextstr), 20, (GXColor){0, 0, 0, 0xff});
	kbText->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	kbText->SetLinesToDraw(1);
	this->Append(kbText);

    trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigLeft = new GuiTrigger;
	trigLeft->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
    trigRight = new GuiTrigger;
	trigRight->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);

    GoRight = new GuiButton(1, 1);
	GoRight->SetSoundClick(keySoundClick);
	GoRight->SetTrigger(trigRight);
    GoRight->Clicked.connect(this, &GuiKeyboard::OnPositionMoved);
	this->Append(GoRight);

	GoLeft = new GuiButton(1, 1);
	GoLeft->SetSoundClick(keySoundClick);
	GoLeft->SetTrigger(trigLeft);
    GoLeft->Clicked.connect(this, &GuiKeyboard::OnPositionMoved);
	this->Append(GoLeft);

    TextPointerBtn = new TextPointer(kbText, 0);
	TextPointerBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	TextPointerBtn->SetPosition(7, 13);
    TextPointerBtn->SetHoldable(true);
    TextPointerBtn->SetTrigger(trigHeldA);
    TextPointerBtn->Held.connect(this, &GuiKeyboard::OnPointerHeld);
	this->Append(TextPointerBtn);

    TextPointerBtn->PositionChanged(0, 0, 0);
    TextPointerBtn->SetLetterPosition(MAX_KEYBOARD_DISPLAY-1);

	key = new GuiImageData(keyboard_key_png);
	keyOver = new GuiImageData(keyboard_key_over_png);
	keyMedium = new GuiImageData(keyboard_mediumkey_png);
	keyMediumOver = new GuiImageData(keyboard_mediumkey_over_png);
	keyLarge = new GuiImageData(keyboard_largekey_png);
	keyLargeOver = new GuiImageData(keyboard_largekey_over_png);

	keySoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	keySoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigH = new GuiTrigger;
	trigH->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trig2 = new GuiTrigger;
	trig2->SetSimpleTrigger(-1, WPAD_BUTTON_2, 0);

	keyBackImg = new GuiImage(keyMedium);
	keyBackOverImg = new GuiImage(keyMediumOver);
	keyBackText = new GuiText("Back", 20, (GXColor){0, 0, 0, 0xff});
	keyBack = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyBack->SetImage(keyBackImg);
	keyBack->SetImageOver(keyBackOverImg);
	keyBack->SetLabel(keyBackText);
	keyBack->SetSoundOver(keySoundOver);
	keyBack->SetSoundHold(keySoundClick);
	keyBack->SetTrigger(trigH);
	keyBack->SetTrigger(trig2);
	keyBack->SetPosition(10*42+40, 0*42+80);
	keyBack->SetEffectGrow();
	keyBack->SetHoldable(true);
	this->Append(keyBack);

	keyCapsImg = new GuiImage(keyMedium);
	keyCapsOverImg = new GuiImage(keyMediumOver);
	keyCapsText = new GuiText("Caps", 20, (GXColor){0, 0, 0, 0xff});
	keyCaps = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyCaps->SetImage(keyCapsImg);
	keyCaps->SetImageOver(keyCapsOverImg);
	keyCaps->SetLabel(keyCapsText);
	keyCaps->SetSoundOver(keySoundOver);
	keyCaps->SetSoundClick(keySoundClick);
	keyCaps->SetTrigger(trigA);
	keyCaps->SetTrigger(trig2);
	keyCaps->SetPosition(0, 2*42+80);
	keyCaps->SetEffectGrow();
	this->Append(keyCaps);

	keyShiftImg = new GuiImage(keyMedium);
	keyShiftOverImg = new GuiImage(keyMediumOver);
	keyShiftText = new GuiText("Shift", 20, (GXColor){0, 0, 0, 0xff});
	keyShift = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyShift->SetImage(keyShiftImg);
	keyShift->SetImageOver(keyShiftOverImg);
	keyShift->SetLabel(keyShiftText);
	keyShift->SetSoundOver(keySoundOver);
	keyShift->SetSoundClick(keySoundClick);
	keyShift->SetTrigger(trigA);
	keyShift->SetTrigger(trig2);
	keyShift->SetPosition(21, 3*42+80);
	keyShift->SetEffectGrow();
	this->Append(keyShift);

    keyEnterImg = new GuiImage(keyMedium);
	keyEnterOverImg = new GuiImage(keyMediumOver);
	keyEnterText = new GuiText("Enter", 20, (GXColor){0, 0, 0, 0xff});
	keyEnter = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyEnter->SetImage(keyEnterImg);
	keyEnter->SetImageOver(keyEnterOverImg);
	keyEnter->SetLabel(keyEnterText);
	keyEnter->SetSoundOver(keySoundOver);
	keyEnter->SetSoundClick(keySoundClick);
	keyEnter->SetTrigger(trigA);
	keyEnter->SetTrigger(trig2);
	keyEnter->SetPosition(10*42+61, 1*42+80);
	keyEnter->SetEffectGrow();
	this->Append(keyEnter);

	keySpaceImg = new GuiImage(keyLarge);
	keySpaceOverImg = new GuiImage(keyLargeOver);
	keySpace = new GuiButton(keyLarge->GetWidth(), keyLarge->GetHeight());
	keySpace->SetImage(keySpaceImg);
	keySpace->SetImageOver(keySpaceOverImg);
	keySpace->SetSoundOver(keySoundOver);
	keySpace->SetSoundClick(keySoundClick);
	keySpace->SetTrigger(trigA);
	keySpace->SetTrigger(trig2);
	keySpace->SetPosition(0, 4*42+80);
	keySpace->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	keySpace->SetEffectGrow();
	this->Append(keySpace);

	char txt[2] = { 0, 0 };

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<11; j++)
		{
			if(keys[i][j].ch != '\0')
			{
				txt[0] = keys[i][j].ch;
				keyImg[i][j] = new GuiImage(key);
				keyImgOver[i][j] = new GuiImage(keyOver);
				keyTxt[i][j] = new GuiText(txt, 20, (GXColor){0, 0, 0, 0xff});
				keyTxt[i][j]->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
				// keyTxt[i][j]->SetPosition(0, -10);
				keyBtn[i][j] = new GuiButton(key->GetWidth(), key->GetHeight());
				keyBtn[i][j]->SetImage(keyImg[i][j]);
				keyBtn[i][j]->SetImageOver(keyImgOver[i][j]);
				keyBtn[i][j]->SetSoundOver(keySoundOver);
				keyBtn[i][j]->SetSoundClick(keySoundClick);
				keyBtn[i][j]->SetTrigger(trigA);
				keyBtn[i][j]->SetTrigger(trig2);
				keyBtn[i][j]->SetLabel(keyTxt[i][j]);
				if(i != 1)
                    keyBtn[i][j]->SetPosition(j*42+21*i+40, i*42+80);
                else
                    keyBtn[i][j]->SetPosition(j*42+21*i-1, i*42+80);
				keyBtn[i][j]->SetEffectGrow();
				this->Append(keyBtn[i][j]);
			}
		}
	}
}

/**
 * Destructor for the GuiKeyboard class.
 */
GuiKeyboard::~GuiKeyboard()
{
	delete kbText;
	delete TextPointerBtn;
	delete GoRight;
    delete GoLeft;
	delete keyTextbox;
	delete keyTextboxImg;
	delete keyCapsText;
	delete keyCapsImg;
	delete keyCapsOverImg;
	delete keyCaps;
    delete keyEnterText;
	delete keyEnterImg;
	delete keyEnterOverImg;
	delete keyEnter;
	delete keyShiftText;
	delete keyShiftImg;
	delete keyShiftOverImg;
	delete keyShift;
	delete keyBackText;
	delete keyBackImg;
	delete keyBackOverImg;
	delete keyBack;
	delete keySpaceImg;
	delete keySpaceOverImg;
	delete keySpace;
	delete key;
	delete keyOver;
	delete keyMedium;
	delete keyMediumOver;
	delete keyLarge;
	delete keyLargeOver;
	delete keySoundOver;
	delete keySoundClick;
	delete trigH;
	delete trigA;
	delete trigHeldA;
	delete trigLeft;
	delete trigRight;
	delete trig2;

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<11; j++)
		{
			if(keys[i][j].ch != '\0')
			{
				delete keyImg[i][j];
				delete keyImgOver[i][j];
				delete keyTxt[i][j];
				delete keyBtn[i][j];
			}
		}
	}
}

void GuiKeyboard::AddChar(int pos, char Char)
{
    if(pos < 0)
        return;

    std::string temp(kbtextstr);
    temp.insert(pos, 1, Char);
    strcpy(kbtextstr, temp.c_str());

    MoveText(1);
}

void GuiKeyboard::RemoveChar(int pos)
{
	if (pos < 0 || pos >= (int) strlen(kbtextstr))
		return;

    std::string temp(kbtextstr);
    temp.erase(pos, 1);
    strcpy(kbtextstr, temp.c_str());

    MoveText(-1);
}

void GuiKeyboard::MoveText(int n)
{
    int LettDiff;
    LettDiff = CalcMaxLine(kbtextstr) - CurrentFirstLetter + n;

    CurrentFirstLetter = CalcMaxLine(kbtextstr);
    kbText->SetText(GetDisplayText(kbtextstr));
    TextPointerBtn->UpdateWidth();

    wchar_t *wstr = charToWideChar(kbtextstr);
    int strlength = fontSystem[20]->getWidth(wstr);
    delete(wstr);

    if(strlength > MAX_KEYBOARD_DISPLAY)
        TextPointerBtn->SetLetterPosition(TextPointerBtn->GetCurrentLetter());
    else
        TextPointerBtn->SetLetterPosition(TextPointerBtn->GetCurrentLetter()+LettDiff);
}

void GuiKeyboard::OnPointerHeld(GuiElement *sender, int pointer, POINT p)
{
    TextPointerBtn->PositionChanged(pointer, p.x, p.y);
}

void GuiKeyboard::OnPositionMoved(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == GoLeft)
    {
        int currentPointLetter = TextPointerBtn->GetCurrentLetter();
        currentPointLetter--;
        if(currentPointLetter < 0)
        {
            currentPointLetter = 0;
            CurrentFirstLetter--;
            if(CurrentFirstLetter < 0)
                CurrentFirstLetter = 0;
        }
        kbText->SetText(GetDisplayText(kbtextstr));
        TextPointerBtn->UpdateWidth();
        TextPointerBtn->SetLetterPosition(currentPointLetter);
    }
    else if(sender == GoRight)
    {
        int currentPointLetter = TextPointerBtn->GetCurrentLetter();
        currentPointLetter++;
        int strlength = strlen(kbtextstr);
        if(currentPointLetter > (MAX_KEYBOARD_DISPLAY-1) || currentPointLetter > strlength)
        {
            currentPointLetter--;
            CurrentFirstLetter++;
            if(CurrentFirstLetter > (strlength-MAX_KEYBOARD_DISPLAY+1))
                CurrentFirstLetter = strlength-MAX_KEYBOARD_DISPLAY+1;
        }
        kbText->SetText(GetDisplayText(kbtextstr));
        TextPointerBtn->UpdateWidth();
        TextPointerBtn->SetLetterPosition(currentPointLetter);
    }
}

void GuiKeyboard::Update(GuiTrigger * t)
{
	if(_elements.size() == 0 || (state == STATE_DISABLED && parentElement))
		return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try	{ _elements.at(i)->Update(t); }
		catch (const std::exception& e) { }
	}

	wchar_t charCode = 0;
	u8 keyCode = 0;
	bool update = false;

	++DeleteDelay;
	++BackDelay;

    if(t->chan == 0) {
        // Update only once every frame (50-60 times per second)
		bool bKeyChangeEvent = (KEYBOARD_GetEvent(&keyboardEvent) == 1);
		if(bKeyChangeEvent) {
			if(keyboardEvent.type == KEYBOARD_PRESSED) {
				keyHeldDelay.reset();
			}
			else {
				// key released -> reset symbol
				keyboardEvent.symbol = 0;
			}
		}

		// if the key was not released it -> add it again as "new" event
		if(	   (keyboardEvent.symbol != 0)
			&& (   bKeyChangeEvent
				|| (keyHeldDelay.elapsedMilliSecs() > 500 && DeleteDelay > 15)))	// delay hold key
		{
			if(((keyboardEvent.symbol >> 8) == 0xF2) && (keyboardEvent.symbol & 0xFF) < 0x80) {
				// this is usually a numpad
				charCode = keyboardEvent.symbol & 0xFF;
			}
			else if(  (keyboardEvent.symbol < 0xD800)										// this is usually a normal character
					|| (keyboardEvent.symbol >= 62102 && keyboardEvent.symbol <= 62105)		// up/down/left/right numpad
					|| (keyboardEvent.symbol >= 62340 && keyboardEvent.symbol <= 62343))	// up/down/left/right arrows
			{
				charCode = keyboardEvent.symbol;
			}

			keyCode = keyboardEvent.modifiers;

			if(charCode != 0 || keyCode != 0)
			    DeleteDelay = 0;
		}
	}

	if(keySpace->GetState() == STATE_CLICKED            || charCode == 0x20)
	{
		if(strlen(kbtextstr) < kbtextmaxlen)
		{
			AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), ' ');
		}
		keySpace->SetState(STATE_SELECTED, t->chan);
	}
    else if(keyEnter->GetState() == STATE_CLICKED            || charCode == 0x0d)
	{
		if(strlen(kbtextstr) < kbtextmaxlen)
		{
			AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), '\n');
		}
		keyEnter->SetState(STATE_SELECTED, t->chan);
	}
	else if(keyBack->GetState() == STATE_HELD)
	{
		if(BackDelay > 80)
		{
            if(strlen(kbtextstr) > 0)
            {
                RemoveChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter()-1);
            }
            BackDelay = 0;
		}
	}
	else if(charCode == 0x08)
	{
        if(strlen(kbtextstr) > 0)
        {
            RemoveChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter()-1);
        }
	}
	else if(keyShift->GetState() == STATE_CLICKED       || keyCode == 0xf101)
	{
		shift ^= 1;
		keyShift->SetState(STATE_SELECTED, t->chan);
		update = true;
	}
	else if(keyCaps->GetState() == STATE_CLICKED        || keyCode == 0xf105)
	{
		caps ^= 1;
		keyCaps->SetState(STATE_SELECTED, t->chan);
		USBKeyboard_SetLed(USBKEYBOARD_LEDCAPS, caps);
		update = true;
	}

	if (charCode > 0x20 && charCode < 0xD800)
	{
		if(strlen(kbtextstr) < kbtextmaxlen)
		{
		    AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), charCode);
        }

		if(shift)
        {
            shift ^= 1;
            update = true;
        }
	}

	char txt[2] = { 0, 0 };

	startloop:

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<11; j++)
		{
			if(keys[i][j].ch != '\0')
			{
				if(update)
				{
					if(shift || caps)
						txt[0] = keys[i][j].chShift;
					else
						txt[0] = keys[i][j].ch;

					keyTxt[i][j]->SetText(txt);
				}

				if(keyBtn[i][j]->GetState() == STATE_CLICKED)
				{
					if(strlen(kbtextstr) < kbtextmaxlen)
					{
						if(shift || caps)
						{
							AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), keys[i][j].chShift);
						}
						else
						{
							AddChar(CurrentFirstLetter+TextPointerBtn->GetCurrentLetter(), keys[i][j].ch);
						}
					}

					keyBtn[i][j]->SetState(STATE_SELECTED, t->chan);

					if(shift)
					{
						shift ^= 1;
						update = true;
						goto startloop;
					}
				}
			}
		}
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
