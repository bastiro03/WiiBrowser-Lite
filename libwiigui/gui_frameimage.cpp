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
#include "gui_frameimage.h"

GuiFrameImage::GuiFrameImage(int w, int h)
{
	width = w;
	height = h;
	Precision = 6;
	Margin = 8.0f;
	ShadowWidth = 3.0f;

	UpperQuadUpper = (GXColor)
	{
		0xFC, 0xFD, 0xFE, 0xFF
	}
	MainQuadUpper = (GXColor)
	{
		0xE5, 0xE9, 0xEF, 0xFF
	}
	MainQuadLower = (GXColor)
	{
		0xC3, 0xC8, 0xD0, 0xFF
	}
	SideQuadUpper = (GXColor)
	{
		0xC6, 0xC9, 0xCF, 0xFF
	}
	SideQuadLower = (GXColor)
	{
		0xAC, 0xB0, 0xB6, 0xFF
	}
	ShadowUpper = (GXColor)
	{
		0x7B, 0x7B, 0x7B, 0xFF
	}
	ShadowLower = (GXColor)
	{
		0x30, 0x30, 0x30, 0xFF
	}

	DrawListSize = 1152;
	DrawList = static_cast<u8*>(memalign(32, DrawListSize + 64));
	CreateDrawList();
}

GuiFrameImage::~GuiFrameImage()
{
	free(DrawList);
}

void GuiFrameImage::CreateDrawList()
{
	GX_BeginDispList(DrawList, DrawListSize + 64);

	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	oldWidth = GetWidth();
	oldHeight = GetHeight();
	oldX = GetLeft();
	oldY = GetTop();
	oldZ = 0;
	oldAlpha = GetAlpha();

	f32 alpha = oldAlpha;
	f32 z = oldZ;

	f32 x1 = oldX;
	f32 y1 = oldY;
	f32 x2 = x1 + oldWidth;
	f32 y2 = y1 + oldHeight;

	//! Upper QUAD
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x1 + Margin, y1, z);
	GX_Color4u8(UpperQuadUpper.r, UpperQuadUpper.g, UpperQuadUpper.b, alpha);

	GX_Position3f32(x2 - Margin, y1, z);
	GX_Color4u8(UpperQuadUpper.r, UpperQuadUpper.g, UpperQuadUpper.b, alpha);

	GX_Position3f32(x2 - Margin, y1 + Margin, z);
	GX_Color4u8(MainQuadUpper.r, MainQuadUpper.g, MainQuadUpper.b, alpha);

	GX_Position3f32(x1 + Margin, y1 + Margin, z);
	GX_Color4u8(MainQuadUpper.r, MainQuadUpper.g, MainQuadUpper.b, alpha);
	GX_End();

	//! Upper/Left Corner Circle
	GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, Precision + 1);
	{
		f32 deg;
		int r = SideQuadUpper.r;
		int g = SideQuadUpper.g;
		int b = SideQuadUpper.b;
		f32 r_p = static_cast<float>(UpperQuadUpper.r - r) / static_cast<float>(Precision - 1);
		f32 g_p = static_cast<float>(UpperQuadUpper.g - g) / static_cast<float>(Precision - 1);
		f32 b_p = static_cast<float>(UpperQuadUpper.b - b) / static_cast<float>(Precision - 1);
		GX_Position3f32(x1 + Margin, y1 + Margin, z);
		GX_Color4u8(MainQuadUpper.r, MainQuadUpper.g, MainQuadUpper.b, alpha);

		for (int i = 0; i < Precision; ++i)
		{
			deg = DegToRad(180 + i * 90 / (f32)(Precision - 1));
			GX_Position3f32(x1 + Margin + Margin * cos(deg), y1 + Margin + Margin * sin(deg), z);
			GX_Color4u8(r + r_p * i, g + g_p * i, b + b_p * i, alpha);
		}
	}
	GX_End();

	//! Left QUAD
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x1, y1 + Margin, z);
	GX_Color4u8(SideQuadUpper.r, SideQuadUpper.g, SideQuadUpper.b, alpha);

	GX_Position3f32(x1 + Margin, y1 + Margin, z);
	GX_Color4u8(MainQuadUpper.r, MainQuadUpper.g, MainQuadUpper.b, alpha);

	GX_Position3f32(x1 + Margin, y2 - Margin, z);
	GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);

	GX_Position3f32(x1, y2 - Margin, z);
	GX_Color4u8(SideQuadLower.r, SideQuadLower.g, SideQuadLower.b, alpha);
	GX_End();

	//! Lower Left Corner Circle
	GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, Precision * 2 + 2);
	{
		//! Transparent shadow
		f32 deg;
		int r = ShadowLower.r;
		int g = ShadowLower.g;
		int b = ShadowLower.b;
		f32 a = 0.1f * alpha;
		f32 r_p = static_cast<float>(SideQuadLower.r - r) / static_cast<float>(Precision - 1);
		f32 g_p = static_cast<float>(SideQuadLower.g - g) / static_cast<float>(Precision - 1);
		f32 b_p = static_cast<float>(SideQuadLower.b - b) / static_cast<float>(Precision - 1);
		f32 a_p = static_cast<float>(alpha - a) / static_cast<float>(Precision - 1);
		GX_Position3f32(x1 + Margin, y2 - Margin, z);
		GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);

		for (int i = 0; i < Precision; ++i)
		{
			deg = DegToRad(90 + i * 90 / (f32)(Precision - 1));
			GX_Position3f32(x1 + Margin + Margin * cos(deg), y2 - Margin + Margin * sin(deg), z);
			GX_Color4u8(r + r_p * i, g + g_p * i, b + b_p * i, a + a_p * i);
		}

		//! Opaque
		r = ShadowUpper.r;
		g = ShadowUpper.g;
		b = ShadowUpper.b;
		r_p = static_cast<float>(SideQuadLower.r - r) / static_cast<float>(Precision - 1);
		g_p = static_cast<float>(SideQuadLower.g - g) / static_cast<float>(Precision - 1);
		b_p = static_cast<float>(SideQuadLower.b - b) / static_cast<float>(Precision - 1);

		GX_Position3f32(x1 + Margin, y2 - Margin, z);
		GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);

		for (int i = 0; i < Precision; ++i)
		{
			deg = DegToRad(90 + i * 90 / (f32)(Precision - 1));
			GX_Position3f32(x1 + Margin + Margin * cos(deg), y2 - Margin + (Margin - ShadowWidth) * sin(deg), z);
			GX_Color4u8(r + r_p * i, g + g_p * i, b + b_p * i, alpha);
		}
	}
	GX_End();

	//! Lower QUAD
	GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
	//! Transparent shadow quad
	GX_Position3f32(x1 + Margin, y2 - Margin, z);
	GX_Color4u8(ShadowUpper.r, ShadowUpper.g, ShadowUpper.b, alpha);

	GX_Position3f32(x2 - Margin, y2 - Margin, z);
	GX_Color4u8(ShadowUpper.r, ShadowUpper.g, ShadowUpper.b, alpha);

	GX_Position3f32(x2 - Margin, y2, z);
	GX_Color4u8(ShadowLower.r, ShadowLower.g, ShadowLower.b, 0.1f * alpha);

	GX_Position3f32(x1 + Margin, y2, z);
	GX_Color4u8(ShadowLower.r, ShadowLower.g, ShadowLower.b, 0.1f * alpha);

	//! Opaque quad
	GX_Position3f32(x1 + Margin, y2 - Margin, z);
	GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);

	GX_Position3f32(x2 - Margin, y2 - Margin, z);
	GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);

	GX_Position3f32(x2 - Margin, y2 - ShadowWidth, z);
	GX_Color4u8(ShadowUpper.r, ShadowUpper.g, ShadowUpper.b, alpha);

	GX_Position3f32(x1 + Margin, y2 - ShadowWidth, z);
	GX_Color4u8(ShadowUpper.r, ShadowUpper.g, ShadowUpper.b, alpha);
	GX_End();

	//! Lower Right Corner Circle
	GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, Precision * 2 + 2);
	{
		f32 deg;
		int r = SideQuadLower.r;
		int g = SideQuadLower.g;
		int b = SideQuadLower.b;
		f32 a = alpha;
		f32 r_p = static_cast<float>(ShadowLower.r - r) / static_cast<float>(Precision - 1);
		f32 g_p = static_cast<float>(ShadowLower.g - g) / static_cast<float>(Precision - 1);
		f32 b_p = static_cast<float>(ShadowLower.b - b) / static_cast<float>(Precision - 1);
		f32 a_p = static_cast<float>(0.1f * alpha - a) / static_cast<float>(Precision - 1);
		GX_Position3f32(x2 - Margin, y2 - Margin, z);
		GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);

		for (int i = 0; i < Precision; ++i)
		{
			deg = DegToRad(i * 90 / (f32)(Precision - 1));
			GX_Position3f32(x2 - Margin + Margin * cos(deg), y2 - Margin + Margin * sin(deg), z);
			GX_Color4u8(r + r_p * i, g + g_p * i, b + b_p * i, a + a_p * i);
		}

		r_p = static_cast<float>(ShadowUpper.r - r) / static_cast<float>(Precision - 1);
		g_p = static_cast<float>(ShadowUpper.g - g) / static_cast<float>(Precision - 1);
		b_p = static_cast<float>(ShadowUpper.b - b) / static_cast<float>(Precision - 1);

		GX_Position3f32(x2 - Margin, y2 - Margin, z);
		GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);

		for (int i = 0; i < Precision; ++i)
		{
			deg = DegToRad(i * 90 / (f32)(Precision - 1));
			GX_Position3f32(x2 - Margin + Margin * cos(deg), y2 - Margin + (Margin - ShadowWidth) * sin(deg), z);
			GX_Color4u8(r + r_p * i, g + g_p * i, b + b_p * i, alpha);
		}
	}
	GX_End();

	//! Right QUAD
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x2 - Margin, y1 + Margin, z);
	GX_Color4u8(MainQuadUpper.r, MainQuadUpper.g, MainQuadUpper.b, alpha);

	GX_Position3f32(x2, y1 + Margin, z);
	GX_Color4u8(SideQuadUpper.r, SideQuadUpper.g, SideQuadUpper.b, alpha);

	GX_Position3f32(x2, y2 - Margin, z);
	GX_Color4u8(SideQuadLower.r, SideQuadLower.g, SideQuadLower.b, alpha);

	GX_Position3f32(x2 - Margin, y2 - Margin, z);
	GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);
	GX_End();

	//! Upper/Left Corner Circle
	GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, Precision + 1);
	{
		f32 deg;
		int r = UpperQuadUpper.r;
		int g = UpperQuadUpper.g;
		int b = UpperQuadUpper.b;
		f32 r_p = static_cast<float>(SideQuadUpper.r - r) / static_cast<float>(Precision - 1);
		f32 g_p = static_cast<float>(SideQuadUpper.g - g) / static_cast<float>(Precision - 1);
		f32 b_p = static_cast<float>(SideQuadUpper.b - b) / static_cast<float>(Precision - 1);
		GX_Position3f32(x2 - Margin, y1 + Margin, z);
		GX_Color4u8(MainQuadUpper.r, MainQuadUpper.g, MainQuadUpper.b, alpha);

		for (int i = 0; i < Precision; ++i)
		{
			deg = DegToRad(270 + i * 90 / (f32)(Precision - 1));
			GX_Position3f32(x2 - Margin + Margin * cos(deg), y1 + Margin + Margin * sin(deg), z);
			GX_Color4u8(r + r_p * i, g + g_p * i, b + b_p * i, alpha);
		}
	}
	GX_End();

	//! Main QUAD
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x1 + Margin, y1 + Margin, z);
	GX_Color4u8(MainQuadUpper.r, MainQuadUpper.g, MainQuadUpper.b, alpha);

	GX_Position3f32(x2 - Margin, y1 + Margin, z);
	GX_Color4u8(MainQuadUpper.r, MainQuadUpper.g, MainQuadUpper.b, alpha);

	GX_Position3f32(x2 - Margin, y2 - Margin, z);
	GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);

	GX_Position3f32(x1 + Margin, y2 - Margin, z);
	GX_Color4u8(MainQuadLower.r, MainQuadLower.g, MainQuadLower.b, alpha);
	GX_End();

	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);

	DrawListSize = GX_EndDispList();
}

void GuiFrameImage::Draw()
{
	if (!IsVisible())
		return;

	if (oldWidth != GetWidth()
		|| oldHeight != GetHeight()
		|| oldX != GetLeft()
		|| oldY != GetTop()
		|| oldZ != 0
		|| oldAlpha != GetAlpha())
	{
		CreateDrawList();
	}

	GX_CallDispList(DrawList, DrawListSize);
}
