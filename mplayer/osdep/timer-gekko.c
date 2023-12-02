/*
   MPlayer timer for Wii

   Copyright (C) 2008 dhewg

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301 USA.
*/

#include <gctypes.h>
#include <ogc/lwp_watchdog.h>
#include "timer.h"

const char timer_name[] = "gekko";

static u64 RelativeTime = 0;

u64 GetRelativeTime(void)
{
    u64 t,r;
    t = gettime();
    r = t - RelativeTime;
    RelativeTime = t;
    return ticks_to_microsecs(r);
}

void InitTimer(void)
{
	GetRelativeTime();
}
