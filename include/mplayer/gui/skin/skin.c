/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <string.h>

#include "skin.h"
#include "cut.h"
#include "font.h"
#include "gui/app.h"
#include "gui/mplayer/widgets.h"

#include "config.h"
#include "help_mp.h"
#include "libavutil/avstring.h"
#include "mp_msg.h"

typedef struct {
    const char *name;
    int (*func)(char *in);
} _item;

static listItems *defList;

static int linenumber;
static unsigned char path[512];
static unsigned char window_name[32];

static wItem *currSection;
static int *currSubItem;
static wItem *currSubItems;

static void ERRORMESSAGE(const char *format, ...)
{
    char p[512];
    char tmp[512];
    va_list ap;

    va_start(ap, format);
    vsnprintf(p, sizeof(p), format, ap);
    va_end(ap);

    mp_msg(MSGT_GPLAYER, MSGL_ERR, MSGTR_SKIN_ERRORMESSAGE, linenumber, p);

    if (mp_msg_test(MSGT_GPLAYER, MSGL_ERR)) {
        snprintf(tmp, sizeof(tmp), MSGTR_SKIN_ERRORMESSAGE, linenumber, p);
        gtkMessageBox(GTK_MB_FATAL, tmp);
    }
}

#define CHECKDEFLIST(str) \
    { \
        if (defList == NULL) \
        { \
            ERRORMESSAGE(MSGTR_SKIN_ERROR_SECTION, str); \
            return 1; \
        } \
    }

#define CHECKWINLIST(str) \
    { \
        if (!window_name[0]) \
        { \
            ERRORMESSAGE(MSGTR_SKIN_ERROR_WINDOW, str); \
            return 1; \
        } \
    }

#define CHECK(name) \
    { \
        if (!strcmp(window_name, name)) \
        { \
            ERRORMESSAGE(MSGTR_SKIN_ERROR_IN_WINDOW, name); \
            return 1; \
        } \
    }

static char *strlower(char *in)
{
    char *p = in;

    while (*p) {
        if (*p >= 'A' && *p <= 'Z')
            *p += 'a' - 'A';

        p++;
    }

    return in;
}

int skinBPRead(char *fname, txSample *bf)
{
    int i = bpRead(fname, bf);

    switch (i) {
    case -1:
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_16bit, fname);
        break;

    case -2:
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_FileNotFound, fname);
        break;

    case -3:
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_BMPReadError, fname);
        break;

    case -4:
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_TGAReadError, fname);
        break;

    case -5:
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_PNGReadError, fname);
        break;

    case -6:
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_RLENotSupported, fname);
        break;

    case -7:
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_UnknownFileType, fname);
        break;

    case -8:
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_ConversionError, fname);
        break;
    }

    return i;
}

// section=movieplayer
static int cmd_section(char *in)
{
    strlower(in);
    defList = NULL;

    if (!strcmp(in, "movieplayer"))
        defList = &appMPlayer;

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] sectionname: %s\n", in);

    return 0;
}

// end
static int cmd_end(char *in)
{
    (void)in;

    if (strlen(window_name)) {
        window_name[0] = 0;
        currSection    = NULL;
        currSubItem    = NULL;
        currSubItems   = NULL;
    } else
        defList = NULL;

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] end section\n");

    return 0;
}

// window=main|sub|playbar|menu
static int cmd_window(char *in)
{
    CHECKDEFLIST("window");

    av_strlcpy(window_name, strlower(in), sizeof(window_name));

    if (!strncmp(in, "main", 4)) {
        currSection  = &appMPlayer.main;
        currSubItem  = &appMPlayer.IndexOfMainItems;
        currSubItems = appMPlayer.mainItems;
    } else if (!strncmp(in, "sub", 3))
        currSection = &appMPlayer.sub;
    else if (!strncmp(in, "playbar", 7)) {
        currSection  = &appMPlayer.bar;
        currSubItem  = &appMPlayer.IndexOfBarItems;
        currSubItems = appMPlayer.barItems;
    } else if (!strncmp(in, "menu", 4)) {
        currSection  = &appMPlayer.menuBase;
        currSubItem  = &appMPlayer.IndexOfMenuItems;
        currSubItems = appMPlayer.menuItems;
    } else
        ERRORMESSAGE(MSGTR_UNKNOWNWINDOWTYPE);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] window: %s\n", window_name);

    return 0;
}

// base=image,x,y[,width,height]
static int cmd_base(char *in)
{
    unsigned char fname[512];
    unsigned char tmp[512];
    int x, y;
    int sx = 0, sy = 0;

    CHECKDEFLIST("base");
    CHECKWINLIST("base");

    cutItem(in, fname, ',', 0);
    x  = cutItemToInt(in, ',', 1);
    y  = cutItemToInt(in, ',', 2);
    sx = cutItemToInt(in, ',', 3);
    sy = cutItemToInt(in, ',', 4);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] base: %s x: %d y: %d ( %dx%d )\n", fname, x, y, sx, sy);

    if (!strcmp(window_name, "main")) {
        defList->main.x    = x;
        defList->main.y    = y;
        defList->main.type = itBase;

        av_strlcpy(tmp, path, sizeof(tmp));
        av_strlcat(tmp, fname, sizeof(tmp));

        if (skinBPRead(tmp, &defList->main.Bitmap) != 0)
            return 1;

        defList->main.width  = defList->main.Bitmap.Width;
        defList->main.height = defList->main.Bitmap.Height;

#ifdef CONFIG_XSHAPE
        Convert32to1(&defList->main.Bitmap, &defList->main.Mask, 0x00ff00ff);
        mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  mask: %lux%lu\n", defList->main.Mask.Width, defList->main.Mask.Height);
#else
        defList->main.Mask.Image = NULL;
#endif

        mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  width: %d height: %d\n", defList->main.width, defList->main.height);
    }

    if (!strcmp(window_name, "sub")) {
        defList->sub.type = itBase;

        av_strlcpy(tmp, path, sizeof(tmp));
        av_strlcat(tmp, fname, sizeof(tmp));

        if (skinBPRead(tmp, &defList->sub.Bitmap) != 0)
            return 1;

        defList->sub.x      = x;
        defList->sub.y      = y;
        defList->sub.width  = defList->sub.Bitmap.Width;
        defList->sub.height = defList->sub.Bitmap.Height;

        if (sx && sy) {
            defList->sub.width  = sx;
            defList->sub.height = sy;
        }

        mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  %d,%d %dx%d\n", defList->sub.x, defList->sub.y, defList->sub.width, defList->sub.height);
    }

    if (!strcmp(window_name, "menu")) {
        defList->menuIsPresent = 1;
        defList->menuBase.type = itBase;

        av_strlcpy(tmp, path, sizeof(tmp));
        av_strlcat(tmp, fname, sizeof(tmp));

        if (skinBPRead(tmp, &defList->menuBase.Bitmap) != 0)
            return 1;

        defList->menuBase.width  = defList->menuBase.Bitmap.Width;
        defList->menuBase.height = defList->menuBase.Bitmap.Height;

#ifdef CONFIG_XSHAPE
        Convert32to1(&defList->menuBase.Bitmap, &defList->menuBase.Mask, 0x00ff00ff);
        mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  mask: %lux%lu\n", defList->menuBase.Mask.Width, defList->menuBase.Mask.Height);
#else
        defList->menuBase.Mask.Image = NULL;
#endif

        mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  width: %d height: %d\n", defList->menuBase.width, defList->menuBase.height);
    }

    if (!strcmp(window_name, "playbar")) {
        defList->barIsPresent = 1;
        defList->bar.x    = x;
        defList->bar.y    = y;
        defList->bar.type = itBase;

        av_strlcpy(tmp, path, sizeof(tmp));
        av_strlcat(tmp, fname, sizeof(tmp));

        if (skinBPRead(tmp, &defList->bar.Bitmap) != 0)
            return 1;

        defList->bar.width  = defList->bar.Bitmap.Width;
        defList->bar.height = defList->bar.Bitmap.Height;

#ifdef CONFIG_XSHAPE
        Convert32to1(&defList->bar.Bitmap, &defList->bar.Mask, 0x00ff00ff);
        mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  mask: %lux%lu\n", defList->bar.Mask.Width, defList->bar.Mask.Height);
#else
        defList->bar.Mask.Image = NULL;
#endif

        mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  width: %d height: %d\n", defList->bar.width, defList->bar.height);
    }

    return 0;
}

// background=R,G,B
static int cmd_background(char *in)
{
    CHECKDEFLIST("background");
    CHECKWINLIST("background");

    CHECK("menu");
    CHECK("main");

    currSection->R = cutItemToInt(in, ',', 0);
    currSection->G = cutItemToInt(in, ',', 1);
    currSection->B = cutItemToInt(in, ',', 2);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin]  background color is #%x%x%x.\n", currSection->R, currSection->G, currSection->B);

    return 0;
}

// button=image,x,y,width,height,message
static int cmd_button(char *in)
{
    unsigned char fname[512];
    unsigned char tmp[512];
    int x, y, sx, sy;
    char msg[32];

    CHECKDEFLIST("button");
    CHECKWINLIST("button");

    CHECK("sub");
    CHECK("menu");

    cutItem(in, fname, ',', 0);
    x  = cutItemToInt(in, ',', 1);
    y  = cutItemToInt(in, ',', 2);
    sx = cutItemToInt(in, ',', 3);
    sy = cutItemToInt(in, ',', 4);
    cutItem(in, msg, ',', 5);

    (*currSubItem)++;
    currSubItems[*currSubItem].type   = itButton;
    currSubItems[*currSubItem].x      = x;
    currSubItems[*currSubItem].y      = y;
    currSubItems[*currSubItem].width  = sx;
    currSubItems[*currSubItem].height = sy;

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] button: fname: %s\n", fname);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  x: %d y: %d sx: %d sy: %d\n", x, y, sx, sy);

    if ((currSubItems[*currSubItem].message = appFindMessage(msg)) == -1) {
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_UnknownMessage, msg);
        return 0;
    }

    currSubItems[*currSubItem].pressed = btnReleased;

    if (currSubItems[*currSubItem].message == evPauseSwitchToPlay)
        currSubItems[*currSubItem].pressed = btnDisabled;

    currSubItems[*currSubItem].tmp = 1;

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  message: %d\n", currSubItems[*currSubItem].message);

    currSubItems[*currSubItem].Bitmap.Image = NULL;

    if (strcmp(fname, "NULL") != 0) {
        av_strlcpy(tmp, path, sizeof(tmp));
        av_strlcat(tmp, fname, sizeof(tmp));

        if (skinBPRead(tmp, &currSubItems[*currSubItem].Bitmap) != 0)
            return 1;
    }

    return 0;
}

// selected=image
static int cmd_selected(char *in)
{
    unsigned char fname[512];
    unsigned char tmp[512];

    CHECKDEFLIST("selected");
    CHECKWINLIST("selected");

    CHECK("main");
    CHECK("sub");
    CHECK("playbar");

    cutItem(in, fname, ',', 0);

    defList->menuSelected.type = itBase;

    av_strlcpy(tmp, path, sizeof(tmp));
    av_strlcat(tmp, fname, sizeof(tmp));

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] selected: %s\n", fname);

    if (skinBPRead(tmp, &defList->menuSelected.Bitmap) != 0)
        return 1;

    defList->menuSelected.width  = defList->menuSelected.Bitmap.Width;
    defList->menuSelected.height = defList->menuSelected.Bitmap.Height;

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  width: %d height: %d\n", defList->menuSelected.width, defList->menuSelected.height);

    return 0;
}

// menu=x,y,width,height,message
static int cmd_menu(char *in)
{
    int x, y, sx, sy, message;
    unsigned char tmp[64];

    CHECKDEFLIST("menu");
    CHECKWINLIST("menu");

    CHECK("main");
    CHECK("sub");
    CHECK("playbar");

    x  = cutItemToInt(in, ',', 0);
    y  = cutItemToInt(in, ',', 1);
    sx = cutItemToInt(in, ',', 2);
    sy = cutItemToInt(in, ',', 3);
    cutItem(in, tmp, ',', 4);

    message = appFindMessage(tmp);

    defList->IndexOfMenuItems++;
    defList->menuItems[defList->IndexOfMenuItems].x      = x;
    defList->menuItems[defList->IndexOfMenuItems].y      = y;
    defList->menuItems[defList->IndexOfMenuItems].width  = sx;
    defList->menuItems[defList->IndexOfMenuItems].height = sy;

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] menuitem: %d\n", defList->IndexOfMenuItems);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  x: %d y: %d sx: %d sy: %d\n", x, y, sx, sy);

    if ((defList->menuItems[defList->IndexOfMenuItems].message = message) == -1)
        ERRORMESSAGE(MSGTR_SKIN_BITMAP_UnknownMessage, tmp);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  message: %d\n", defList->mainItems[defList->IndexOfMainItems].message);

    defList->menuItems[defList->IndexOfMenuItems].Bitmap.Image = NULL;
    return 0;
}

// hpotmeter=button,bwidth,bheight,phases,numphases,default,x,y,width,height,message
static int cmd_hpotmeter(char *in)
{
    int x, y, pwidth, pheight, ph, sx, sy, message, d;
    unsigned char tmp[512];
    unsigned char pfname[512];
    unsigned char phfname[512];
    wItem *item;

    CHECKDEFLIST("hpotmeter");
    CHECKWINLIST("hpotmeter");

    CHECK("sub");
    CHECK("menu");

    cutItem(in, pfname, ',', 0);
    pwidth  = cutItemToInt(in, ',', 1);
    pheight = cutItemToInt(in, ',', 2);
    cutItem(in, phfname, ',', 3);
    ph = cutItemToInt(in, ',', 4);
    d  = cutItemToInt(in, ',', 5);
    x  = cutItemToInt(in, ',', 6);
    y  = cutItemToInt(in, ',', 7);
    sx = cutItemToInt(in, ',', 8);
    sy = cutItemToInt(in, ',', 9);
    cutItem(in, tmp, ',', 10);

    message = appFindMessage(tmp);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] h/v potmeter: pointer filename: '%s'\n", pfname);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  pointer size is %dx%d\n", pwidth, pheight);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  phasebitmaps filename: '%s'\n", phfname);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]   position: %d,%d %dx%d\n", x, y, sx, sy);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]   default value: %d\n", d);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  message: %d\n", message);

    (*currSubItem)++;
    item               = &currSubItems[*currSubItem];
    item->type         = itHPotmeter;
    item->x            = x;
    item->y            = y;
    item->width        = sx;
    item->height       = sy;
    item->numphases    = ph;
    item->pwidth       = pwidth;
    item->pheight      = pheight;
    item->message      = message;
    item->value        = (float)d;
    item->pressed      = btnReleased;
    item->Bitmap.Image = NULL;

    if (strcmp(phfname, "NULL") != 0) {
        av_strlcpy(tmp, path, sizeof(tmp));
        av_strlcat(tmp, phfname, sizeof(tmp));

        if (skinBPRead(tmp, &item->Bitmap) != 0)
            return 1;
    }

    item->Mask.Image = NULL;

    if (strcmp(pfname, "NULL") != 0) {
        av_strlcpy(tmp, path, sizeof(tmp));
        av_strlcat(tmp, pfname, sizeof(tmp));

        if (skinBPRead(tmp, &item->Mask) != 0)
            return 1;
    }

    return 0;
}

// vpotmeter=button,bwidth,bheight,phases,numphases,default,x,y,width,height,message
static int cmd_vpotmeter(char *in)
{
    int r = cmd_hpotmeter(in);
    wItem *item;

    item       = &currSubItems[*currSubItem];
    item->type = itVPotmeter;
    return r;
}

// potmeter=phases,numphases,default,x,y,width,height,message
static int cmd_potmeter(char *in)
{
    int x, y, ph, sx, sy, message, d;
    unsigned char tmp[512];
    unsigned char phfname[512];
    wItem *item;

    CHECKDEFLIST("potmeter");
    CHECKWINLIST("potmeter");

    CHECK("sub");
    CHECK("menu");

    cutItem(in, phfname, ',', 0);
    ph = cutItemToInt(in, ',', 1);
    d  = cutItemToInt(in, ',', 2);
    x  = cutItemToInt(in, ',', 3);
    y  = cutItemToInt(in, ',', 4);
    sx = cutItemToInt(in, ',', 5);
    sy = cutItemToInt(in, ',', 6);
    cutItem(in, tmp, ',', 7);

    message = appFindMessage(tmp);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] potmeter: phases filename: '%s'\n", phfname);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  position: %d,%d %dx%d\n", x, y, sx, sy);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  numphases: %d\n", ph);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  default value: %d\n", d);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  message: %d\n", message);

    (*currSubItem)++;
    item               = &currSubItems[*currSubItem];
    item->type         = itPotmeter;
    item->x            = x;
    item->y            = y;
    item->width        = sx;
    item->height       = sy;
    item->numphases    = ph;
    item->message      = message;
    item->value        = (float)d;
    item->Bitmap.Image = NULL;

    if (strcmp(phfname, "NULL") != 0) {
        av_strlcpy(tmp, path, sizeof(tmp));
        av_strlcat(tmp, phfname, sizeof(tmp));

        if (skinBPRead(tmp, &item->Bitmap) != 0)
            return 1;
    }

    return 0;
}

// font=fontfile
static int cmd_font(char *in)
{
    char name[512];
    wItem *item;

    CHECKDEFLIST("font");
    CHECKWINLIST("font");

    CHECK("sub");
    CHECK("menu");

    cutItem(in, name, ',', 0);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] font\n");
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  name: %s\n", name);

    (*currSubItem)++;
    item         = &currSubItems[*currSubItem];
    item->type   = itFont;
    item->fontid = fntRead(path, name);

    switch (item->fontid) {
    case -1:
        ERRORMESSAGE(MSGTR_SKIN_FONT_NotEnoughtMemory);
        return 1;

    case -2:
        ERRORMESSAGE(MSGTR_SKIN_FONT_TooManyFontsDeclared);
        return 1;

    case -3:
        ERRORMESSAGE(MSGTR_SKIN_FONT_FontFileNotFound);
        return 1;

    case -4:
        ERRORMESSAGE(MSGTR_SKIN_FONT_FontImageNotFound);
        return 1;
    }

    return 0;
}

// slabel=x,y,fontfile,"text"
static int cmd_slabel(char *in)
{
    char tmp[512];
    char sid[64];
    int x, y, id;
    wItem *item;

    CHECKDEFLIST("slabel");
    CHECKWINLIST("slabel");

    CHECK("sub");
    CHECK("menu");

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] slabel\n");

    x = cutItemToInt(in, ',', 0);
    y = cutItemToInt(in, ',', 1);
    cutItem(in, sid, ',', 2);

    id = fntFindID(sid);

    if (id < 0) {
        ERRORMESSAGE(MSGTR_SKIN_FONT_NonExistentFontID, sid);
        return 1;
    }

    cutItem(in, tmp, ',', 3);
    cutItem(tmp, tmp, '"', 1);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  pos: %d,%d\n", x, y);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  id: %s ( %d )\n", sid, id);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  str: '%s'\n", tmp);

    (*currSubItem)++;
    item         = &currSubItems[*currSubItem];
    item->type   = itSLabel;
    item->fontid = id;
    item->x      = x;
    item->y      = y;
    item->width  = -1;
    item->height = -1;

    if ((item->label = malloc(strlen(tmp) + 1)) == NULL) {
        ERRORMESSAGE(MSGTR_SKIN_FONT_NotEnoughtMemory);
        return 1;
    }

    strcpy(item->label, tmp);

    return 0;
}

// dlabel=x,y,width,align,fontfile,"text"
static int cmd_dlabel(char *in)
{
    char tmp[512];
    char sid[64];
    int x, y, sx, id, a;
    wItem *item;

    CHECKDEFLIST("dlabel");
    CHECKWINLIST("dlabel");

    CHECK("sub");
    CHECK("menu");

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] dlabel\n");

    x  = cutItemToInt(in, ',', 0);
    y  = cutItemToInt(in, ',', 1);
    sx = cutItemToInt(in, ',', 2);
    a  = cutItemToInt(in, ',', 3);
    cutItem(in, sid, ',', 4);

    id = fntFindID(sid);

    if (id < 0) {
        ERRORMESSAGE(MSGTR_SKIN_FONT_NonExistentFontID, sid);
        return 1;
    }

    cutItem(in, tmp, ',', 5);
    cutItem(tmp, tmp, '"', 1);

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  pos: %d,%d width: %d align: %d\n", x, y, sx, a);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  id: %s ( %d )\n", sid, id);
    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin]  str: '%s'\n", tmp);

    (*currSubItem)++;
    item         = &currSubItems[*currSubItem];
    item->type   = itDLabel;
    item->fontid = id;
    item->align  = a;
    item->x      = x;
    item->y      = y;
    item->width  = sx;
    item->height = -1;

    if ((item->label = malloc(strlen(tmp) + 1)) == NULL) {
        ERRORMESSAGE(MSGTR_SKIN_FONT_NotEnoughtMemory);
        return 1;
    }

    strcpy(item->label, tmp);

    return 0;
}

// decoration=enable|disable
static int cmd_decoration(char *in)
{
    char tmp[512];

    CHECKDEFLIST("decoration");
    CHECKWINLIST("decoration");

    CHECK("sub");
    CHECK("menu");
    CHECK("playbar");

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] window decoration is %s\n", in);

    strlower(in);
    cutItem(in, tmp, ',', 0);

    if (strcmp(tmp, "enable") != 0 && strcmp(tmp, "disable") != 0) {
        ERRORMESSAGE(MSGTR_SKIN_UnknownParameter, tmp);
        return 1;
    }

    if (strcmp(tmp, "enable") != 0)
        defList->mainDecoration = 0;
    else
        defList->mainDecoration = 1;

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "\n[skin] window decoration is %s\n", (defList->mainDecoration ? "enabled" : "disabled"));

    return 0;
}

static _item skinItem[] = {
    { "section",    cmd_section    },
    { "end",        cmd_end        },
    { "window",     cmd_window     },
    { "base",       cmd_base       },
    { "button",     cmd_button     },
    { "selected",   cmd_selected   },
    { "background", cmd_background },
    { "vpotmeter",  cmd_vpotmeter  },
    { "hpotmeter",  cmd_hpotmeter  },
    { "potmeter",   cmd_potmeter   },
    { "font",       cmd_font       },
    { "slabel",     cmd_slabel     },
    { "dlabel",     cmd_dlabel     },
    { "decoration", cmd_decoration },
    { "menu",       cmd_menu       }
};

static const int ITEMS = sizeof(skinItem) / sizeof(_item);

char *strswap(char *in, char what, char whereof)
{
    int i;

    if (strlen(in) == 0)
        return NULL;

    for (i = 0; i < (int)strlen(in); i++)
        if (in[i] == what)
            in[i] = whereof;

    return in;
}

char *trim(char *in)
{
    int c = 0, id = 0, i;

    if (strlen(in) == 0)
        return NULL;

    while (c != (int)strlen(in)) {
        if (in[c] == '"')
            id = !id;

        if ((in[c] == ' ') && (!id)) {
            for (i = 0; i < (int)strlen(in) - c; i++)
                in[c + i] = in[c + i + 1];
            continue;
        }

        c++;
    }

    return in;
}

static char *setname(char *item1, char *item2)
{
    static char fn[512];

    av_strlcpy(fn, item1, sizeof(fn));
    av_strlcat(fn, "/", sizeof(fn));
    av_strlcat(fn, item2, sizeof(fn));
    av_strlcpy(path, fn, sizeof(path));
    av_strlcat(path, "/", sizeof(path));
    av_strlcat(fn, "/skin", sizeof(fn));

    return fn;
}

int skinRead(char *dname)
{
    char *fn;
    FILE *skinFile;
    unsigned char tmp[256];
    unsigned char *ptmp;
    unsigned char command[32];
    unsigned char param[256];
    int i;

    fn = setname(skinDirInHome, dname);

    if ((skinFile = fopen(fn, "rt")) == NULL) {
        fn = setname(skinMPlayerDir, dname);

        if ((skinFile = fopen(fn, "rt")) == NULL) {
            mp_msg(MSGT_GPLAYER, MSGL_ERR, MSGTR_SKIN_SkinFileNotFound, fn);
            return -1;
        }
    }

    mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[skin] file: %s\n", fn);

    appFreeStruct();

    linenumber = 0;

    while (fgets(tmp, sizeof(tmp), skinFile)) {
        linenumber++;

        tmp[strcspn(tmp, "\n\r")] = 0; // remove any kind of newline, if any
        strswap(tmp, '\t', ' ');
        trim(tmp);
        ptmp = strchr(tmp, ';');

        if (ptmp)
            *ptmp = 0;

        if (!*tmp)
            continue;

        cutItem(tmp, command, '=', 0);
        cutItem(tmp, param, '=', 1);
        strlower(command);

        for (i = 0; i < ITEMS; i++)
            if (!strcmp(command, skinItem[i].name))
                if (skinItem[i].func(param) != 0)
                    return -2;
    }

    if (linenumber == 0) {
        mp_msg(MSGT_GPLAYER, MSGL_ERR, MSGTR_SKIN_SkinFileNotReadable, fn);
        return -1;
    }

    return 0;
}
