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
 * for Wii-FileXplorer 2009
 ***************************************************************************/

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "libwiigui/gui.h"
#include "TextPointer.h"

//!Display a list of files
class TextEditor : public GuiWindow
{
	public:
		TextEditor(const wchar_t *intext, int LinesToDraw, const char *path);
		~TextEditor();
		void SetTriggerUpdate(bool set);
		void SetText(const wchar_t *intext);
        void WriteTextFile(const char * path);
        int GetState();
		void ResetState();
		void Update(GuiTrigger * t);
		static TextEditor * LoadFileEd(const char *filepath);
	protected:
        int EditLine();
        void OnButtonClick(GuiElement *sender, int pointer, POINT p);
        void OnPointerHeld(GuiElement *sender, int pointer, POINT p);

		bool triggerupdate;
		bool ExitEditor;
		bool LineEditing;
		bool FileEdited;
		int linestodraw;
		u32 filesize;
		char * filepath;

        /** Buttons **/
		GuiButton * maximizeBtn;
		GuiButton * minimizeBtn;
		GuiButton * closeBtn;
		GuiButton * PlusBtn;
		TextPointer * TextPointerBtn;

        /** Images **/
		GuiImage * bgTexteditorImg;
		GuiImage * closeImg;
		GuiImage * closeImgOver;
		GuiImage * maximizeImg;
		GuiImage * minimizeImg;

        /** ImageDatas **/
		GuiImageData * bgTexteditorData;
		GuiImageData * closeImgData;
		GuiImageData * closeImgOverData;
		GuiImageData * maximizeImgData;
		GuiImageData * minimizeImgData;

        /** Sounds **/
		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;

        /** Triggers **/
		GuiTrigger * trigA;
		GuiTrigger * trigHeldA;
		GuiTrigger * trigPlus;
		GuiTrigger * trigB;

        /** Texts **/
		GuiText * filenameTxt;
		GuiLongText * MainFileTxt;
};

#endif
