/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_imagedata.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const u8 * i, int maxw, int maxh)
{
	data = NULL;
	width = 0;
	height = 0;

	// utils/pngu.c's DecodePNG signature is
	//   u8* DecodePNG(const u8* src, int* width, int* height, u8* dst)
	// The old libwiigui/pngu.h had a 5-arg form taking (maxw, maxh) that
	// was never matched by any implementation. Max-size clamping is now
	// handled inside PNGU_DecodeTo4x4RGBA8 (see MAX_TEX_WIDTH/HEIGHT in
	// pngu.c), so the maxw/maxh hints from callers are informational only.
	(void)maxw; (void)maxh;

	if(i)
		data = DecodePNG(i, &width, &height, NULL);
}

/**
 * Destructor for the GuiImageData class.
 */
GuiImageData::~GuiImageData()
{
	if(data)
	{
		free(data);
		data = NULL;
	}
}

u8 * GuiImageData::GetImage()
{
	return data;
}

int GuiImageData::GetWidth()
{
	return width;
}

int GuiImageData::GetHeight()
{
	return height;
}
