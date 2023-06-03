/*
 * Bink DSP routines
 * Copyright (c) 2009 Kostya Shishkov
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Bink DSP routines
 */

#include "dsputil.h"
#include "binkdsp.h"
#ifdef HAVE_PAIRED
#include "libavutil/ppc/paired.h"
#endif

#define A1  2896 /* (1/sqrt(2))<<12 */
#define A2  2217
#define A3  3784
#define A4 -5352

#define IDCT_TRANSFORM(dest,s0,s1,s2,s3,s4,s5,s6,s7,d0,d1,d2,d3,d4,d5,d6,d7,munge,src) {\
    const int a0 = (src)[s0] + (src)[s4]; \
    const int a1 = (src)[s0] - (src)[s4]; \
    const int a2 = (src)[s2] + (src)[s6]; \
    const int a3 = (A1*((src)[s2] - (src)[s6])) >> 11; \
    const int a4 = (src)[s5] + (src)[s3]; \
    const int a5 = (src)[s5] - (src)[s3]; \
    const int a6 = (src)[s1] + (src)[s7]; \
    const int a7 = (src)[s1] - (src)[s7]; \
    const int b0 = a4 + a6; \
    const int b1 = (A3*(a5 + a7)) >> 11; \
    const int b2 = ((A4*a5) >> 11) - b0 + b1; \
    const int b3 = (A1*(a6 - a4) >> 11) - b2; \
    const int b4 = ((A2*a7) >> 11) + b3 - b1; \
    (dest)[d0] = munge(a0+a2   +b0); \
    (dest)[d1] = munge(a1+a3-a2+b2); \
    (dest)[d2] = munge(a1-a3+a2+b3); \
    (dest)[d3] = munge(a0-a2   -b4); \
    (dest)[d4] = munge(a0-a2   +b4); \
    (dest)[d5] = munge(a1-a3+a2-b3); \
    (dest)[d6] = munge(a1+a3-a2-b2); \
    (dest)[d7] = munge(a0+a2   -b0); \
}
/* end IDCT_TRANSFORM macro */

#define MUNGE_NONE(x) (x)
#define IDCT_COL(dest,src) IDCT_TRANSFORM(dest,0,8,16,24,32,40,48,56,0,8,16,24,32,40,48,56,MUNGE_NONE,src)

#define MUNGE_ROW(x) (((x) + 0x7F)>>8)
#define IDCT_ROW(dest,src) IDCT_TRANSFORM(dest,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,MUNGE_ROW,src)

static inline void bink_idct_col(int* dest, const int32_t* src)
{
	if ((src[8] | src[16] | src[24] | src[32] | src[40] | src[48] | src[56]) == 0)
	{
		dest[0] =
			dest[8] =
			dest[16] =
			dest[24] =
			dest[32] =
			dest[40] =
			dest[48] =
			dest[56] = src[0];
	}
	else
	{
		IDCT_COL(dest, src);
	}
}

static void bink_idct_c(int32_t* block)
{
	int i;
	int temp[64];

	for (i = 0; i < 8; i++)
		bink_idct_col(&temp[i], &block[i]);
	for (i = 0; i < 8; i++)
	{
		IDCT_ROW((&block[8 * i]), (&temp[8 * i]));
	}
}

static void bink_idct_add_c(uint8_t* dest, int linesize, int32_t* block)
{
	int i, j;

	bink_idct_c(block);
	for (i = 0; i < 8; i++, dest += linesize, block += 8)
		for (j = 0; j < 8; j++)
			dest[j] += block[j];
}

static void bink_idct_put_c(uint8_t* dest, int linesize, int32_t* block)
{
	int i;
	int temp[64];
	for (i = 0; i < 8; i++)
		bink_idct_col(&temp[i], &block[i]);
	for (i = 0; i < 8; i++)
	{
		IDCT_ROW((&dest[i * linesize]), (&temp[8 * i]));
	}
}

static void scale_block_c(const uint8_t src[64]/*align 8*/, uint8_t* dst/*align 8*/, int linesize)
{
	int i, j;
	uint16_t* dst1 = (uint16_t*)dst;
	uint16_t* dst2 = (uint16_t*)(dst + linesize);

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 8; i++)
		{
			dst1[i] = dst2[i] = src[i] * 0x0101;
		}
		src += 8;
		dst1 += linesize;
		dst2 += linesize;
	}
}

#ifdef HAVE_PAIRED
static void scale_block_paired(const uint8_t src[64], uint8_t* dst, int linesize)
{
	const float scalar = 257.0;
	vector float pair;

	uint16_t* dst1 = (uint16_t*)(dst - linesize * 2);
	uint16_t* dst2 = (uint16_t*)(dst - linesize);
	src -= 8;

	for (int i = 0; i < 8; i++) {
		pair = psq_lu(8, src, 0, 4);
		pair = ps_mul(pair, scalar);
		psq_stux(pair, dst1, linesize * 2, 0, 5);
		psq_stux(pair, dst2, linesize * 2, 0, 5);

		pair = psq_l(2, src, 0, 4);
		pair = ps_mul(pair, scalar);
		psq_st(pair, 4, dst1, 0, 5);
		psq_st(pair, 4, dst2, 0, 5);

		pair = psq_l(4, src, 0, 4);
		pair = ps_mul(pair, scalar);
		psq_st(pair, 8, dst1, 0, 5);
		psq_st(pair, 8, dst2, 0, 5);

		pair = psq_l(6, src, 0, 4);
		pair = ps_mul(pair, scalar);
		psq_st(pair, 12, dst1, 0, 5);
		psq_st(pair, 12, dst2, 0, 5);
	}
}
#endif

void ff_binkdsp_init(BinkDSPContext* c)
{
	c->idct_add = bink_idct_add_c;
	c->idct_put = bink_idct_put_c;
	c->scale_block = scale_block_c;
	if (HAVE_PAIRED) c->scale_block = scale_block_paired;
}
