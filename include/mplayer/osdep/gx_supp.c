/****************************************************************************
*	gx_supp.c - Generic GX Support for Emulators
*	softdev 2007
*	dhewg 2008
*	sepp256 2008 - Coded YUV->RGB conversion in TEV.
*	Extrems 2009-2011
*
*	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation; either version 2 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License along
*	with this program; if not, write to the Free Software Foundation, Inc.,
*	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* NGC GX Video Functions
*
* These are pretty standard functions to setup and use GX scaling.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <ogc/mutex.h>
#include <ogc/lwp.h>
#include <ogc/lwp_heap.h>
#include <ogc/system.h>
#include <ogc/machine/asm.h>
#include <ogc/machine/processor.h>

#include "gx_supp.h"
#include "ave-rvl.h"
#include "mem2.h"
#include "libvo/csputils.h"
#include "libvo/video_out.h"

#define HASPECT 320
#define VASPECT 240

/*** 2D ***/
static st_mem2_area *vi_area = NULL;

static u32 whichfb;
static void *xfb[2];
GXRModeObj *vmode = NULL;

int screenwidth = 640;
int screenheight = 480;

/*** 3D GX ***/
static void *gp_fifo;
static u8 dlist[32] ATTRIBUTE_ALIGN(32);
static bool draw_pending, flip_pending;

/*** Texture memory ***/
static st_mem2_area *gx_area = NULL;

static u8 *Ytexture = NULL;
static u8 *Utexture = NULL;
static u8 *Vtexture = NULL;

static u32 Ytexsize, UVtexsize;

static GXTexObj YtexObj, UtexObj, VtexObj;
static u16 Ywidth, Yheight, UVwidth, UVheight;

/* New texture based scaler */
static f32 square[] ATTRIBUTE_ALIGN(32) = {
	-HASPECT,  VASPECT,
	 HASPECT,  VASPECT,
	 HASPECT, -VASPECT,
	-HASPECT, -VASPECT,
};

static GXColor colors[] ATTRIBUTE_ALIGN(32) = {
	{0, 255, 0, 255}	//G
};

static f32 Ytexcoords[] ATTRIBUTE_ALIGN(32) = {
	0.0, 0.0,
	1.0, 0.0,
	1.0, 1.0,
	0.0, 1.0,
};

static f32 UVtexcoords[] ATTRIBUTE_ALIGN(32) = {
	0.0, 0.0,
	1.0, 0.0,
	1.0, 1.0,
	0.0, 1.0,
};

void DCBlockFlush(void *);

void mpviSetup(int video_mode, bool overscan)
{
	VIDEO_Init();
	
	switch(video_mode) {
		case 1:		// NTSC (480i)
			vmode = &TVNtsc480IntDf;
			break;
		case 2:		// Progressive (480p)
			vmode = &TVNtsc480Prog;
			break;
		case 3:		// PAL (50Hz)
			vmode = &TVPal574IntDfScale;
			break;
		case 4:		// PAL (60Hz)
			vmode = &TVEurgb60Hz480IntDf;
			break;
		default:
			vmode = VIDEO_GetPreferredMode(NULL);
	}
	
	bool is_pal, wide_mode;
	
	is_pal = (vmode->viTVMode >> 2) == VI_PAL;
#ifdef HW_RVL
	wide_mode = CONF_GetAspectRatio() == CONF_ASPECT_16_9;
#endif
	
	int videoWidth = is_pal ? VI_MAX_WIDTH_PAL : VI_MAX_WIDTH_NTSC;
	int videoHeight = is_pal ? VI_MAX_HEIGHT_PAL : VI_MAX_HEIGHT_NTSC;
	
	float scanX = wide_mode ? 0.95 : 0.93;
	float scanY = !is_pal ? 0.95 : 0.94;
	
	if (overscan) {
		vmode->viHeight = videoHeight * scanY;
		vmode->viHeight += vmode->viHeight % 2;
	} else vmode->viHeight = videoHeight;
	
	vmode->xfbHeight = vmode->viHeight;
	vmode->efbHeight = MIN(vmode->xfbHeight, 528);
	
#ifdef HW_RVL
	if (wide_mode)
		screenwidth = ceil((screenheight / 9.0) * 16.0);
	else screenwidth = ceil((screenheight / 3.0) * 4.0);
#endif
	
	if (overscan) {
		vmode->viWidth = videoWidth * scanX;
		vmode->viWidth = (vmode->viWidth + 15) & ~15;
	} else vmode->viWidth = videoWidth;
	
#ifndef HW_DOL
	vmode->fbWidth = vmode->viWidth;
#endif
	
	vmode->viXOrigin = (videoWidth - vmode->viWidth) / 2;
	vmode->viYOrigin = (videoHeight - vmode->viHeight) / 2;
	
	VIDEO_Configure(vmode);
	
#ifdef HW_RVL
	if (vi_area == NULL) {
		u32 xfbsize = (VI_MAX_WIDTH_PAL * VI_DISPLAY_PIX_SZ) * (VI_MAX_HEIGHT_PAL + 4);
		vi_area = mem2_area_alloc(xfbsize * 2);
		
		if (xfb[0] == NULL)
			xfb[0] = MEM_K0_TO_K1(__lwp_heap_allocate(&vi_area->heap, xfbsize));
		if (xfb[1] == NULL)
			xfb[1] = MEM_K0_TO_K1(__lwp_heap_allocate(&vi_area->heap, xfbsize));
	}
#else
	if (xfb[1]) free(MEM_K1_TO_K0(xfb[1]));
	if (xfb[0]) free(MEM_K1_TO_K0(xfb[0]));
	
	xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
#endif
	
	VIDEO_ClearFrameBuffer(vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer(vmode, xfb[1], COLOR_BLACK);
	
	VIDEO_SetNextFramebuffer(xfb[whichfb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	
	VIDEO_WaitVSync();
	
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else
	    while (VIDEO_GetNextField())
	    	VIDEO_WaitVSync();
}

void mpviClear()
{
	VIDEO_SetPreRetraceCallback(NULL);
	VIDEO_SetPostRetraceCallback(NULL);
	
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	
	VIDEO_WaitVSync();
	
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

static void drawdone_cb(void)
{
	draw_pending = false;
	
	if (!vo_vsync) {
		VIDEO_SetNextFramebuffer(xfb[whichfb]);
		VIDEO_Flush();
		whichfb ^= 1;
	} else flip_pending = true;
}

static void vblank_cb(u32 retraceCnt)
{
	if (vo_vsync && flip_pending) {
		VIDEO_SetNextFramebuffer(xfb[whichfb]);
		VIDEO_Flush();
		whichfb ^= 1;
		flip_pending = false;
	}
}

void mpgxInit(bool vf)
{
	Mtx44 perspective;
	
	gp_fifo = memalign(32, GX_FIFO_MINSIZE);
	memset(gp_fifo, 0x00, GX_FIFO_MINSIZE);
	
	GX_Init(gp_fifo, GX_FIFO_MINSIZE);
	
	GX_SetViewport(0, 0, vmode->fbWidth, vmode->efbHeight, 0.0, 1.0);
	GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);
#ifndef HW_DOL
	GX_SetDispCopySrc(0, 0, ((vmode->fbWidth >> 1) + 15) & ~15, vmode->efbHeight);
#else
	GX_SetDispCopySrc(0, 0, vmode->fbWidth, vmode->efbHeight);
#endif

	f32 yscale = GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight);
	u32 xfbHeight = GX_SetDispCopyYScale(yscale);
	
	GX_SetDispCopyDst(vmode->fbWidth, xfbHeight);
	GX_SetCopyClear((GXColor){0x00, 0x00, 0x00, 0xFF}, GX_MAX_Z24);
	GX_SetCopyFilter(GX_FALSE, NULL, vf, vmode->vfilter);
	GX_SetFieldMode(GX_FALSE, GX_DISABLE);
	
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetClipMode(GX_DISABLE);
	
	GX_SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
	
	guOrtho(perspective, screenheight / 2.0, -(screenheight / 2.0), -(screenwidth / 2.0), screenwidth / 2.0, 0.0, 1.0);
	GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);
	
	GX_CopyDisp(xfb[whichfb ^ 1], GX_TRUE);
	
	VIDEO_SetPreRetraceCallback(vblank_cb);
	GX_SetDrawDoneCallback(drawdone_cb);
	GX_Flush();
}

void mpgxSetupYUVp(int colorspace, bool levelconv)
{
	//Setup TEV
	GX_SetNumChans(1);
	GX_SetNumTexGens(3);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
	
	//Y'UV->RGB formulation 2
	GX_SetNumTevStages(12);
	GX_SetTevKColor(GX_KCOLOR0, (GXColor){255,   0,   0, levelconv ? 18 : 0});	//R {1, 0, 0, 16*1.164}
	GX_SetTevKColor(GX_KCOLOR1, (GXColor){  0,   0, 255, levelconv ? 41 : 0});	//B {0, 0, 1, 0.164}
	
	static const GXColor uv_coeffs[MP_CSP_COUNT][2] = {
		[MP_CSP_DEFAULT] = {
			{203, 103,   0, 255},	// {1.596/2, 0.813/2, 0}
			{  0,  24, 128, 255}	// {0, 0.391/4, 2.018/4}
		},
		[MP_CSP_BT_601] = {
			{179,  90,   0, 255},	// {1.403/2, 0.714/2, 0}
			{  0,  21, 112, 255}	// {0, 0.344/4, 1.773/4}
		},
		[MP_CSP_BT_709] = {
			{200,  59,   0, 255},	// {1.5701/2, 0.4664/2, 0}
			{  0,  11, 118, 255}	// {0, 0.187/4, 1.8556/4}
		},
		[MP_CSP_SMPTE_240M] = {
			{201,  63,   0, 255},	// {1.5756/2, 0.5/2, 0}
			{  0,  13, 116, 255}	// {0, 0.2253/4, 1.827/4}
		},
		[MP_CSP_EBU] = {
			{145,  73,   0, 255},	// {1.140/2, 0.581/2, 0}
			{  0,  24, 129, 255}	// {0, 0.396/4, 2.029/4}
		},
	};
	
	GX_SetTevKColor(GX_KCOLOR2, uv_coeffs[colorspace][0]);
	GX_SetTevKColor(GX_KCOLOR3, uv_coeffs[colorspace][1]);
	
	//Stage 0: TEVREG0 <- { 0, 2Um, 2Up }; TEVREG0A <- {16*1.164}
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K1);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_RASC, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_SUBHALF, GX_CS_SCALE_2, GX_ENABLE, GX_TEVREG0);
	GX_SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVREG0);
	
	//Stage 1: TEVREG1 <- { 0, 2Up, 2Um };
	GX_SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
	GX_SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
	GX_SetTevColorIn(GX_TEVSTAGE1, GX_CC_KONST, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_SUBHALF, GX_CS_SCALE_2, GX_ENABLE, GX_TEVREG1);
	GX_SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Stage 2: TEVREG2 <- { Vp, Vm, 0 }
	GX_SetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K0);
	GX_SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0);
	GX_SetTevColorIn(GX_TEVSTAGE2, GX_CC_RASC, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_SUBHALF, GX_CS_SCALE_1, GX_ENABLE, GX_TEVREG2);
	GX_SetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Stage 3: TEVPREV <- { (Vm), (Vp), 0 }
	GX_SetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0);
	GX_SetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0);
	GX_SetTevColorIn(GX_TEVSTAGE3, GX_CC_KONST, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_SUBHALF, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Stage 4: TEVPREV <- { (-1.598Vm), (-0.813Vp), 0 }; TEVPREVA <- {Y' - 16*1.164}
	GX_SetTevKColorSel(GX_TEVSTAGE4, GX_TEV_KCSEL_K2);
	GX_SetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE4, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_2, GX_DISABLE, GX_TEVPREV);
	GX_SetTevKAlphaSel(GX_TEVSTAGE4, GX_TEV_KASEL_1);
	GX_SetTevAlphaIn(GX_TEVSTAGE4, GX_CA_ZERO, GX_CA_KONST, GX_CA_A0, GX_CA_TEXA);
	GX_SetTevAlphaOp(GX_TEVSTAGE4, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
	
	//Stage 5: TEVPREV <- { -1.598Vm (+1.139/2Vp), -0.813Vp +0.813/2Vm), 0 }; TEVREG1A <- {Y' -16*1.164 - Y'*0.164} = {(Y'-16)*1.164}
	GX_SetTevKColorSel(GX_TEVSTAGE5, GX_TEV_KCSEL_K2);
	GX_SetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE5, GX_CC_ZERO, GX_CC_KONST, GX_CC_C2, GX_CC_CPREV);
	GX_SetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
	GX_SetTevKAlphaSel(GX_TEVSTAGE5, GX_TEV_KASEL_K1_A);
	GX_SetTevAlphaIn(GX_TEVSTAGE5, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_APREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVREG1);
	
	//Stage 6: TEVPREV <- {	-1.598Vm (+1.598Vp), -0.813Vp (+0.813Vm), 0 } = {	(+1.598V), (-0.813V), 0 }
	GX_SetTevKColorSel(GX_TEVSTAGE6, GX_TEV_KCSEL_K2);
	GX_SetTevOrder(GX_TEVSTAGE6, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE6, GX_CC_ZERO, GX_CC_KONST, GX_CC_C2, GX_CC_CPREV);
	GX_SetTevColorOp(GX_TEVSTAGE6, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE6, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE6, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Stage 7: TEVPREV <- {	((Y'-16)*1.164) +1.598V, ((Y'-16)*1.164) -0.813V, ((Y'-16)*1.164) }
	GX_SetTevKColorSel(GX_TEVSTAGE7, GX_TEV_KCSEL_1);
	GX_SetTevOrder(GX_TEVSTAGE7, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE7, GX_CC_ZERO, GX_CC_ONE, GX_CC_A1, GX_CC_CPREV);
	GX_SetTevColorOp(GX_TEVSTAGE7, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE7, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE7, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Stage 8: TEVPREV <- {	(Y'-16)*1.164 +1.598V, (Y'-16)*1.164 -0.813V (-.394/2Up), (Y'-16)*1.164 (-2.032/2Um)}
	GX_SetTevKColorSel(GX_TEVSTAGE8, GX_TEV_KCSEL_K3);
	GX_SetTevOrder(GX_TEVSTAGE8, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE8, GX_CC_ZERO, GX_CC_KONST, GX_CC_C1, GX_CC_CPREV);
	GX_SetTevColorOp(GX_TEVSTAGE8, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE8, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE8, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Stage 9: TEVPREV <- { (Y'-16)*1.164 +1.598V, (Y'-16)*1.164 -0.813V (-.394Up), (Y'-16)*1.164 (-2.032Um)}
	GX_SetTevKColorSel(GX_TEVSTAGE9, GX_TEV_KCSEL_K3);
	GX_SetTevOrder(GX_TEVSTAGE9, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE9, GX_CC_ZERO, GX_CC_KONST, GX_CC_C1, GX_CC_CPREV);
	GX_SetTevColorOp(GX_TEVSTAGE9, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE9, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE9, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Stage 10: TEVPREV <- { (Y'-16)*1.164 +1.598V, (Y'-16)*1.164 -0.813V -.394Up (+.394/2Um), (Y'-16)*1.164 -2.032Um (+2.032/2Up)}
	GX_SetTevKColorSel(GX_TEVSTAGE10, GX_TEV_KCSEL_K3);
	GX_SetTevOrder(GX_TEVSTAGE10, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE10, GX_CC_ZERO, GX_CC_KONST, GX_CC_C0, GX_CC_CPREV);
	GX_SetTevColorOp(GX_TEVSTAGE10, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE10, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE10, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Stage 11: TEVPREV <- { (Y'-16)*1.164 +1.598V, (Y'-16)*1.164 -0.813V -.394Up (+.394Um), (Y'-16)*1.164 -2.032Um (+2.032Up)} = { (Y'-16)*1.164 +1.139V, (Y'-16)*1.164 -0.58V -.394U, (Y'-16)*1.164 +2.032U}
	GX_SetTevKColorSel(GX_TEVSTAGE11, GX_TEV_KCSEL_K3);
	GX_SetTevOrder(GX_TEVSTAGE11, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLORNULL);
	GX_SetTevColorIn(GX_TEVSTAGE11, GX_CC_ZERO, GX_CC_KONST, GX_CC_C0, GX_CC_CPREV);
	GX_SetTevColorOp(GX_TEVSTAGE11, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	GX_SetTevKAlphaSel(GX_TEVSTAGE11, GX_TEV_KASEL_1);
	GX_SetTevAlphaIn(GX_TEVSTAGE11, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
	GX_SetTevAlphaOp(GX_TEVSTAGE11, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	
	//Setup vertex description/format
	GX_ClearVtxDesc();
	
	GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_TEX1, GX_INDEX8);
	
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
	
	GX_SetArray(GX_VA_POS, square, sizeof(f32) * 2);
	GX_SetArray(GX_VA_CLR0, colors, sizeof(GXColor));
	GX_SetArray(GX_VA_TEX0, Ytexcoords, sizeof(f32) * 2);
	GX_SetArray(GX_VA_TEX1, UVtexcoords, sizeof(f32) * 2);
	
	GX_BeginDispList(dlist, 32);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position1x8(0); GX_Color1x8(0); GX_TexCoord1x8(0); GX_TexCoord1x8(0);
		GX_Position1x8(1); GX_Color1x8(0); GX_TexCoord1x8(1); GX_TexCoord1x8(1);
		GX_Position1x8(2); GX_Color1x8(0); GX_TexCoord1x8(2); GX_TexCoord1x8(2);
		GX_Position1x8(3); GX_Color1x8(0); GX_TexCoord1x8(3); GX_TexCoord1x8(3);
	GX_End();
	GX_EndDispList();
}

//nunchuk control
extern float m_screenleft_shift, m_screenright_shift;
extern float m_screentop_shift, m_screenbottom_shift;

static f32 mysquare[8] ATTRIBUTE_ALIGN(32);

void mpgxUpdateSquare()
{
	memcpy(mysquare, square, sizeof(square));
	
	mysquare[0] -= m_screenleft_shift * 100.0;
	mysquare[6] -= m_screenleft_shift * 100.0;
	mysquare[2] -= m_screenright_shift * 100.0;
	mysquare[4] -= m_screenright_shift * 100.0;
	mysquare[1] -= m_screentop_shift * 100.0;
	mysquare[3] -= m_screentop_shift * 100.0;
	mysquare[5] -= m_screenbottom_shift * 100.0;
	mysquare[7] -= m_screenbottom_shift * 100.0;
	
	DCBlockFlush(mysquare);
	GX_SetArray(GX_VA_POS, mysquare, sizeof(f32) * 2);
	GX_InvVtxCache();
}

void mpgxSetSquare(f32 haspect, f32 vaspect)
{
	/*** Set new aspect ***/
	square[0] = square[6] = -haspect;
	square[2] = square[4] = haspect;
	square[1] = square[3] = vaspect;
	square[5] = square[7] = -vaspect;
	
	mpgxUpdateSquare();
}

void mpgxConfigYUVp(u32 luma_width, u32 luma_height, u32 chroma_width, u32 chroma_height)
{
	Ywidth = MIN((luma_width + 7) & ~7, 1024);
	Yheight = MIN((luma_height + 3) & ~3, 1024);
	
	Ytexsize = Ywidth * Yheight;
	
	UVwidth = MIN((chroma_width + 7) & ~7, 1024);
	UVheight = MIN((chroma_height + 3) & ~3, 1024);
	
	UVtexsize = UVwidth * UVheight;
	
#ifdef HW_RVL
	u32 heapsize = Ytexsize + (UVtexsize * 2);
	
	if (gx_area == NULL) gx_area = mem2_area_alloc(heapsize);
	else mem2_area_realloc(gx_area, heapsize);
	
	Ytexture = MEM_K0_TO_K1(__lwp_heap_allocate(&gx_area->heap, Ytexsize));
	Utexture = MEM_K0_TO_K1(__lwp_heap_allocate(&gx_area->heap, UVtexsize));
	Vtexture = MEM_K0_TO_K1(__lwp_heap_allocate(&gx_area->heap, UVtexsize));
#else
	if (Vtexture) free(MEM_K1_TO_K0(Vtexture));
	if (Utexture) free(MEM_K1_TO_K0(Utexture));
	if (Ytexture) free(MEM_K1_TO_K0(Ytexture));
	
	Ytexture = MEM_K0_TO_K1(memalign(32, Ytexsize));
	Utexture = MEM_K0_TO_K1(memalign(32, UVtexsize));
	Vtexture = MEM_K0_TO_K1(memalign(32, UVtexsize));
#endif
	
	f32 YtexcoordS = (double)luma_width / (double)Ywidth;
	f32 UVtexcoordS = (double)chroma_width / (double)UVwidth;
	
	Ytexcoords[2] = Ytexcoords[4] = YtexcoordS;
	UVtexcoords[2] = UVtexcoords[4] = UVtexcoordS;
	
	f32 YtexcoordT = (double)luma_height / (double)Yheight;
	f32 UVtexcoordT = (double)chroma_height / (double)UVheight;
	
	Ytexcoords[5] = Ytexcoords[7] = YtexcoordT;
	UVtexcoords[5] = UVtexcoords[7] = UVtexcoordT;
	
	DCBlockFlush(Ytexcoords);
	DCBlockFlush(UVtexcoords);
	
	//init YUV texture objects
	GX_InitTexObj(&YtexObj, Ytexture, Ywidth, Yheight, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_InitTexObjLOD(&YtexObj, GX_LINEAR, GX_LINEAR, 0.0, 0.0, 0.0, GX_TRUE, GX_TRUE, GX_ANISO_4);
	GX_InitTexObj(&UtexObj, Utexture, UVwidth, UVheight, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_InitTexObjLOD(&UtexObj, GX_LINEAR, GX_LINEAR, 0.0, 0.0, 0.0, GX_TRUE, GX_TRUE, GX_ANISO_4);
	GX_InitTexObj(&VtexObj, Vtexture, UVwidth, UVheight, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_InitTexObjLOD(&VtexObj, GX_LINEAR, GX_LINEAR, 0.0, 0.0, 0.0, GX_TRUE, GX_TRUE, GX_ANISO_4);
}

#define LUMA_COPY(type) \
{ \
	type *Ydst = (type *)Ytexture - 1; \
	 \
	type *Ysrc1 = (type *)buffer[0] - 1; \
	type *Ysrc2 = (type *)(buffer[0] + stride[0]) - 1; \
	type *Ysrc3 = (type *)(buffer[0] + (stride[0] * 2)) - 1; \
	type *Ysrc4 = (type *)(buffer[0] + (stride[0] * 3)) - 1; \
	 \
	int rows = Yheight / 4; \
	 \
	while (rows--) { \
		int tiles = Ywidth / 8; \
		 \
		while (tiles--) { \
			*++Ydst = *++Ysrc1; \
			*++Ydst = *++Ysrc2; \
			*++Ydst = *++Ysrc3; \
			*++Ydst = *++Ysrc4; \
		} \
		 \
		Ysrc1 = (type *)((u32)Ysrc1 + Yrowpitch); \
		Ysrc2 = (type *)((u32)Ysrc2 + Yrowpitch); \
		Ysrc3 = (type *)((u32)Ysrc3 + Yrowpitch); \
		Ysrc4 = (type *)((u32)Ysrc4 + Yrowpitch); \
	} \
}

#define CHROMA_COPY(type) \
{ \
	type *Udst = (type *)Utexture - 1; \
	type *Vdst = (type *)Vtexture - 1; \
	 \
	type *Usrc1 = (type *)buffer[1] - 1; \
	type *Usrc2 = (type *)(buffer[1] + stride[1]) - 1; \
	type *Usrc3 = (type *)(buffer[1] + (stride[1] * 2)) - 1; \
	type *Usrc4 = (type *)(buffer[1] + (stride[1] * 3)) - 1; \
	 \
	type *Vsrc1 = (type *)buffer[2] - 1; \
	type *Vsrc2 = (type *)(buffer[2] + stride[2]) - 1; \
	type *Vsrc3 = (type *)(buffer[2] + (stride[2] * 2)) - 1; \
	type *Vsrc4 = (type *)(buffer[2] + (stride[2] * 3)) - 1; \
	 \
	int rows = UVheight / 4; \
	 \
	while (rows--) { \
		int tiles = UVwidth / 8; \
		 \
		while (tiles--) { \
			*++Udst = *++Usrc1; \
			*++Udst = *++Usrc2; \
			*++Udst = *++Usrc3; \
			*++Udst = *++Usrc4; \
			 \
			*++Vdst = *++Vsrc1; \
			*++Vdst = *++Vsrc2; \
			*++Vdst = *++Vsrc3; \
			*++Vdst = *++Vsrc4; \
		} \
		 \
		Usrc1 = (type *)((u32)Usrc1 + UVrowpitch); \
		Usrc2 = (type *)((u32)Usrc2 + UVrowpitch); \
		Usrc3 = (type *)((u32)Usrc3 + UVrowpitch); \
		Usrc4 = (type *)((u32)Usrc4 + UVrowpitch); \
		 \
		Vsrc1 = (type *)((u32)Vsrc1 + UVrowpitch); \
		Vsrc2 = (type *)((u32)Vsrc2 + UVrowpitch); \
		Vsrc3 = (type *)((u32)Vsrc3 + UVrowpitch); \
		Vsrc4 = (type *)((u32)Vsrc4 + UVrowpitch); \
	} \
}

void mpgxCopyYUVp(u8 *buffer[3], int stride[3])
{
	s16 Yrowpitch = (stride[0] * 4) - Ywidth;
	
	if (stride[0] & 7)
		LUMA_COPY(u64)
	else LUMA_COPY(double)
	
	s16 UVrowpitch = (stride[1] * 4) - UVwidth;
	
	if (stride[1] & 7)
		CHROMA_COPY(u64)
	else CHROMA_COPY(double)
}

void mpgxBlitOSD(int x0, int y0, int w, int h, unsigned char *src, unsigned char *srca, int stride)
{
	s16 pitch = stride - w;
	
	u8 *Ycached = MEM_K1_TO_K0(Ytexture);
	DCInvalidateRange(Ycached, Ytexsize);
	
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (*srca) {
				int dxs = x + x0;
				int dys = y + y0;
				
				u8 *Ydst = Ycached + ((dys & (~3)) * Ywidth) + ((dxs & (~7)) << 2) + ((dys & 3) << 3) + (dxs & 7);
				*Ydst = (((*Ydst) * (*srca)) >> 8) + (*src);
			}
			
			src++; srca++;
		}
		
		src += pitch;
		srca += pitch;
	}
	
	DCFlushRange(Ycached, Ytexsize);
}

void mpgxWaitDrawDone()
{
	if (draw_pending)
		GX_WaitDrawDone();
}

void mpgxPushFrame()
{
	GX_InvalidateTexAll();
	
	if (vo_vsync && flip_pending)
		VIDEO_WaitVSync();
	
	GX_LoadTexObj(&YtexObj, GX_TEXMAP0);	// MAP0 <- Y
	GX_LoadTexObj(&UtexObj, GX_TEXMAP1);	// MAP1 <- U
	GX_LoadTexObj(&VtexObj, GX_TEXMAP2);	// MAP2 <- V
	
#ifndef HW_DOL
	u16 xfb_copypt = vmode->fbWidth >> 1;
	
	for (int dxs = 0; dxs < 2; dxs++) {
		u16 efb_offset = (xfb_copypt & ~15) * dxs;
		GX_SetScissorBoxOffset(efb_offset, 0);
		GX_CallDispList(dlist, 32);
		
		u32 xfb_offset = (xfb_copypt * VI_DISPLAY_PIX_SZ) * dxs;
		GX_CopyDisp(xfb[whichfb] + xfb_offset, GX_TRUE);
	}
#else
	GX_CallDispList(dlist, 32);
	GX_CopyDisp(xfb[whichfb], GX_TRUE);
#endif
	
	GX_SetDrawDone();
	draw_pending = true;
}
