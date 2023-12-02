/*
   getch2_gekko.c - MPlayer TermIO driver for Wii

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


#include <math.h>
#include <limits.h>

#include "config.h"
#include "keycodes.h"
#include "input/input.h"
#include "mp_fifo.h"
#include "plat_gekko.h"

#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>

#include <wiikeyboard/keyboard.h>
#include <wiikeyboard/wsksymdef.h>

#include <ogc/usbmouse.h>


#define PAD_DEADZONE 18
#define PAD_MODIFIER 16

#define IR_OFFSET 96

typedef struct {
	u16 pad;
	u32 wpad;
	int key;
} pad_map;


extern int screenwidth;
extern int screenheight;

int screen_width = 80;
int screen_height = 24;
char *erase_to_end_of_line = NULL;

float m_screenleft_shift = 0.0, m_screenright_shift = 0.0;
float m_screentop_shift = 0.0, m_screenbottom_shift = 0.0;
bool nunchuk_update = false;

static int getch2_status = false;

static const pad_map pad_maps[] = {
	{ PAD_BUTTON_A,		WPAD_BUTTON_A,		'a' },
	{ PAD_BUTTON_B,		WPAD_BUTTON_B,		'b' },
	{ PAD_BUTTON_X,		WPAD_BUTTON_1,		'x' },
	{ PAD_BUTTON_START,	WPAD_BUTTON_HOME,	'z' },
	{ PAD_TRIGGER_L,	WPAD_BUTTON_MINUS,	'l' },
	{ PAD_TRIGGER_R,	WPAD_BUTTON_PLUS,	'r' },
	{ PAD_BUTTON_LEFT,	WPAD_BUTTON_LEFT,	KEY_LEFT },
	{ PAD_BUTTON_RIGHT,	WPAD_BUTTON_RIGHT,	KEY_RIGHT },
	{ PAD_BUTTON_UP,	WPAD_BUTTON_UP,		KEY_UP },
	{ PAD_BUTTON_DOWN,	WPAD_BUTTON_DOWN,	KEY_DOWN }
};

static const pad_map pad_maps_mod[] = {
	{ PAD_BUTTON_A,		WPAD_BUTTON_A,		'A' },
	{ PAD_BUTTON_B,		WPAD_BUTTON_B,		'B' },
	{ PAD_BUTTON_X,		WPAD_BUTTON_1,		'X' },
	{ PAD_BUTTON_START,	WPAD_BUTTON_HOME,	'Z' },
	{ PAD_TRIGGER_L,	WPAD_BUTTON_MINUS,	'L' },
	{ PAD_TRIGGER_R,	WPAD_BUTTON_PLUS,	'R' },
	{ PAD_BUTTON_LEFT,	WPAD_BUTTON_LEFT,	KEY_KP4 },
	{ PAD_BUTTON_RIGHT,	WPAD_BUTTON_RIGHT,	KEY_KP6 },
	{ PAD_BUTTON_UP,	WPAD_BUTTON_UP,		KEY_KP8 },
	{ PAD_BUTTON_DOWN,	WPAD_BUTTON_DOWN,	KEY_KP2 }
};


void get_screen_size()
{
	int new_width, new_height;
	CON_GetMetrics(&new_width, &new_height);
	
	if (new_width > 0)
		screen_width = new_width;
	
	if (new_height > 0)
		screen_height = new_height;
}

void reset_screen_position()
{
	m_screenleft_shift = 0.0;
	m_screenright_shift = 0.0;
	m_screentop_shift = 0.0;
	m_screenbottom_shift = 0.0;
}

void getch2_enable()
{
	if (!getch2_status)
	{
		PAD_Init();
		
#ifdef HW_RVL
		WPAD_Init();
		WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth + (IR_OFFSET * 2), screenheight + (IR_OFFSET * 2));
		WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
		WPAD_SetIdleTimeout(60);
		
		KEYBOARD_Init(NULL);
		//MOUSE_Init();
#endif
	}
	
	getch2_status = true;
}

void getch2_disable()
{
#ifdef HW_RVL
	if (getch2_status)
	{
		WPAD_Disconnect(WPAD_CHAN_ALL);
		WPAD_Shutdown();
		
		KEYBOARD_Deinit();
		//MOUSE_Deinit();
	}
#endif
	
	getch2_status = false;
}

#ifdef HW_RVL
int getch2_internal(void)
{
	keyboard_event event;
	s32 result = KEYBOARD_GetEvent(&event);

	if (result && (event.type == KEYBOARD_PRESSED))
	{
		switch (event.symbol)
		{
			case KS_Home:
				return KEY_HOME;
			case KS_End:
				return KEY_END;
			case KS_Delete:
				return KEY_DEL;
			case KS_KP_Insert:
				return KEY_INS;
			case KS_BackSpace:
				return KEY_BS;
			case KS_Prior:
				return KEY_PGUP;
			case KS_Next:
				return KEY_PGDWN;
			case KS_Return:
				return KEY_ENTER;
			case KS_Escape:
				return KEY_ESC;
			case KS_Left:
				return KEY_LEFT;
			case KS_Up:
				return KEY_UP;
			case KS_Right:
				return KEY_RIGHT;
			case KS_Down:
				return KEY_DOWN;
		}

		if(KS_f20 >= event.symbol && event.symbol >= KS_f1)
			return KEY_F + 1 + event.symbol - KS_f1;

		return event.symbol;
	}

	return 0;
}
#endif

void getch2(void)
{
	if (reset_pressed || power_pressed)
	{
		mplayer_put_key(KEY_CLOSE_WIN);
		return;
	}
	
	static s64 lasttime = 0;
	s64 curtime = gettime();
	
	if (getch2_status && (ticks_to_millisecs(curtime - lasttime) > (TB_MSPERSEC / 60)))
	{
		float _m_screenleft_shift = m_screenleft_shift;
		float _m_screenright_shift = m_screenright_shift;
		float _m_screentop_shift = m_screentop_shift;
		float _m_screenbottom_shift = m_screenbottom_shift;
		
		PAD_ScanPads();
		
		for (int controller = 0; controller < PAD_CHANMAX; controller++)
		{
			u16 held = PAD_ButtonsHeld(controller);
			pad_map *mapping = pad_maps;
			
			if (held & PAD_BUTTON_Y)
				mapping = pad_maps_mod;
			
			u16 down = PAD_ButtonsDown(controller);
			int counts = sizeof(pad_maps) / sizeof(pad_map);
			
			for (int counter = 0; counter < counts; counter++)
			{
				if (down & mapping[counter].pad)
					mplayer_put_key(mapping[counter].key);
			}
			
			if (held & PAD_TRIGGER_Z)
				reset_screen_position();
			
			float joy_x, joy_y;
			
			joy_x = PAD_StickX(controller);
			joy_y = PAD_StickY(controller);
			
			if (fabs(joy_x) > PAD_DEADZONE)
			{
				m_screenleft_shift -= (joy_x / SCHAR_MAX) / PAD_MODIFIER;
				m_screenright_shift -= (joy_x / SCHAR_MAX) / PAD_MODIFIER;
			}
			
			if (fabs(joy_y) > PAD_DEADZONE)
			{
				m_screentop_shift -= (joy_y / SCHAR_MAX) / PAD_MODIFIER;
				m_screenbottom_shift -= (joy_y / SCHAR_MAX) / PAD_MODIFIER;
			}
			
			joy_x = PAD_SubStickX(controller);
			joy_y = PAD_SubStickY(controller);
			
			if (fabs(joy_x) > PAD_DEADZONE)
				m_screenright_shift -= (joy_x / SCHAR_MAX) / PAD_MODIFIER;
			
			if (fabs(joy_y) > PAD_DEADZONE)
				m_screenbottom_shift -= (joy_y / SCHAR_MAX) / PAD_MODIFIER;
		}
		
#ifdef HW_RVL
		u8 battery = 0;
		WPAD_ScanPads();
		
		for (int channel = 0; channel < WPAD_MAX_WIIMOTES; channel++)
		{
			u32 expansion = WPAD_EXP_NONE;
			s32 result = WPAD_Probe(channel, &expansion);
			
			if (result == WPAD_ERR_NONE)
			{
				WPADData *pointer = WPAD_Data(channel);
				WPADData data = *pointer;		// Wait, what?
				
				if (battery > 0)
					battery = (battery + data.battery_level) / 2;
				else
					battery = data.battery_level;
				
				WPAD_IR(channel, &data.ir);
				
				if (data.ir.valid)
				{
					//char command[40];			// Not final implementation.
					//snprintf(command, sizeof(command), "set_mouse_pos %i %i", (int)data.ir.x - IR_OFFSET, (int)data.ir.y - IR_OFFSET);
					//mp_input_queue_cmd(mp_input_parse_cmd(command));
				}
				
				u32 held = WPAD_ButtonsHeld(channel);
				pad_map *mapping = pad_maps;
				
				if (held & WPAD_BUTTON_2)
					mapping = pad_maps_mod;
				
				u32 down = WPAD_ButtonsDown(channel);
				int counts = sizeof(pad_maps) / sizeof(pad_map);
				
				for (int counter = 0; counter < counts; counter++)
				{
					if (down & mapping[counter].wpad)
						mplayer_put_key(mapping[counter].key);
				}
				
				WPAD_Expansion(channel, &data.exp);
				float joy_x, joy_y;
				
				switch (expansion)
				{
					case EXP_NUNCHUK:
						if (data.exp.nunchuk.btns_held & NUNCHUK_BUTTON_C)
							reset_screen_position();
						
						bool stretch = data.exp.nunchuk.btns_held & NUNCHUK_BUTTON_Z;
						
						joy_x = data.exp.nunchuk.js.pos.x - data.exp.nunchuk.js.center.x;
						joy_y = data.exp.nunchuk.js.pos.y - data.exp.nunchuk.js.center.y;
						
						if (fabs(joy_x) > PAD_DEADZONE)
						{
							if (!stretch)
								m_screenleft_shift -= (joy_x / data.exp.nunchuk.js.center.x) / PAD_MODIFIER;
							
							m_screenright_shift -= (joy_x / data.exp.nunchuk.js.center.x) / PAD_MODIFIER;
						}
						
						if (fabs(joy_y) > PAD_DEADZONE)
						{
							if (!stretch)
								m_screentop_shift -= (joy_y / data.exp.nunchuk.js.center.y) / PAD_MODIFIER;
							
							m_screenbottom_shift -= (joy_y / data.exp.nunchuk.js.center.y) / PAD_MODIFIER;
						}
						
						break;
					case EXP_CLASSIC:			// Incomplete.
						if (data.exp.classic.btns_held & CLASSIC_CTRL_BUTTON_B)
							reset_screen_position();
						
						joy_x = data.exp.classic.rjs.pos.x - data.exp.classic.rjs.center.x;
						joy_y = data.exp.classic.rjs.pos.y - data.exp.classic.rjs.center.y;
						
						if (fabs(joy_x) > PAD_DEADZONE)
							m_screenright_shift -= (joy_x / data.exp.classic.rjs.center.x) / PAD_MODIFIER;
						
						if (fabs(joy_y) > PAD_DEADZONE)
							m_screenbottom_shift -= (joy_y / data.exp.classic.rjs.center.y) / PAD_MODIFIER;
						
						joy_x = data.exp.classic.ljs.pos.x - data.exp.classic.ljs.center.x;
						joy_y = data.exp.classic.ljs.pos.y - data.exp.classic.ljs.center.y;
						
						if (fabs(joy_x) > PAD_DEADZONE)
							m_screenleft_shift -= (joy_x / data.exp.classic.ljs.center.x) / PAD_MODIFIER;
						
						if (fabs(joy_y) > PAD_DEADZONE)
							m_screentop_shift -= (joy_y / data.exp.classic.ljs.center.y) / PAD_MODIFIER;
						
						break;
				}
			}
		}
		
		double scale = (pow((double)battery, 2) / UCHAR_MAX) / UCHAR_MAX;
		WPAD_SetIdleTimeout((scale * 240) + 60);
#endif
		
		if ((m_screenleft_shift != _m_screenleft_shift) ||
			(m_screenright_shift != _m_screenright_shift) ||
			(m_screentop_shift != _m_screentop_shift) ||
			(m_screenbottom_shift != _m_screenbottom_shift))
		{
			nunchuk_update = true;
			mpgxUpdateSquare();
		}
		
#ifdef HW_RVL
		int kb_get = getch2_internal();
		
		if (kb_get > 0)
			mplayer_put_key(kb_get);
#endif
		
		lasttime = curtime;
	}
}

#if defined(HAVE_LANGINFO) && defined(CONFIG_ICONV)
#include <locale.h>
#include <langinfo.h>
#endif

#ifdef CONFIG_ICONV
char* get_term_charset(void)
{
#ifdef HAVE_LANGINFO
  	static const char *charset_aux = "ASCII";
    char *charset = NULL; 
    setlocale(LC_CTYPE, "");
    charset = nl_langinfo(CODESET);
    setlocale(LC_CTYPE, "C");
    if(charset==NULL || charset[0]=='\0')charset=charset_aux;
#else
	static const char *charset = "ASCII";
#endif
	return charset;
}
#endif

