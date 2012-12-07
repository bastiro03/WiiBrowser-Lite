/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * video.h
 * Video routines
 ***************************************************************************/

#ifndef _VIDEO_H_
#define _VIDEO_H_
#define MAX_TEX_WIDTH 768
#define MAX_TEX_HEIGHT 480
#include <ogcsys.h>

void InitVideo ();
void StopGX();
void ResetVideo_Menu();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY, u8 alphaF );
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled);
void Menu_DrawGif(u8 data[], u16 width, u16 height, u8 format, f32 xpos, f32 ypos, f32 zpos, f32 degrees, f32 scaleX, f32 scaleY, u8 alpha, f32 minwidth, f32 maxwidth, f32 minheight, f32 maxheight);

extern int screenheight;
extern int screenwidth;
extern u32 FrameTimer;

#endif
