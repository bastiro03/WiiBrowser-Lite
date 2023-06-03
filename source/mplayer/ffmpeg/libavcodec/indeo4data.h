/*
 * Indeo Video Interactive 4 compatible decoder
 * Copyright (c) 2009-2010 Maxim Poliakovski
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * This file contains data needed for the Indeo 4 decoder.
 */

#ifndef AVCODEC_INDEO4DATA_H
#define AVCODEC_INDEO4DATA_H

#include <stdint.h>
#include "dsputil.h"
#include "ivi_common.h"

/**
 *  standard picture dimensions
 */
static const uint16_t ivi4_common_pic_sizes[14] = {
	640, 480, 320, 240, 160, 120, 704, 480, 352, 240, 352, 288, 176, 144
};

/**
 *  Indeo 4 8x8 scan (zigzag) patterns
 */
static const uint8_t ivi4_alternate_scan_8x8[64] = {
	0, 8, 1, 9, 16, 24, 2, 3, 17, 25, 10, 11, 32, 40, 48, 56,
	4, 5, 6, 7, 33, 41, 49, 57, 18, 19, 26, 27, 12, 13, 14, 15,
	34, 35, 43, 42, 50, 51, 59, 58, 20, 21, 22, 23, 31, 30, 29, 28,
	36, 37, 38, 39, 47, 46, 45, 44, 52, 53, 54, 55, 63, 62, 61, 60
};

static const uint8_t ivi4_alternate_scan_4x4[16] = {
	0, 1, 4, 5, 8, 12, 2, 3, 9, 13, 6, 7, 10, 11, 14, 15
};

static const uint8_t ivi4_vertical_scan_4x4[16] = {
	0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15
};

static const uint8_t ivi4_horizontal_scan_4x4[16] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static const uint8_t* scan_index_to_tab[15] = {
	// for 8x8 transforms
	ff_zigzag_direct,
	ivi4_alternate_scan_8x8,
	ff_ivi_horizontal_scan_8x8,
	ff_ivi_vertical_scan_8x8,
	ff_zigzag_direct,

	// for 4x4 transforms
	ff_ivi_direct_scan_4x4,
	ivi4_alternate_scan_4x4,
	ivi4_vertical_scan_4x4,
	ivi4_horizontal_scan_4x4,
	ff_ivi_direct_scan_4x4,

	// TODO: check if those are needed
	ff_ivi_horizontal_scan_8x8,
	ff_ivi_horizontal_scan_8x8,
	ff_ivi_horizontal_scan_8x8,
	ff_ivi_horizontal_scan_8x8,
	ff_ivi_horizontal_scan_8x8
};

/**
 *  Indeo 4 dequant tables
 */
static const uint16_t ivi4_quant_8x8_intra[9][64] = {
	{
		43, 342, 385, 470, 555, 555, 598, 726,
		342, 342, 470, 513, 555, 598, 726, 769,
		385, 470, 555, 555, 598, 726, 726, 811,
		470, 470, 555, 555, 598, 726, 769, 854,
		470, 555, 555, 598, 683, 726, 854, 1025,
		555, 555, 598, 683, 726, 854, 1025, 1153,
		555, 555, 598, 726, 811, 982, 1195, 1451,
		555, 598, 726, 811, 982, 1195, 1451, 1793
	},
	{
		86, 1195, 2390, 2390, 4865, 4865, 4865, 4865,
		1195, 1195, 2390, 2390, 4865, 4865, 4865, 4865,
		2390, 2390, 4865, 4865, 6827, 6827, 6827, 6827,
		2390, 2390, 4865, 4865, 6827, 6827, 6827, 6827,
		4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
		4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
		4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
		4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827
	},
	{
		235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
		235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
		235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
		235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
		235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
		235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
		235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
		235, 1067, 1195, 1323, 1451, 1579, 1707, 1835
	},
	{
		1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
		1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
		1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
		1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
		1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
		1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
		1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
		1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414
	},
	{
		897, 897, 897, 897, 897, 897, 897, 897,
		1067, 1067, 1067, 1067, 1067, 1067, 1067, 1067,
		1238, 1238, 1238, 1238, 1238, 1238, 1238, 1238,
		1409, 1409, 1409, 1409, 1409, 1409, 1409, 1409,
		1579, 1579, 1579, 1579, 1579, 1579, 1579, 1579,
		1750, 1750, 1750, 1750, 1750, 1750, 1750, 1750,
		1921, 1921, 1921, 1921, 1921, 1921, 1921, 1921,
		2091, 2091, 2091, 2091, 2091, 2091, 2091, 2091
	},
	{
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
		3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
		3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
		3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
		3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
		3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414
	},
	{
		2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
		2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
		2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
		2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
		2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
		2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
		2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
		2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390
	},
	{
		22, 171, 214, 257, 257, 299, 299, 342,
		171, 171, 257, 257, 299, 299, 342, 385,
		214, 257, 257, 299, 299, 342, 342, 385,
		257, 257, 257, 299, 299, 342, 385, 427,
		257, 257, 299, 299, 342, 385, 427, 513,
		257, 299, 299, 342, 385, 427, 513, 598,
		299, 299, 299, 385, 385, 470, 598, 726,
		299, 299, 385, 385, 470, 598, 726, 897
	},
	{
		86, 598, 1195, 1195, 2390, 2390, 2390, 2390,
		598, 598, 1195, 1195, 2390, 2390, 2390, 2390,
		1195, 1195, 2390, 2390, 3414, 3414, 3414, 3414,
		1195, 1195, 2390, 2390, 3414, 3414, 3414, 3414,
		2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
		2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
		2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
		2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414
	}
};

static const uint16_t ivi4_quant_8x8_inter[9][64] = {
	{
		427, 427, 470, 427, 427, 427, 470, 470,
		427, 427, 470, 427, 427, 427, 470, 470,
		470, 470, 470, 470, 470, 470, 470, 470,
		427, 427, 470, 470, 427, 427, 470, 470,
		427, 427, 470, 427, 427, 427, 470, 470,
		427, 427, 470, 427, 427, 427, 470, 470,
		470, 470, 470, 470, 470, 470, 470, 470,
		470, 470, 470, 470, 470, 470, 470, 470
	},
	{
		1707, 1707, 2433, 2433, 3414, 3414, 3414, 3414,
		1707, 1707, 2433, 2433, 3414, 3414, 3414, 3414,
		2433, 2433, 3414, 3414, 4822, 4822, 4822, 4822,
		2433, 2433, 3414, 3414, 4822, 4822, 4822, 4822,
		3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
		3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
		3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
		3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414
	},
	{
		1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
		1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
		1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
		1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
		1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
		1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
		1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
		1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281
	},
	{
		2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
		2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
		2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
		2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
		2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
		2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
		2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
		2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433
	},
	{
		1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
		1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
		1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281,
		1238, 1238, 1238, 1238, 1238, 1238, 1238, 1238,
		1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
		1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
		1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281,
		1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281
	},
	{
		2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
		2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
		3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
		3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
		2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
		2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
		2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
		2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433
	},
	{
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
		1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707
	},
	{
		86, 171, 171, 214, 214, 214, 214, 257,
		171, 171, 214, 214, 214, 214, 257, 257,
		171, 214, 214, 214, 214, 257, 257, 257,
		214, 214, 214, 214, 257, 257, 257, 299,
		214, 214, 214, 257, 257, 257, 299, 299,
		214, 214, 257, 257, 257, 299, 299, 299,
		214, 257, 257, 257, 299, 299, 299, 342,
		257, 257, 257, 299, 299, 299, 342, 342
	},
	{
		854, 854, 1195, 1195, 1707, 1707, 1707, 1707,
		854, 854, 1195, 1195, 1707, 1707, 1707, 1707,
		1195, 1195, 1707, 1707, 2390, 2390, 2390, 2390,
		1195, 1195, 1707, 1707, 2390, 2390, 2390, 2390,
		1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
		1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
		1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
		1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707
	}
};

static const uint16_t ivi4_quant_4x4_intra[5][16] = {
	{
		22, 214, 257, 299,
		214, 257, 299, 342,
		257, 299, 342, 427,
		299, 342, 427, 513
	},
	{
		129, 1025, 1451, 1451,
		1025, 1025, 1451, 1451,
		1451, 1451, 2049, 2049,
		1451, 1451, 2049, 2049
	},
	{
		43, 171, 171, 171,
		43, 171, 171, 171,
		43, 171, 171, 171,
		43, 171, 171, 171
	},
	{
		43, 43, 43, 43,
		171, 171, 171, 171,
		171, 171, 171, 171,
		171, 171, 171, 171
	},
	{
		43, 43, 43, 43,
		43, 43, 43, 43,
		43, 43, 43, 43,
		43, 43, 43, 43
	}
};

static const uint16_t ivi4_quant_4x4_inter[5][16] = {
	{
		107, 214, 257, 299,
		214, 257, 299, 299,
		257, 299, 299, 342,
		299, 299, 342, 342
	},
	{
		513, 1025, 1238, 1238,
		1025, 1025, 1238, 1238,
		1238, 1238, 1451, 1451,
		1238, 1238, 1451, 1451
	},
	{
		43, 171, 171, 171,
		43, 171, 171, 171,
		43, 171, 171, 171,
		43, 171, 171, 171
	},
	{
		43, 43, 43, 43,
		171, 171, 171, 171,
		171, 171, 171, 171,
		171, 171, 171, 171
	},
	{
		43, 43, 43, 43,
		43, 43, 43, 43,
		43, 43, 43, 43,
		43, 43, 43, 43
	}
};

/**
 *  Table for mapping quant matrix index from the bitstream
 *  into internal quant table number.
 */
static const uint8_t quant_index_to_tab[22] = {
	0, 1, 0, 2, 1, 3, 0, 4, 1, 5, 0, 1, 6, 7, 8, // for 8x8 quant matrixes
	0, 1, 2, 2, 3, 3, 4 // for 4x4 quant matrixes
};

#endif /* AVCODEC_INDEO4DATA_H */
