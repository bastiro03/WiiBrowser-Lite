/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * video.h
 * Video routines
 ***************************************************************************/

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <ogcsys.h>

void InitVideo();
void StopGX();
void ResetVideo_Menu();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY, u8 alphaF);
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled);
u8 *TakeScreenshot(bool global);
void SaveScreenshot(char *path);

extern int screenheight;
extern int screenwidth;
extern u32 FrameTimer;
extern u8 *videoScreenshot;

/* `vmode` must match the `extern "C"` declaration in
 * include/mplayer/osdep/gx_supp.h (included from menu.cpp for the
 * legacy-but-still-header-only MPlayer glue). Keeping C linkage here
 * avoids a conflicting-declaration error when both headers are seen
 * in the same translation unit. */
#ifdef __cplusplus
extern "C" {
#endif
extern GXRModeObj *vmode;
#ifdef __cplusplus
}
#endif

#endif
