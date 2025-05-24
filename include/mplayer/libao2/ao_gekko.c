/*
   ao_gekko.c - MPlayer audio driver for Wii

   Copyright (C) 2008 dhewg
   Improved by Tantric, rodries and Extrems

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <ogcsys.h>

#include "config.h"
#include "libaf/af.h"
#include "libaf/af_format.h"
#include "audio_out.h"
#include "audio_out_internal.h"
#include "mp_msg.h"
#include "help_mp.h"

#include "osdep/plat_gekko.h"
#include "osdep/ave-rvl.h"
#include "osdep/wiilight.h"
#include "osdep/mem2.h"


#define BUFFER_SIZE 8192
#define BUFFER_COUNT 32

#define HW_CHANNELS 2

#define PAN_CENTER 0.7071067811865475		// sqrt(1/2)
#define PAN_SIDE 0.871779788708134			// sqrt(19/25)
#define PAN_SIDE_INV 0.4898979485566356		// sqrt(6/25)

#define PHASE_SHF 0.25						// "90 degrees"
#define PHASE_SHF_INV 0.75


static const ao_info_t info = {
	"gekko audio output",
	"gekko",
	"Team Twiizers",
	""
};

LIBAO_EXTERN(gekko)

static u8 *buffers[BUFFER_COUNT];
static u8 buffer_fill = 0;
static u8 buffer_play = 0;
static int buffered = 0;

static float request_mult = 1.0;
static int request_size = BUFFER_SIZE;

static u32 dma_lastpt = 0;
static bool playing = false;

static s32 snd_mode = CONF_SOUND_STEREO;
static s32 led_mode = CONF_LED_OFF;

static st_mem2_area *ao_area = NULL;
static ao_control_vol_t volume = { 0x8E, 0x8E };


static void switch_buffers()
{
	if (buffered > 0)
	{
		buffered -= BUFFER_SIZE;
		buffer_play = (buffer_play + 1) % BUFFER_COUNT;
		
		AUDIO_InitDMA((u32)buffers[buffer_play], BUFFER_SIZE);
	}
	else
	{
		AUDIO_StopDMA();
		
		dma_lastpt = 0;
		playing = false;
	}
}

static int control(int cmd, void *arg)
{
	switch (cmd)
	{
		case AOCONTROL_QUERY_FORMAT:
			return CONTROL_TRUE;
		case AOCONTROL_GET_VOLUME:
		case AOCONTROL_SET_VOLUME:
		{
			ao_control_vol_t *vol = (ao_control_vol_t *)arg;
			
			if (cmd == AOCONTROL_SET_VOLUME)
			{
				volume.left = vol->left * 2.55;
				volume.right = vol->right * 2.55;
				
				if (snd_mode == CONF_SOUND_MONO)
					volume.left = volume.right = (volume.left + volume.right) / 2;
				
				AVE_SetVolume(clamp(ceil(volume.right), 0x00, 0xFF), clamp(ceil(volume.left), 0x00, 0xFF));
			}
			
			vol->left = volume.left / 2.55;
			vol->right = volume.right / 2.55;
			
			return CONTROL_OK;
		}
		default:
			return CONTROL_UNKNOWN;
	}
}

static int init(int rate, int channels, int format, int flags)
{
	bool quality = rate > 32000;
	
	AUDIO_Init(NULL);
	AUDIO_SetDSPSampleRate(quality ? AI_SAMPLERATE_48KHZ : AI_SAMPLERATE_32KHZ);
	AUDIO_RegisterDMACallback(switch_buffers);
	
	ao_data.channels = clamp(channels, 2, 6);
	
	request_mult = (float)ao_data.channels / HW_CHANNELS;
	request_size = BUFFER_SIZE * request_mult;
	
	ao_data.samplerate = quality ? 48000 : 32000;
	ao_data.format = AF_FORMAT_S16_NE;
	ao_data.bps = ao_data.channels * ao_data.samplerate * sizeof(s16);
	ao_data.buffersize = request_size * BUFFER_COUNT;
	ao_data.outburst = request_size;
	
	ao_area = mem2_area_alloc(BUFFER_SIZE * BUFFER_COUNT);
	
	for (int counter = 0; counter < BUFFER_COUNT; counter++)
	{
		buffers[counter] = (u8 *)__lwp_heap_allocate(&ao_area->heap, BUFFER_SIZE);
		memset(buffers[counter], 0x00, BUFFER_SIZE);
		DCFlushRange(buffers[counter], BUFFER_SIZE);
	}
	
	buffer_fill = 0;
	buffer_play = 0;
	buffered = 0;
	
	dma_lastpt = 0;
	playing = false;
	
	snd_mode = CONF_GetSoundMode();
	led_mode = CONF_GetIdleLedMode();
	
	if (led_mode > 0)
	{
		WIILIGHT_Init();
		WIILIGHT_TurnOn();
	}
	
	return CONTROL_TRUE;
}

static void reset(void)
{
	AUDIO_StopDMA();
	
	for (int counter = 0; counter < BUFFER_COUNT; counter++)
	{
		memset(buffers[counter], 0x00, BUFFER_SIZE);
		DCFlushRange(buffers[counter], BUFFER_SIZE);
	}
	
	buffer_fill = 0;
	buffer_play = 0;
	buffered = 0;
	
	dma_lastpt = 0;
	playing = false;
}

static void uninit(int immed)
{
	AUDIO_StopDMA();
	AUDIO_RegisterDMACallback(NULL);
	mem2_area_free(ao_area);
	
	if (led_mode > 0)
		WIILIGHT_TurnOff();
}

static void audio_pause(void)
{
	dma_lastpt = AUDIO_GetDMABytesLeft();
	
	AUDIO_StopDMA();
	playing = false;
}

static void audio_resume(void)
{
	playing = true;
	
	AUDIO_InitDMA((u32)buffers[buffer_play] + (BUFFER_SIZE - dma_lastpt), dma_lastpt);
	AUDIO_StartDMA();
}

static int get_space(void)
{
	return ((BUFFER_SIZE * (BUFFER_COUNT - 1)) - buffered) * request_mult;
}

static int play(void *data, int len, int flags)
{
	int processed = 0;
	int remaining = len;
	
	s16 *source = (s16 *)data;
	
	while ((remaining >= request_size) && (get_space() >= request_size))
	{
		int samples = BUFFER_SIZE / (sizeof(s16) * HW_CHANNELS);
		s16 *destination = (s16 *)buffers[buffer_fill];
		
		for (int counter = 0; counter < samples; counter++)
		{
			int prs = max((counter - 1), -(processed / (sizeof(s16) * ao_data.channels))) * ao_data.channels;
			int crs = counter * ao_data.channels;	// "I'm surrounded!"
			int nrs = min((counter + 1), (remaining / (sizeof(s16) * ao_data.channels))) * ao_data.channels;
			
			s32 left, right;
			
			if (ao_data.channels > 1)
			{
				left = source[crs];
				right = source[crs + 1];
			}
			else
			{
				left = right = source[crs];
			}
			
			switch (ao_data.channels)
			{
				case 6:
				case 5:
					// Left rear
					left += ((source[crs + 2] * PHASE_SHF_INV) + (source[nrs + 2] * PHASE_SHF)) * PAN_SIDE;
					right += ((source[crs + 2] * PHASE_SHF_INV) + (source[prs + 2] * PHASE_SHF)) * PAN_SIDE_INV;
					
					// Right rear
					left += ((source[crs + 3] * PHASE_SHF_INV) + (source[nrs + 3] * PHASE_SHF)) * PAN_SIDE_INV;
					right += ((source[crs + 3] * PHASE_SHF_INV) + (source[prs + 3] * PHASE_SHF)) * PAN_SIDE;
					
					// Center front
					left += source[crs + 4] * PAN_CENTER;
					right += source[crs + 4] * PAN_CENTER;
					break;
				case 4:
					// Center rear
					left += ((source[crs + 2] * PHASE_SHF_INV) + (source[nrs + 2] * PHASE_SHF)) * PAN_CENTER;
					right += ((source[crs + 2] * PHASE_SHF_INV) + (source[prs + 2] * PHASE_SHF)) * PAN_CENTER;
					
					// Center front
					left += source[crs + 3] * PAN_CENTER;
					right += source[crs + 3] * PAN_CENTER;
					break;
			}
			
			if (snd_mode == CONF_SOUND_MONO)
				left = right = (left + right) / 2;
			
			int cws = counter * HW_CHANNELS;
			
			destination[cws] = clamp(right, SHRT_MIN, SHRT_MAX);
			destination[cws + 1] = clamp(left, SHRT_MIN, SHRT_MAX);
		}
		
		DCFlushRange(buffers[buffer_fill], BUFFER_SIZE);
		buffer_fill = (buffer_fill + 1) % BUFFER_COUNT;
		
		processed += request_size;
		remaining -= request_size;
		
		source += request_size / sizeof(s16);
		buffered += BUFFER_SIZE;
	}
	
	if (!playing && (buffered > request_size))
	{
		playing = true;
		switch_buffers();
		AUDIO_StartDMA();
	}
	
	return processed;
}

static float get_delay(void)
{
	if (playing)
	{
		if (led_mode > 0)
		{
			s16 *source = (s16 *)buffers[buffer_play];
			
			static u32 last = 0;
			u32 current = (BUFFER_SIZE - AUDIO_GetDMABytesLeft()) / sizeof(s16);
			
			if (last > current)
				last = 0;
			
			static double reference = 0.0;
			double average = 0.0;
			
			for (int counter = last; counter < current; counter++)
			{
				float value = (float)source[counter] / SHRT_MAX;
				
				if (!counter)
					average = value;
				else
					average = (average + value) / 2;
			}
			
			double level = (fabs((average + reference) / 2) * (((volume.left + volume.right) / 2) / 0xFF)) * ((float)led_mode / 2);
			WIILIGHT_SetLevel(clamp(level * UCHAR_MAX, 0x00, 0xFF));
			
			last = current;
			reference = average;
		}
		
		dma_lastpt = AUDIO_GetDMABytesLeft();
	}
	
	return ((float)(buffered + dma_lastpt) * request_mult) / ao_data.bps;
}
