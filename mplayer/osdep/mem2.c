/*
 * This file is part of MPlayer CE.
 *
 * MPlayer CE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer CE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer CE; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ogc/machine/asm.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_heap.h>
#include <ogc/system.h>

#include "mp_msg.h"
#include "osdep/mem2.h"

#define ROUNDDOWN32(v)	(((u32)(v)-0x1f)&~0x1f)

static st_mem2_range stray;

st_mem2_area *mem2_area_alloc(u32 size)
{
	u32 overhead = (size / 32) * HEAP_BLOCK_USED_OVERHEAD;
	u32 total_size = size + overhead + HEAP_MIN_SIZE;
	
	st_mem2_range working;
	bool expand = true;
	
	if ((stray.floor != NULL) && (stray.ceiling != NULL)) {
		working.ceiling = stray.ceiling;
		working.floor = ROUNDDOWN32(working.ceiling - total_size);
		
		if (working.floor >= stray.floor) {
			if (working.floor == stray.floor) {
				stray.floor = NULL;
				stray.ceiling = NULL;
			} else
				stray.ceiling = working.floor;
			
			expand = false;
		}
	}
	
	if (expand) {
		u32 level;
		_CPU_ISR_Disable(level);
		
		working.ceiling = SYS_GetArena2Hi();
		working.floor = ROUNDDOWN32(working.ceiling - total_size);
		
		if (working.floor < SYS_GetArena2Lo()) {
			_CPU_ISR_Restore(level);
			mp_msg(MSGT_OSDEP, MSGL_FATAL, "mem2: Cannot allocate area of %d(+%d) bytes.\n",size,overhead);
			return NULL;
		} else
			SYS_SetArena2Hi(working.floor);
		
		_CPU_ISR_Restore(level);
	}
	
	st_mem2_area *area = calloc(1, sizeof(st_mem2_area));
	area->range.floor = working.floor;
	area->range.ceiling = working.ceiling;
	area->size = total_size;
	
	__lwp_heap_init(&area->heap, area->range.floor, area->size, 32);
	return area;
}

void mem2_area_realloc(st_mem2_area *area, u32 size)
{
	if (area == NULL)
		return;
	
	st_mem2_range usable = area->range;
	bool adjacent;
	
	if (adjacent = (usable.floor == stray.ceiling))
		usable.floor = stray.floor;
	else if (adjacent = (usable.ceiling == stray.floor))
		usable.ceiling = stray.ceiling;
	
	u32 overhead = (size / 32) * HEAP_BLOCK_USED_OVERHEAD;
	u32 total_size = size + overhead + HEAP_MIN_SIZE;
	u32 available = usable.ceiling - usable.floor;
	
	st_mem2_range working;
	
	u32 level;
	_CPU_ISR_Disable(level);
	
	if (total_size < available) {
		working.ceiling = usable.ceiling;
		working.floor = ROUNDDOWN32(usable.ceiling - total_size);
		
		if (usable.floor != SYS_GetArena2Hi()) {
			stray.ceiling = working.floor;
			stray.floor = usable.floor;
		} else {
			SYS_SetArena2Hi(working.floor);
			if (adjacent) {
				stray.floor = NULL;
				stray.ceiling = NULL;
			}
		}
	} else {
		if (usable.floor != SYS_GetArena2Hi()) {
			working.ceiling = SYS_GetArena2Hi();
			stray.floor = usable.floor;
			stray.ceiling = usable.ceiling;
		} else {
			working.ceiling = usable.ceiling;
			if (adjacent) {
				stray.floor = NULL;
				stray.ceiling = NULL;
			}
		}
		
		working.floor = ROUNDDOWN32(working.ceiling - total_size);
		
		if (working.floor < SYS_GetArena2Lo()) {
			_CPU_ISR_Restore(level);
			mp_msg(MSGT_OSDEP, MSGL_FATAL, "mem2: Cannot reallocate area of %d->%d(+%d) bytes (%p).\n",area->size,size,overhead,area->range.floor);
			return;
		} else	
			SYS_SetArena2Hi(working.floor);
	}
	
	_CPU_ISR_Restore(level);
	
	area->range.floor = working.floor;
	area->range.ceiling = working.ceiling;
	area->size = total_size;
	
	__lwp_heap_init(&area->heap, area->range.floor, area->size, 32);
}

void mem2_area_free(st_mem2_area *area)
{
	if (area == NULL)
		return;
	
	st_mem2_range usable = area->range;
	bool adjacent;
	
	if (adjacent = (usable.floor == stray.ceiling))
		usable.floor = stray.floor;
	else if (adjacent = (usable.ceiling == stray.floor))
		usable.ceiling = stray.ceiling;
	
	u32 level;
	_CPU_ISR_Disable(level);
	
	if (usable.floor == SYS_GetArena2Hi()) {
		SYS_SetArena2Hi(usable.ceiling);
		_CPU_ISR_Restore(level);
		
		if (adjacent) {
			stray.floor = NULL;
			stray.ceiling = NULL;
		}
	} else {
		_CPU_ISR_Restore(level);
		mp_msg(MSGT_OSDEP, MSGL_WARN, "mem2: Cannot reclaim area of %d bytes (%p).\n",area->size,area->range.floor);
		
		stray.floor = usable.floor;
		stray.ceiling = usable.ceiling;
	}
	
	free(area);
}
