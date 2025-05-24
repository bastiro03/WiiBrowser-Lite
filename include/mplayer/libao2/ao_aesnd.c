/*
 * AESND audio output driver
 *
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
#include <string.h>
#include <limits.h>
#include <math.h>
#include <ogcsys.h>
#include <aesndlib.h>

#include "config.h"
#include "audio_out.h"
#include "audio_out_internal.h"
#include "mp_msg.h"
#include "help_mp.h"
#include "libaf/af.h"
#include "libaf/af_format.h"
#include "libvo/fastmemcpy.h"
#include "osdep/ave-rvl.h"
#include "osdep/mem2.h"

#define BURST_SIZE 5760
#define NUM_BUFFERS 18

static const ao_info_t info = {
	"AESND audio output",
	"aesnd",
	"Extrems <metaradil@gmail.com>",
	""
};

LIBAO_EXTERN(aesnd)

static st_mem2_area *ao_area = NULL;
static AESNDPB *voice = NULL;

static void *buffers[NUM_BUFFERS];
static u8 fill_buffer, play_buffer;
static int queued;
static bool active;

static void aesnd_callback(AESNDPB *pb, u32 state)
{
	if (state == VOICE_STATE_STREAM) {
		if (queued > 0) {
			AESND_SetVoiceBuffer(pb, buffers[play_buffer], BURST_SIZE);
			play_buffer = (play_buffer + 1) % NUM_BUFFERS;
			queued -= BURST_SIZE;
		} else {
			AESND_SetVoiceStop(pb, true);
			active = false;
		}
	}
}

static int control(int cmd, void *arg)
{
#ifndef HW_RVL
	static u8 left = 0xFF, right = 0xFF;
#else
	u8 left, right;
#endif

	switch (cmd) {
		case AOCONTROL_QUERY_FORMAT:
			return CONTROL_TRUE;
		case AOCONTROL_GET_VOLUME:
		{
		#ifdef HW_RVL
			AVE_GetVolume(&left, &right);
		#endif
			((ao_control_vol_t *)arg)->left = left / 2.55;
			((ao_control_vol_t *)arg)->right = right / 2.55;
			return CONTROL_OK;
		}
		case AOCONTROL_SET_VOLUME:
		{
			left = clamp(ceil(((ao_control_vol_t *)arg)->left * 2.55), 0x00, 0xFF);
			right = clamp(ceil(((ao_control_vol_t *)arg)->right * 2.55), 0x00, 0xFF);
		#ifndef HW_RVL
			AESND_SetVoiceVolume(voice, left, right);
		#else
			AVE_SetVolume(left, right);
		#endif
			return CONTROL_OK;
		}
		default:
			return CONTROL_UNKNOWN;
	}
}

static int init(int rate, int channels, int format, int flags)
{
	int bits = format & AF_FORMAT_BITS_MASK;
	int bytes;
	
	u32 aesnd_format;
	bool stereo = channels > 1;
	
	if (bits > AF_FORMAT_8BIT) {
		ao_data.format = AF_FORMAT_S16_NE;
		aesnd_format = stereo ? VOICE_STEREO16 : VOICE_MONO16;
		bytes = sizeof(s16);
	} else {
		ao_data.format = AF_FORMAT_S8;
		aesnd_format = stereo ? VOICE_STEREO8 : VOICE_MONO8;
		bytes = sizeof(s8);
	}
	
	ao_data.channels = min(channels, 2);
	ao_data.samplerate = clamp(rate, 1, 144000);
	ao_data.bps = ao_data.channels * ao_data.samplerate * bytes;
	ao_data.buffersize = BURST_SIZE * NUM_BUFFERS;
	ao_data.outburst = BURST_SIZE;
	
#ifndef HW_RVL
	for (int i = 0; i < NUM_BUFFERS; i++)
		buffers[i] = memalign(32, BURST_SIZE);
#else
	ao_area = mem2_area_alloc(ao_data.buffersize);
	if (ao_area == NULL) return CONTROL_FALSE;
	
	for (int i = 0; i < NUM_BUFFERS; i++)
		buffers[i] = __lwp_heap_allocate(&ao_area->heap, BURST_SIZE);
#endif
	
	fill_buffer = 0;
	play_buffer = 0;
	queued = 0;
	active = false;
	
	AESND_Init();
	
	voice = AESND_AllocateVoice(aesnd_callback);
	AESND_SetVoiceFormat(voice, aesnd_format);
	AESND_SetVoiceFrequency(voice, ao_data.samplerate);
	AESND_SetVoiceVolume(voice, 0xFF, 0xFF);
	AESND_SetVoiceStream(voice, true);
	
	return CONTROL_TRUE;
}

static void reset(void)
{
	AESND_SetVoiceStop(voice, true);
	
	fill_buffer = 0;
	play_buffer = 0;
	queued = 0;
	active = false;
}

static void uninit(int immed)
{
	AESND_FreeVoice(voice);
#ifndef HW_RVL
	for (int i = 0; i < NUM_BUFFERS; i++)
		free(buffers[i]);
#else
	mem2_area_free(ao_area);
#endif
}

static void audio_pause(void)
{
	AESND_Pause(true);
	active = false;
}

static void audio_resume(void)
{
	active = true;
	AESND_Pause(false);
}

static int get_space(void)
{
	return (BURST_SIZE * (NUM_BUFFERS - 1)) - queued;
}

static int play(void *data, int len, int flags)
{
	int bytes;
	int copied = 0;
	int length = len;
	
	if (!(flags & AOPLAY_FINAL_CHUNK))
		length = (length / BURST_SIZE) * BURST_SIZE;
	
	while (((bytes = min(length, BURST_SIZE)) > 0) &&
		!(active && (fill_buffer == play_buffer))) {
		if (bytes < BURST_SIZE)
			memset(buffers[fill_buffer] + bytes, 0x00, BURST_SIZE - bytes);
		
		fast_memcpy(buffers[fill_buffer], data + copied, bytes);
		fill_buffer = (fill_buffer + 1) % NUM_BUFFERS;
		
		length -= bytes;
		copied += bytes;
		queued += bytes;
	}
	
	if (!active) {
		active = true;
		AESND_SetVoiceStop(voice, false);
		AESND_Pause(false);
	}
	
	return copied;
}

static float get_delay(void)
{
	return (float)queued / ao_data.bps;
}
