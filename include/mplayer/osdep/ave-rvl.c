/*
	libbroadway - A general purpose library to control the Wii.
	Low-level video support

Copyright (C) 2008, 2009	Hector Martin "marcan" <marcan@marcansoft.com>
Copyright (C) 2009		Haxx Enterprises <bushing@gmail.com>
Copyright (c) 2009		Sven Peter <svenpeter@gmail.com>
Copyright (C) 2009-2010		Alex Marshall <trap15@raidenii.net>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Some routines and initialization constants originally came from the
"GAMECUBE LOW LEVEL INFO" document and sourcecode released by Titanik
of Crazy Nation and the GC Linux project.
*/

#include <gctypes.h>
#include <ogc/machine/processor.h>

#define HW_REG_BASE		0xD800000

#define HW_GPIO1BOUT	(HW_REG_BASE + 0x0C0)
#define HW_GPIO1BDIR	(HW_REG_BASE + 0x0C4)
#define HW_GPIO1BIN		(HW_REG_BASE + 0x0C8)

#define SLAVE_AVE	0xE0

static u8 volume[2] = {0x8E, 0x8E};

/* Sets the direction of the GPIOs */
static inline void aveSetDirection(u32 dir)
{
	u32 val = (read32(HW_GPIO1BDIR) & ~0x8000) | 0x4000;
	if(dir) val |= 0x8000;
	write32(HW_GPIO1BDIR, val);
}

/* Sets the A/V Encoder I2C Clock */
static inline void aveSetClock(u32 scl)
{
	u32 val = read32(HW_GPIO1BOUT) & ~0x4000;
	if(scl) val |= 0x4000;
	write32(HW_GPIO1BOUT, val);
}

/* Sets the A/V Encoder I2C Data */
static inline void aveSetData(u32 sda)
{
	u32 val = read32(HW_GPIO1BOUT) & ~0x8000;
	if(sda) val |= 0x8000;
	write32(HW_GPIO1BOUT, val);
}

/* Gets the A/V Encoder I2C Data */
static inline u32 aveGetData(void)
{
	return (read32(HW_GPIO1BIN) & 0x8000) ? 1 : 0;
}

static inline u32 aveSendI2C(u8 data)
{
	int i;
	for(i = 0; i < 8; i++) {
		if(data & 0x80) aveSetData(1);
		else aveSetData(0);
		udelay(2);
		
		aveSetClock(1);
		udelay(2);
		
		aveSetClock(0);
		data <<= 1;
	}
	
	aveSetDirection(0);
	udelay(2);
	aveSetClock(1);
	udelay(2);
	
	if(aveGetData() != 0)
		return 0;
	
	aveSetData(0);
	aveSetDirection(1);
	aveSetClock(0);
	return 1;
}

static u32 __sendSlaveAddress(u8 addr)
{
	aveSetData(0);
	udelay(2);
	
	aveSetClock(0);
	
	if(!aveSendI2C(addr))
		return 0;
	
	return 1;
}
 
static u32 __VISendI2CData(u8 addr,void *val,u32 len)
{
	u8 c;
	u32 i;
	u32 ret;

	aveSetDirection(1);
	aveSetClock(1);

	aveSetData(1);
	udelay(4);

	ret = __sendSlaveAddress(addr);
	if(ret == 0) {
		return 0;
	}

	aveSetDirection(1);
	for(i = 0; i < len; i++) {
		c = ((u8*)val)[i];
		if(!aveSendI2C(c)) {
			return 0;
		}
	}

	aveSetDirection(1);
	aveSetData(0);
	udelay(2);
	aveSetData(1);

	return 1;
}

void AVE_GetVolume(u8 *left, u8 *right)
{
	*left = volume[0];
	*right = volume[1];
}

void AVE_SetVolume(u8 left, u8 right)
{
	u8 buffer[3] = {0x71, right, left};
	
	volume[0] = left;
	volume[1] = right;
	
	__VISendI2CData(SLAVE_AVE, buffer, 3);
	udelay(2);
}
