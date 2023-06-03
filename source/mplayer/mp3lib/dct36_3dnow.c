/*
 * dct36_3dnow.c - 3DNow! optimized dct36()
 *
 * This code based 'dct36_3dnow.s' by Syuuhei Kashiyama
 * <squash@mb.kcom.ne.jp>, only two types of changes have been made:
 *
 * - removed PREFETCH instruction for speedup
 * - changed function name for support 3DNow! automatic detection
 *
 * You can find Kashiyama's original 3dnow! support patch
 * (for mpg123-0.59o) at
 * http://user.ecc.u-tokyo.ac.jp/~g810370/linux-simd/ (Japanese).
 *
 * by KIMURA Takuhiro <kim@hannah.ipc.miyakyo-u.ac.jp> - until 31.Mar.1999
 *                    <kim@comtec.co.jp>               - after  1.Apr.1999
 *
 * Modified for use with MPlayer, for details see the changelog at
 * http://svn.mplayerhq.hu/mplayer/trunk/
 * $Id: dct36_3dnow.c 31281 2010-05-30 10:01:40Z reimar $
 *
 * Original disclaimer:
 *  The author of this program disclaim whole expressed or implied
 *  warranties with regard to this program, and in no event shall the
 *  author of this program liable to whatever resulted from the use of
 *  this program. Use it at your own risk.
 *
 * 2003/06/21: Moved to GCC inline assembly - Alex Beregszaszi
 */

#include "config.h"
#include "mangle.h"
#include "mpg123.h"
#include "libavutil/x86_cpu.h"

#ifdef DCT36_OPTIMIZE_FOR_K7
void dct36_3dnowex(real* inbuf, real* o1,
                   real* o2, real* wintab, real* tsbuf)
#else
void dct36_3dnow(real* inbuf, real* o1,
	real* o2, real* wintab, real* tsbuf)
#endif
{
	volatile __asm__ (
		
	"movq (%%"
	REG_a
	"),%%mm0\n\t"
		"movq 4(%%"
	REG_a
	"),%%mm1\n\t"
		"pfadd %%mm1,%%mm0\n\t"
		"movq %%mm0,4(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm1\n\t"
		"movq 12(%%"
	REG_a
	"),%%mm2\n\t"
		"punpckldq %%mm2,%%mm1\n\t"
		"pfadd %%mm2,%%mm1\n\t"
		"movq %%mm1,12(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm2\n\t"
		"movq 20(%%"
	REG_a
	"),%%mm3\n\t"
		"punpckldq %%mm3,%%mm2\n\t"
		"pfadd %%mm3,%%mm2\n\t"
		"movq %%mm2,20(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm3\n\t"
		"movq 28(%%"
	REG_a
	"),%%mm4\n\t"
		"punpckldq %%mm4,%%mm3\n\t"
		"pfadd %%mm4,%%mm3\n\t"
		"movq %%mm3,28(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm4\n\t"
		"movq 36(%%"
	REG_a
	"),%%mm5\n\t"
		"punpckldq %%mm5,%%mm4\n\t"
		"pfadd %%mm5,%%mm4\n\t"
		"movq %%mm4,36(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movq 44(%%"
	REG_a
	"),%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movq %%mm5,44(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm6\n\t"
		"movq 52(%%"
	REG_a
	"),%%mm7\n\t"
		"punpckldq %%mm7,%%mm6\n\t"
		"pfadd %%mm7,%%mm6\n\t"
		"movq %%mm6,52(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm7\n\t"
		"movq 60(%%"
	REG_a
	"),%%mm0\n\t"
		"punpckldq %%mm0,%%mm7\n\t"
		"pfadd %%mm0,%%mm7\n\t"
		"movq %%mm7,60(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm0\n\t"
		"movd 68(%%"
	REG_a
	"),%%mm1\n\t"
		"pfadd %%mm1,%%mm0\n\t"
		"movd %%mm0,68(%%"
	REG_a
	")\n\t"
		"movd 4(%%"
	REG_a
	"),%%mm0\n\t"
		"movd 12(%%"
	REG_a
	"),%%mm1\n\t"
		"punpckldq %%mm1,%%mm0\n\t"
		"punpckldq 20(%%"
	REG_a
	"),%%mm1\n\t"
		"pfadd %%mm1,%%mm0\n\t"
		"movd %%mm0,12(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm0\n\t"
		"movd %%mm0,20(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm1\n\t"
		"movd 28(%%"
	REG_a
	"),%%mm2\n\t"
		"punpckldq %%mm2,%%mm1\n\t"
		"punpckldq 36(%%"
	REG_a
	"),%%mm2\n\t"
		"pfadd %%mm2,%%mm1\n\t"
		"movd %%mm1,28(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm1\n\t"
		"movd %%mm1,36(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm2\n\t"
		"movd 44(%%"
	REG_a
	"),%%mm3\n\t"
		"punpckldq %%mm3,%%mm2\n\t"
		"punpckldq 52(%%"
	REG_a
	"),%%mm3\n\t"
		"pfadd %%mm3,%%mm2\n\t"
		"movd %%mm2,44(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm2\n\t"
		"movd %%mm2,52(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm3\n\t"
		"movd 60(%%"
	REG_a
	"),%%mm4\n\t"
		"punpckldq %%mm4,%%mm3\n\t"
		"punpckldq 68(%%"
	REG_a
	"),%%mm4\n\t"
		"pfadd %%mm4,%%mm3\n\t"
		"movd %%mm3,60(%%"
	REG_a
	")\n\t"
		"psrlq $32,%%mm3\n\t"
		"movd %%mm3,68(%%"
	REG_a
	")\n\t"

		"movq 24(%%"
	REG_a
	"),%%mm0\n\t"
		"movq 48(%%"
	REG_a
	"),%%mm1\n\t"
		"movd "
	MANGLE(COS9)
	"+12,%%mm2\n\t"
		"punpckldq %%mm2,%%mm2\n\t"
		"movd "
	MANGLE(COS9)
	"+24,%%mm3\n\t"
		"punpckldq %%mm3,%%mm3\n\t"
		"pfmul %%mm2,%%mm0\n\t"
		"pfmul %%mm3,%%mm1\n\t"
		"push %%"
	REG_a
	"\n\t"
		"movl $1,%%eax\n\t"
		"movd %%eax,%%mm7\n\t"
		"pi2fd %%mm7,%%mm7\n\t"
		"pop %%"
	REG_a
	"\n\t"
		"movq 8(%%"
	REG_a
	"),%%mm2\n\t"
		"movd "
	MANGLE(COS9)
	"+4,%%mm3\n\t"
		"punpckldq %%mm3,%%mm3\n\t"
		"pfmul %%mm3,%%mm2\n\t"
		"pfadd %%mm0,%%mm2\n\t"
		"movq 40(%%"
	REG_a
	"),%%mm3\n\t"
		"movd "
	MANGLE(COS9)
	"+20,%%mm4\n\t"
		"punpckldq %%mm4,%%mm4\n\t"
		"pfmul %%mm4,%%mm3\n\t"
		"pfadd %%mm3,%%mm2\n\t"
		"movq 56(%%"
	REG_a
	"),%%mm3\n\t"
		"movd "
	MANGLE(COS9)
	"+28,%%mm4\n\t"
		"punpckldq %%mm4,%%mm4\n\t"
		"pfmul %%mm4,%%mm3\n\t"
		"pfadd %%mm3,%%mm2\n\t"
		"movq (%%"
	REG_a
	"),%%mm3\n\t"
		"movq 16(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+8,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfadd %%mm4,%%mm3\n\t"
		"movq 32(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+16,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfadd %%mm4,%%mm3\n\t"
		"pfadd %%mm1,%%mm3\n\t"
		"movq 64(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+32,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfadd %%mm4,%%mm3\n\t"
		"movq %%mm2,%%mm4\n\t"
		"pfadd %%mm3,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+0,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 108(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 104(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
#ifdef DCT36_OPTIMIZE_FOR_K7
		"pswapd %%mm5,%%mm5\n\t"
		"movq %%mm5,32(%%"
	REG_c
	")\n\t"
#else
		"movd %%mm5,36(%%"REG_c")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,32(%%"REG_c")\n\t"
#endif
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 32(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 36(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 32(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 36(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,1024(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,1152(%%"
	REG_D
	")\n\t"
		"movq %%mm3,%%mm4\n\t"
		"pfsub %%mm2,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+32,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 140(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 72(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd %%mm5,68(%%"
	REG_c
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,0(%%"
	REG_c
	")\n\t"
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 0(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 68(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 0(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 68(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,0(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,2176(%%"
	REG_D
	")\n\t"
		"movq 8(%%"
	REG_a
	"),%%mm2\n\t"
		"movq 40(%%"
	REG_a
	"),%%mm3\n\t"
		"pfsub %%mm3,%%mm2\n\t"
		"movq 56(%%"
	REG_a
	"),%%mm3\n\t"
		"pfsub %%mm3,%%mm2\n\t"
		"movd "
	MANGLE(COS9)
	"+12,%%mm3\n\t"
		"punpckldq %%mm3,%%mm3\n\t"
		"pfmul %%mm3,%%mm2\n\t"
		"movq 16(%%"
	REG_a
	"),%%mm3\n\t"
		"movq 32(%%"
	REG_a
	"),%%mm4\n\t"
		"pfsub %%mm4,%%mm3\n\t"
		"movq 64(%%"
	REG_a
	"),%%mm4\n\t"
		"pfsub %%mm4,%%mm3\n\t"
		"movd "
	MANGLE(COS9)
	"+24,%%mm4\n\t"
		"punpckldq %%mm4,%%mm4\n\t"
		"pfmul %%mm4,%%mm3\n\t"
		"movq 48(%%"
	REG_a
	"),%%mm4\n\t"
		"pfsub %%mm4,%%mm3\n\t"
		"movq (%%"
	REG_a
	"),%%mm4\n\t"
		"pfadd %%mm4,%%mm3\n\t"
		"movq %%mm2,%%mm4\n\t"
		"pfadd %%mm3,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+4,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 112(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 100(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd %%mm5,40(%%"
	REG_c
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,28(%%"
	REG_c
	")\n\t"
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 28(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 40(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 28(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 40(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,896(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,1280(%%"
	REG_D
	")\n\t"
		"movq %%mm3,%%mm4\n\t"
		"pfsub %%mm2,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+28,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 136(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 76(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd %%mm5,64(%%"
	REG_c
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,4(%%"
	REG_c
	")\n\t"
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 4(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 64(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 4(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 64(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,128(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,2048(%%"
	REG_D
	")\n\t"

		"movq 8(%%"
	REG_a
	"),%%mm2\n\t"
		"movd "
	MANGLE(COS9)
	"+20,%%mm3\n\t"
		"punpckldq %%mm3,%%mm3\n\t"
		"pfmul %%mm3,%%mm2\n\t"
		"pfsub %%mm0,%%mm2\n\t"
		"movq 40(%%"
	REG_a
	"),%%mm3\n\t"
		"movd "
	MANGLE(COS9)
	"+28,%%mm4\n\t"
		"punpckldq %%mm4,%%mm4\n\t"
		"pfmul %%mm4,%%mm3\n\t"
		"pfsub %%mm3,%%mm2\n\t"
		"movq 56(%%"
	REG_a
	"),%%mm3\n\t"
		"movd "
	MANGLE(COS9)
	"+4,%%mm4\n\t"
		"punpckldq %%mm4,%%mm4\n\t"
		"pfmul %%mm4,%%mm3\n\t"
		"pfadd %%mm3,%%mm2\n\t"
		"movq (%%"
	REG_a
	"),%%mm3\n\t"
		"movq 16(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+32,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfsub %%mm4,%%mm3\n\t"
		"movq 32(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+8,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfsub %%mm4,%%mm3\n\t"
		"pfadd %%mm1,%%mm3\n\t"
		"movq 64(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+16,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfadd %%mm4,%%mm3\n\t"
		"movq %%mm2,%%mm4\n\t"
		"pfadd %%mm3,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+8,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 116(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 96(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd %%mm5,44(%%"
	REG_c
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,24(%%"
	REG_c
	")\n\t"
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 24(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 44(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 24(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 44(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,768(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,1408(%%"
	REG_D
	")\n\t"
		"movq %%mm3,%%mm4\n\t"
		"pfsub %%mm2,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+24,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 132(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 80(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd %%mm5,60(%%"
	REG_c
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,8(%%"
	REG_c
	")\n\t"
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 8(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 60(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 8(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 60(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,256(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,1920(%%"
	REG_D
	")\n\t"
		"movq 8(%%"
	REG_a
	"),%%mm2\n\t"
		"movd "
	MANGLE(COS9)
	"+28,%%mm3\n\t"
		"punpckldq %%mm3,%%mm3\n\t"
		"pfmul %%mm3,%%mm2\n\t"
		"pfsub %%mm0,%%mm2\n\t"
		"movq 40(%%"
	REG_a
	"),%%mm3\n\t"
		"movd "
	MANGLE(COS9)
	"+4,%%mm4\n\t"
		"punpckldq %%mm4,%%mm4\n\t"
		"pfmul %%mm4,%%mm3\n\t"
		"pfadd %%mm3,%%mm2\n\t"
		"movq 56(%%"
	REG_a
	"),%%mm3\n\t"
		"movd "
	MANGLE(COS9)
	"+20,%%mm4\n\t"
		"punpckldq %%mm4,%%mm4\n\t"
		"pfmul %%mm4,%%mm3\n\t"
		"pfsub %%mm3,%%mm2\n\t"
		"movq (%%"
	REG_a
	"),%%mm3\n\t"
		"movq 16(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+16,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfsub %%mm4,%%mm3\n\t"
		"movq 32(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+32,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfadd %%mm4,%%mm3\n\t"
		"pfadd %%mm1,%%mm3\n\t"
		"movq 64(%%"
	REG_a
	"),%%mm4\n\t"
		"movd "
	MANGLE(COS9)
	"+8,%%mm5\n\t"
		"punpckldq %%mm5,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"pfsub %%mm4,%%mm3\n\t"
		"movq %%mm2,%%mm4\n\t"
		"pfadd %%mm3,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+12,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 120(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 92(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd %%mm5,48(%%"
	REG_c
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,20(%%"
	REG_c
	")\n\t"
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 20(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 48(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 20(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 48(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,640(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,1536(%%"
	REG_D
	")\n\t"
		"movq %%mm3,%%mm4\n\t"
		"pfsub %%mm2,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+20,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 128(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 84(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd %%mm5,56(%%"
	REG_c
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,12(%%"
	REG_c
	")\n\t"
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 12(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 56(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 12(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 56(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,384(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,1792(%%"
	REG_D
	")\n\t"

		"movq (%%"
	REG_a
	"),%%mm4\n\t"
		"movq 16(%%"
	REG_a
	"),%%mm3\n\t"
		"pfsub %%mm3,%%mm4\n\t"
		"movq 32(%%"
	REG_a
	"),%%mm3\n\t"
		"pfadd %%mm3,%%mm4\n\t"
		"movq 48(%%"
	REG_a
	"),%%mm3\n\t"
		"pfsub %%mm3,%%mm4\n\t"
		"movq 64(%%"
	REG_a
	"),%%mm3\n\t"
		"pfadd %%mm3,%%mm4\n\t"
		"movq %%mm7,%%mm5\n\t"
		"punpckldq "
	MANGLE(tfcos36)
	"+16,%%mm5\n\t"
		"pfmul %%mm5,%%mm4\n\t"
		"movq %%mm4,%%mm5\n\t"
		"pfacc %%mm5,%%mm5\n\t"
		"movd 124(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 88(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd %%mm5,52(%%"
	REG_c
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,16(%%"
	REG_c
	")\n\t"
		"movq %%mm4,%%mm6\n\t"
		"punpckldq %%mm6,%%mm5\n\t"
		"pfsub %%mm6,%%mm5\n\t"
		"punpckhdq %%mm5,%%mm5\n\t"
		"movd 16(%%"
	REG_d
	"),%%mm6\n\t"
		"punpckldq 52(%%"
	REG_d
	"),%%mm6\n\t"
		"pfmul %%mm6,%%mm5\n\t"
		"movd 16(%%"
	REG_S
	"),%%mm6\n\t"
		"punpckldq 52(%%"
	REG_S
	"),%%mm6\n\t"
		"pfadd %%mm6,%%mm5\n\t"
		"movd %%mm5,512(%%"
	REG_D
	")\n\t"
		"psrlq $32,%%mm5\n\t"
		"movd %%mm5,1664(%%"
	REG_D
	")\n\t"

		"femms\n\t"
	:
	:
	"a"(inbuf), "S"(o1), "c"(o2), "d"(wintab), "D"(tsbuf)
	:
	"memory"
	)
}
