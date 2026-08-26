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
#ifndef GUI_FRAMEIMAGE_H_
#define GUI_FRAMEIMAGE_H_

#include "libwiigui/gui.h"

class GuiFrameImage : public GuiElement
{
	public:
		//!Constructor
		//!\param w Width of window
		//!\param h Height of window
		GuiFrameImage(int w, int h);
		//!Destructor
		virtual ~GuiFrameImage();
		//!Draw image process
		virtual void Draw();
		//! Set colors
		void SetColorUpperQuadUpper(const GXColor &c) { UpperQuadUpper = c; oldX = 0; }
		void SetColorMainQuadUpper(const GXColor &c) { MainQuadUpper = c; oldX = 0; }
		void SetColorMainQuadLower(const GXColor &c) { MainQuadLower = c; oldX = 0; }
		void SetColorSideQuadUpper(const GXColor &c) { SideQuadUpper = c; oldX = 0; }
		void SetColorSideQuadLower(const GXColor &c) { SideQuadLower = c; oldX = 0; }
		void SetColorShadowUpper(const GXColor &c) { ShadowUpper = c; oldX = 0; }
		void SetColorShadowLower(const GXColor &c) { ShadowLower = c; oldX = 0; }
	private:
		//!Create the gx draw list
		void CreateDrawList();

		int Precision;
		f32 Margin;
		f32 ShadowWidth;
		u8 *DrawList;
		u32 DrawListSize;

		GXColor UpperQuadUpper;
		GXColor MainQuadUpper;
		GXColor MainQuadLower;
		GXColor SideQuadUpper;
		GXColor SideQuadLower;
		GXColor ShadowUpper;
		GXColor ShadowLower;

		//! Re-Init on variable change
		int oldX, oldY, oldZ;
		int oldWidth, oldHeight;
		int oldAlpha;
};

#endif
