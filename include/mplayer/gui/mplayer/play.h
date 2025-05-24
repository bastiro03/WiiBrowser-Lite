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

#ifndef MPLAYER_GUI_PLAY_H
#define MPLAYER_GUI_PLAY_H

extern int mplGotoTheNext;

void ChangeSkin(char *);
void mplAbsSeek(float);
void mplCurr(void);
void mplEnd(void);
void mplFullScreen(void);
void mplNext(void);
void mplPause(void);
void mplPlay(void);
void mplPrev(void);
void mplRelSeek(float);
void mplSetFileName(char *, char *, int);
void mplState(void);

#endif /* MPLAYER_GUI_PLAY_H */
