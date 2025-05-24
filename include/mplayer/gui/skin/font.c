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

#include <gtk/gtk.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "cut.h"
#include "gui/interface.h"
#include "skin.h"

#include "libavutil/avstring.h"
#include "mp_msg.h"

static bmpFont *Fonts[MAX_FONTS];

static int fntAddNewFont(char *name)
{
    int id, i;

    for (id = 0; id < MAX_FONTS; id++)
        if (!Fonts[id])
            break;

    if (id == MAX_FONTS)
        return -2;

    Fonts[id] = calloc(1, sizeof(*Fonts[id]));

    if (!Fonts[id])
        return -1;

    av_strlcpy(Fonts[id]->name, name, MAX_FONT_NAME);

    for (i = 0; i < ASCII_CHRS + EXTRA_CHRS; i++) {
        Fonts[id]->Fnt[i].x  = -1;
        Fonts[id]->Fnt[i].y  = -1;
        Fonts[id]->Fnt[i].sx = -1;
        Fonts[id]->Fnt[i].sy = -1;
    }

    return id;
}

void fntFreeFont(void)
{
    int i;

    for (i = 0; i < MAX_FONTS; i++) {
        if (Fonts[i]) {
            bpFree(&Fonts[i]->Bitmap);
            gfree((void **)&Fonts[i]);
        }
    }
}

int fntRead(char *path, char *fname)
{
    FILE *f;
    unsigned char tmp[512];
    unsigned char *ptmp;
    unsigned char command[32];
    unsigned char param[256];
    int id, n, i;

    id = fntAddNewFont(fname);

    if (id < 0)
        return id;

    av_strlcpy(tmp, path, sizeof(tmp));
    av_strlcat(tmp, fname, sizeof(tmp));
    av_strlcat(tmp, ".fnt", sizeof(tmp));
    f = fopen(tmp, "rt");

    if (!f) {
        gfree((void **)&Fonts[id]);
        return -3;
    }

    while (fgets(tmp, sizeof(tmp), f)) {
        tmp[strcspn(tmp, "\n\r")] = 0; // remove any kind of newline, if any
        strswap(tmp, '\t', ' ');
        trim(tmp);
        ptmp = strchr(tmp, ';');

        if (ptmp && !(ptmp == tmp + 1 && tmp[0] == '"' && tmp[2] == '"'))
            *ptmp = 0;

        if (!*tmp)
            continue;

        n = (strncmp(tmp, "\"=", 2) == 0 ? 1 : 0);
        cutItem(tmp, command, '=', n);
        cutItem(tmp, param, '=', n + 1);

        if (command[0] == '"') {
            if (!command[1])
                command[0] = '=';
            else if (command[1] == '"')
                command[1] = 0;
            else
                cutItem(command, command, '"', 1);

            if (command[0] & 0x80) {
                for (i = 0; i < EXTRA_CHRS; i++) {
                    if (!Fonts[id]->nonASCIIidx[i][0]) {
                        strncpy(Fonts[id]->nonASCIIidx[i], command, UTF8LENGTH);
                        break;
                    }
                }

                if (i == EXTRA_CHRS)
                    continue;

                i += ASCII_CHRS;
            } else
                i = command[0];

            cutItem(param, tmp, ',', 0);
            Fonts[id]->Fnt[i].x = atoi(tmp);

            cutItem(param, tmp, ',', 1);
            Fonts[id]->Fnt[i].y = atoi(tmp);

            cutItem(param, tmp, ',', 2);
            Fonts[id]->Fnt[i].sx = atoi(tmp);

            cutItem(param, tmp, ',', 3);
            Fonts[id]->Fnt[i].sy = atoi(tmp);

            mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[font]  char: '%s' params: %d,%d %dx%d\n", command, Fonts[id]->Fnt[i].x, Fonts[id]->Fnt[i].y, Fonts[id]->Fnt[i].sx, Fonts[id]->Fnt[i].sy);
        } else if (!strcmp(command, "image")) {
            av_strlcpy(tmp, path, sizeof(tmp));
            av_strlcat(tmp, param, sizeof(tmp));

            mp_dbg(MSGT_GPLAYER, MSGL_DBG2, "[font] image file: %s\n", tmp);

            if (skinBPRead(tmp, &Fonts[id]->Bitmap) != 0) {
                bpFree(&Fonts[id]->Bitmap);
                gfree((void **)&Fonts[id]);
                fclose(f);
                return -4;
            }
        }
    }

    fclose(f);
    return 0;
}

int fntFindID(char *name)
{
    int i;

    for (i = 0; i < MAX_FONTS; i++)
        if (Fonts[i])
            if (!strcmp(name, Fonts[i]->name))
                return i;

    return -1;
}

// get Fnt index of character (utf8 or normal one) *str points to,
// then move pointer to next/previous character
static int fntGetCharIndex(int id, unsigned char **str, gboolean utf8, int direction)
{
    unsigned char *p, uchar[6] = "";   // glib implements 31-bit UTF-8
    int i, c = -1;

    if (**str & 0x80) {
        if (utf8) {
            p    = *str;
            *str = g_utf8_next_char(*str);
            strncpy(uchar, p, *str - p);

            if (direction < 0)
                *str = g_utf8_prev_char(p);
        } else {
            uchar[0] = **str;
            *str    += direction;
        }

        for (i = 0; (i < EXTRA_CHRS) && Fonts[id]->nonASCIIidx[i][0]; i++) {
            if (strncmp(Fonts[id]->nonASCIIidx[i], uchar, UTF8LENGTH) == 0)
                return i + ASCII_CHRS;

            if (!utf8 &&
                (Fonts[id]->nonASCIIidx[i][0] == (*uchar >> 6 | 0xc0) &&
                 Fonts[id]->nonASCIIidx[i][1] == ((*uchar & 0x3f) | 0x80) &&
                 Fonts[id]->nonASCIIidx[i][2] == 0))
                c = i + ASCII_CHRS;
        }
    } else {
        c = **str;

        if (utf8 && (direction < 0))
            *str = g_utf8_prev_char(*str);
        else
            *str += direction;
    }

    return c;
}

int fntTextWidth(int id, char *str)
{
    int size = 0, c;
    gboolean utf8;
    unsigned char *p;

    utf8 = g_utf8_validate(str, -1, NULL);
    p    = str;

    while (*p) {
        c = fntGetCharIndex(id, &p, utf8, 1);

        if (c == -1 || Fonts[id]->Fnt[c].sx == -1)
            c = ' ';

        if (Fonts[id]->Fnt[c].sx != -1)
            size += Fonts[id]->Fnt[c].sx;
    }

    return size;
}

static int fntTextHeight(int id, char *str)
{
    int max = 0, c, h;
    gboolean utf8;
    unsigned char *p;

    utf8 = g_utf8_validate(str, -1, NULL);
    p    = str;

    while (*p) {
        c = fntGetCharIndex(id, &p, utf8, 1);

        if (c == -1 || Fonts[id]->Fnt[c].sx == -1)
            c = ' ';

        h = Fonts[id]->Fnt[c].sy;

        if (h > max)
            max = h;
    }

    return max;
}

txSample *fntRender(wItem *item, int px, char *txt)
{
    unsigned char *u;
    unsigned int i;
    int c, dx, tw, th, fbw, iw, id, ofs;
    int x, y, fh, fw, fyc, yc;
    uint32_t *ibuf;
    uint32_t *obuf;
    gboolean utf8;

    id = item->fontid;
    tw = fntTextWidth(id, txt);

    if (!tw)
        return NULL;

    iw  = item->width;
    fbw = Fonts[id]->Bitmap.Width;
    th  = fntTextHeight(id, txt);

    if (item->height != th)
        bpFree(&item->Bitmap);

    if (!item->Bitmap.Image) {
        item->Bitmap.Height    = item->height = th;
        item->Bitmap.Width     = item->width = iw;
        item->Bitmap.ImageSize = item->height * iw * 4;

        if (!item->Bitmap.ImageSize)
            return NULL;

        item->Bitmap.BPP   = 32;
        item->Bitmap.Image = malloc(item->Bitmap.ImageSize);

        if (!item->Bitmap.Image)
            return NULL;
    }

    obuf = (uint32_t *)item->Bitmap.Image;
    ibuf = (uint32_t *)Fonts[id]->Bitmap.Image;

    for (i = 0; i < item->Bitmap.ImageSize / 4; i++)
        obuf[i] = 0x00ff00ff;

    if (tw <= iw) {
        switch (item->align) {
        default:
        case fntAlignLeft:
            dx = 0;
            break;

        case fntAlignCenter:
            dx = (iw - tw) / 2;
            break;

        case fntAlignRight:
            dx = iw - tw;
            break;
        }
    } else
        dx = px;

    ofs = dx;

    utf8 = g_utf8_validate(txt, -1, NULL);
    u    = txt;

    while (*u) {
        c = fntGetCharIndex(id, &u, utf8, 1);

        if (c != -1)
            fw = Fonts[id]->Fnt[c].sx;

        if (c == -1 || fw == -1) {
            c  = ' ';
            fw = Fonts[id]->Fnt[c].sx;
        }

        if (fw == -1)
            continue;

        fh  = Fonts[id]->Fnt[c].sy;
        fyc = Fonts[id]->Fnt[c].y * fbw + Fonts[id]->Fnt[c].x;
        yc  = dx;

        if (dx >= 0) {
            for (y = 0; y < fh; y++) {
                for (x = 0; x < fw; x++)
                    if (dx + x >= 0 && dx + x < iw)
                        obuf[yc + x] = ibuf[fyc + x];

                fyc += fbw;
                yc  += iw;
            }
        }

        dx += fw;
    }

    if (ofs > 0 && tw > item->width) {
        dx = ofs;
        u  = txt + strlen(txt);

        while (u > (unsigned char *)txt) {
            c = fntGetCharIndex(id, &u, utf8, -1);

            if (c != -1)
                fw = Fonts[id]->Fnt[c].sx;

            if (c == -1 || fw == -1) {
                c  = ' ';
                fw = Fonts[id]->Fnt[c].sx;
            }

            if (fw == -1)
                continue;

            fh  = Fonts[id]->Fnt[c].sy;
            fyc = Fonts[id]->Fnt[c].y * fbw + Fonts[id]->Fnt[c].x;

            dx -= fw;
            yc  = dx;

            if (dx >= 0) {
                for (y = 0; y < fh; y++) {
                    for (x = fw - 1; x >= 0; x--)
                        if (dx + x >= 0 && dx + x < iw)
                            obuf[yc + x] = ibuf[fyc + x];

                    fyc += fbw;
                    yc  += iw;
                }
            }
        }
    }

    return &item->Bitmap;
}
