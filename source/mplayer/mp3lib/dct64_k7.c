/*
* This code was taken from http://www.mpg123.org
* See ChangeLog of mpg123-0.59s-pre.1 for detail
* Applied to mplayer by Nick Kurshev <nickols_k@mail.ru>
* Partial 3dnowex-DSP! optimization by Nick Kurshev
*
* TODO: optimize scalar 3dnow! code
* Warning: Phases 7 & 8 are not tested
*/

#include "config.h"
#include "mangle.h"
#include "mpg123.h"
#include "libavutil/x86_cpu.h"

static unsigned long long int attribute_used __attribute__((aligned(8))) x_plus_minus_3dnow
=
0x8000000000000000ULL;
static float attribute_used plus_1f = 1.0;

void dct64_MMX_3dnowex(short* a, short* b, real* c)
{
	char tmp[256];
	volatile __asm__ (
		
	"       mov %2,%%"
	REG_a
	"\n\t"

		"       lea 128+%3,%%"
	REG_d
	"\n\t"
		"       mov %0,%%"
	REG_S
	"\n\t"
		"       mov %1,%%"
	REG_D
	"\n\t"
		"       mov $"
	MANGLE(costab_mmx)
	",%%"
	REG_b
	"\n\t"
		"       lea %3,%%"
	REG_c
	"\n\t"

		/* Phase 1*/
		"       movq    (%%"
	REG_a
	"), %%mm0\n\t"
		"       movq    8(%%"
	REG_a
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  120(%%"
	REG_a
	"), %%mm1\n\t"
		"       pswapd  112(%%"
	REG_a
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, (%%"
	REG_d
	")\n\t"
		"       movq    %%mm4, 8(%%"
	REG_d
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsub   %%mm5, %%mm7\n\t"
		"       pfmul   (%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul   8(%%"
	REG_b
	"), %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 120(%%"
	REG_d
	")\n\t"
		"       movq    %%mm7, 112(%%"
	REG_d
	")\n\t"

		"       movq    16(%%"
	REG_a
	"), %%mm0\n\t"
		"       movq    24(%%"
	REG_a
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  104(%%"
	REG_a
	"), %%mm1\n\t"
		"       pswapd  96(%%"
	REG_a
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 16(%%"
	REG_d
	")\n\t"
		"       movq    %%mm4, 24(%%"
	REG_d
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsub   %%mm5, %%mm7\n\t"
		"       pfmul   16(%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul   24(%%"
	REG_b
	"), %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 104(%%"
	REG_d
	")\n\t"
		"       movq    %%mm7, 96(%%"
	REG_d
	")\n\t"

		"       movq    32(%%"
	REG_a
	"), %%mm0\n\t"
		"       movq    40(%%"
	REG_a
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  88(%%"
	REG_a
	"), %%mm1\n\t"
		"       pswapd  80(%%"
	REG_a
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 32(%%"
	REG_d
	")\n\t"
		"       movq    %%mm4, 40(%%"
	REG_d
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsub   %%mm5, %%mm7\n\t"
		"       pfmul   32(%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul   40(%%"
	REG_b
	"), %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 88(%%"
	REG_d
	")\n\t"
		"       movq    %%mm7, 80(%%"
	REG_d
	")\n\t"

		"       movq    48(%%"
	REG_a
	"), %%mm0\n\t"
		"       movq    56(%%"
	REG_a
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  72(%%"
	REG_a
	"), %%mm1\n\t"
		"       pswapd  64(%%"
	REG_a
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 48(%%"
	REG_d
	")\n\t"
		"       movq    %%mm4, 56(%%"
	REG_d
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsub   %%mm5, %%mm7\n\t"
		"       pfmul   48(%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul   56(%%"
	REG_b
	"), %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 72(%%"
	REG_d
	")\n\t"
		"       movq    %%mm7, 64(%%"
	REG_d
	")\n\t"

		/* Phase 2*/

		"       movq    (%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    8(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  56(%%"
	REG_d
	"), %%mm1\n\t"
		"       pswapd  48(%%"
	REG_d
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, (%%"
	REG_c
	")\n\t"
		"       movq    %%mm4, 8(%%"
	REG_c
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsub   %%mm5, %%mm7\n\t"
		"       pfmul   64(%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul   72(%%"
	REG_b
	"), %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 56(%%"
	REG_c
	")\n\t"
		"       movq    %%mm7, 48(%%"
	REG_c
	")\n\t"

		"       movq    16(%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    24(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  40(%%"
	REG_d
	"), %%mm1\n\t"
		"       pswapd  32(%%"
	REG_d
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 16(%%"
	REG_c
	")\n\t"
		"       movq    %%mm4, 24(%%"
	REG_c
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsub   %%mm5, %%mm7\n\t"
		"       pfmul   80(%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul   88(%%"
	REG_b
	"), %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 40(%%"
	REG_c
	")\n\t"
		"       movq    %%mm7, 32(%%"
	REG_c
	")\n\t"

		/* Phase 3*/

		"       movq    64(%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    72(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  120(%%"
	REG_d
	"), %%mm1\n\t"
		"       pswapd  112(%%"
	REG_d
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 64(%%"
	REG_c
	")\n\t"
		"       movq    %%mm4, 72(%%"
	REG_c
	")\n\t"
		"       pfsubr  %%mm1, %%mm3\n\t"
		"       pfsubr  %%mm5, %%mm7\n\t"
		"       pfmul   64(%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul   72(%%"
	REG_b
	"), %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 120(%%"
	REG_c
	")\n\t"
		"       movq    %%mm7, 112(%%"
	REG_c
	")\n\t"

		"       movq    80(%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    88(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  104(%%"
	REG_d
	"), %%mm1\n\t"
		"       pswapd  96(%%"
	REG_d
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 80(%%"
	REG_c
	")\n\t"
		"       movq    %%mm4, 88(%%"
	REG_c
	")\n\t"
		"       pfsubr  %%mm1, %%mm3\n\t"
		"       pfsubr  %%mm5, %%mm7\n\t"
		"       pfmul   80(%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul   88(%%"
	REG_b
	"), %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 104(%%"
	REG_c
	")\n\t"
		"       movq    %%mm7, 96(%%"
	REG_c
	")\n\t"

		/* Phase 4*/

		"       movq    96(%%"
	REG_b
	"), %%mm2\n\t"
		"       movq    104(%%"
	REG_b
	"), %%mm6\n\t"

		"       movq    (%%"
	REG_c
	"), %%mm0\n\t"
		"       movq    8(%%"
	REG_c
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  24(%%"
	REG_c
	"), %%mm1\n\t"
		"       pswapd  16(%%"
	REG_c
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, (%%"
	REG_d
	")\n\t"
		"       movq    %%mm4, 8(%%"
	REG_d
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsub   %%mm5, %%mm7\n\t"
		"       pfmul   %%mm2, %%mm3\n\t"
		"       pfmul   %%mm6, %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 24(%%"
	REG_d
	")\n\t"
		"       movq    %%mm7, 16(%%"
	REG_d
	")\n\t"

		"       movq    32(%%"
	REG_c
	"), %%mm0\n\t"
		"       movq    40(%%"
	REG_c
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  56(%%"
	REG_c
	"), %%mm1\n\t"
		"       pswapd  48(%%"
	REG_c
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 32(%%"
	REG_d
	")\n\t"
		"       movq    %%mm4, 40(%%"
	REG_d
	")\n\t"
		"       pfsubr  %%mm1, %%mm3\n\t"
		"       pfsubr  %%mm5, %%mm7\n\t"
		"       pfmul   %%mm2, %%mm3\n\t"
		"       pfmul   %%mm6, %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 56(%%"
	REG_d
	")\n\t"
		"       movq    %%mm7, 48(%%"
	REG_d
	")\n\t"

		"       movq    64(%%"
	REG_c
	"), %%mm0\n\t"
		"       movq    72(%%"
	REG_c
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  88(%%"
	REG_c
	"), %%mm1\n\t"
		"       pswapd  80(%%"
	REG_c
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 64(%%"
	REG_d
	")\n\t"
		"       movq    %%mm4, 72(%%"
	REG_d
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsub   %%mm5, %%mm7\n\t"
		"       pfmul   %%mm2, %%mm3\n\t"
		"       pfmul   %%mm6, %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 88(%%"
	REG_d
	")\n\t"
		"       movq    %%mm7, 80(%%"
	REG_d
	")\n\t"

		"       movq    96(%%"
	REG_c
	"), %%mm0\n\t"
		"       movq    104(%%"
	REG_c
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  120(%%"
	REG_c
	"), %%mm1\n\t"
		"       pswapd  112(%%"
	REG_c
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 96(%%"
	REG_d
	")\n\t"
		"       movq    %%mm4, 104(%%"
	REG_d
	")\n\t"
		"       pfsubr  %%mm1, %%mm3\n\t"
		"       pfsubr  %%mm5, %%mm7\n\t"
		"       pfmul   %%mm2, %%mm3\n\t"
		"       pfmul   %%mm6, %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 120(%%"
	REG_d
	")\n\t"
		"       movq    %%mm7, 112(%%"
	REG_d
	")\n\t"

		/* Phase 5 */

		"       movq    112(%%"
	REG_b
	"), %%mm2\n\t"

		"       movq    (%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    16(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  8(%%"
	REG_d
	"), %%mm1\n\t"
		"       pswapd  24(%%"
	REG_d
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, (%%"
	REG_c
	")\n\t"
		"       movq    %%mm4, 16(%%"
	REG_c
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsubr  %%mm5, %%mm7\n\t"
		"       pfmul   %%mm2, %%mm3\n\t"
		"       pfmul   %%mm2, %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 8(%%"
	REG_c
	")\n\t"
		"       movq    %%mm7, 24(%%"
	REG_c
	")\n\t"

		"       movq    32(%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    48(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  40(%%"
	REG_d
	"), %%mm1\n\t"
		"       pswapd  56(%%"
	REG_d
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 32(%%"
	REG_c
	")\n\t"
		"       movq    %%mm4, 48(%%"
	REG_c
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsubr  %%mm5, %%mm7\n\t"
		"       pfmul   %%mm2, %%mm3\n\t"
		"       pfmul   %%mm2, %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 40(%%"
	REG_c
	")\n\t"
		"       movq    %%mm7, 56(%%"
	REG_c
	")\n\t"

		"       movq    64(%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    80(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  72(%%"
	REG_d
	"), %%mm1\n\t"
		"       pswapd  88(%%"
	REG_d
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 64(%%"
	REG_c
	")\n\t"
		"       movq    %%mm4, 80(%%"
	REG_c
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsubr  %%mm5, %%mm7\n\t"
		"       pfmul   %%mm2, %%mm3\n\t"
		"       pfmul   %%mm2, %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 72(%%"
	REG_c
	")\n\t"
		"       movq    %%mm7, 88(%%"
	REG_c
	")\n\t"

		"       movq    96(%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    112(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq    %%mm0, %%mm3\n\t"
		"       movq    %%mm4, %%mm7\n\t"
		"       pswapd  104(%%"
	REG_d
	"), %%mm1\n\t"
		"       pswapd  120(%%"
	REG_d
	"), %%mm5\n\t"
		"       pfadd   %%mm1, %%mm0\n\t"
		"       pfadd   %%mm5, %%mm4\n\t"
		"       movq    %%mm0, 96(%%"
	REG_c
	")\n\t"
		"       movq    %%mm4, 112(%%"
	REG_c
	")\n\t"
		"       pfsub   %%mm1, %%mm3\n\t"
		"       pfsubr  %%mm5, %%mm7\n\t"
		"       pfmul   %%mm2, %%mm3\n\t"
		"       pfmul   %%mm2, %%mm7\n\t"
		"       pswapd  %%mm3, %%mm3\n\t"
		"       pswapd  %%mm7, %%mm7\n\t"
		"       movq    %%mm3, 104(%%"
	REG_c
	")\n\t"
		"       movq    %%mm7, 120(%%"
	REG_c
	")\n\t"

		/* Phase 6. This is the end of easy road. */
		/* Code below is coded in scalar mode. Should be optimized */

		"       movd    "
	MANGLE(plus_1f)
	", %%mm6\n\t"
		"       punpckldq 120(%%"
	REG_b
	"), %%mm6\n\t" /* mm6 = 1.0 | 120(%%"REG_b")*/
		"       movq    "
	MANGLE(x_plus_minus_3dnow)
	", %%mm7\n\t" /* mm7 = +1 | -1 */

		"       movq    32(%%"
	REG_c
	"), %%mm0\n\t"
		"       movq    64(%%"
	REG_c
	"), %%mm2\n\t"
		"       movq    %%mm0, %%mm1\n\t"
		"       movq    %%mm2, %%mm3\n\t"
		"       pxor    %%mm7, %%mm1\n\t"
		"       pxor    %%mm7, %%mm3\n\t"
		"       pfacc   %%mm1, %%mm0\n\t"
		"       pfacc   %%mm3, %%mm2\n\t"
		"       pfmul   %%mm6, %%mm0\n\t"
		"       pfmul   %%mm6, %%mm2\n\t"
		"       movq    %%mm0, 32(%%"
	REG_d
	")\n\t"
		"       movq    %%mm2, 64(%%"
	REG_d
	")\n\t"

		"       movd    44(%%"
	REG_c
	"), %%mm0\n\t"
		"       movd    40(%%"
	REG_c
	"), %%mm2\n\t"
		"       movd    120(%%"
	REG_b
	"), %%mm3\n\t"
		"       punpckldq 76(%%"
	REG_c
	"), %%mm0\n\t"
		"       punpckldq 72(%%"
	REG_c
	"), %%mm2\n\t"
		"       punpckldq %%mm3, %%mm3\n\t"
		"       movq    %%mm0, %%mm4\n\t"
		"       movq    %%mm2, %%mm5\n\t"
		"       pfsub   %%mm2, %%mm0\n\t"
		"       pfmul   %%mm3, %%mm0\n\t"
		"       movq    %%mm0, %%mm1\n\t"
		"       pfadd   %%mm5, %%mm0\n\t"
		"       pfadd   %%mm4, %%mm0\n\t"
		"       movq    %%mm0, %%mm2\n\t"
		"       punpckldq %%mm1, %%mm0\n\t"
		"       punpckhdq %%mm1, %%mm2\n\t"
		"       movq    %%mm0, 40(%%"
	REG_d
	")\n\t"
		"       movq    %%mm2, 72(%%"
	REG_d
	")\n\t"

		"       movd   48(%%"
	REG_c
	"), %%mm3\n\t"
		"       movd   60(%%"
	REG_c
	"), %%mm2\n\t"
		"       pfsub  52(%%"
	REG_c
	"), %%mm3\n\t"
		"       pfsub  56(%%"
	REG_c
	"), %%mm2\n\t"
		"       pfmul 120(%%"
	REG_b
	"), %%mm3\n\t"
		"       pfmul 120(%%"
	REG_b
	"), %%mm2\n\t"
		"       movq    %%mm2, %%mm1\n\t"

		"       pfadd  56(%%"
	REG_c
	"), %%mm1\n\t"
		"       pfadd  60(%%"
	REG_c
	"), %%mm1\n\t"
		"       movq    %%mm1, %%mm0\n\t"

		"       pfadd  48(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfadd  52(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfadd   %%mm3, %%mm1\n\t"
		"       punpckldq %%mm2, %%mm1\n\t"
		"       pfadd   %%mm3, %%mm2\n\t"
		"       punpckldq %%mm2, %%mm0\n\t"
		"       movq    %%mm1, 56(%%"
	REG_d
	")\n\t"
		"       movq    %%mm0, 48(%%"
	REG_d
	")\n\t"

		/*---*/

		"       movd   92(%%"
	REG_c
	"), %%mm1\n\t"
		"       pfsub  88(%%"
	REG_c
	"), %%mm1\n\t"
		"       pfmul 120(%%"
	REG_b
	"), %%mm1\n\t"
		"       movd   %%mm1, 92(%%"
	REG_d
	")\n\t"
		"       pfadd  92(%%"
	REG_c
	"), %%mm1\n\t"
		"       pfadd  88(%%"
	REG_c
	"), %%mm1\n\t"
		"       movq   %%mm1, %%mm0\n\t"

		"       pfadd  80(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfadd  84(%%"
	REG_c
	"), %%mm0\n\t"
		"       movd   %%mm0, 80(%%"
	REG_d
	")\n\t"

		"       movd   80(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfsub  84(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfmul 120(%%"
	REG_b
	"), %%mm0\n\t"
		"       pfadd  %%mm0, %%mm1\n\t"
		"       pfadd  92(%%"
	REG_d
	"), %%mm0\n\t"
		"       punpckldq %%mm1, %%mm0\n\t"
		"       movq   %%mm0, 84(%%"
	REG_d
	")\n\t"

		"       movq    96(%%"
	REG_c
	"), %%mm0\n\t"
		"       movq    %%mm0, %%mm1\n\t"
		"       pxor    %%mm7, %%mm1\n\t"
		"       pfacc   %%mm1, %%mm0\n\t"
		"       pfmul   %%mm6, %%mm0\n\t"
		"       movq    %%mm0, 96(%%"
	REG_d
	")\n\t"

		"       movd  108(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfsub 104(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfmul 120(%%"
	REG_b
	"), %%mm0\n\t"
		"       movd  %%mm0, 108(%%"
	REG_d
	")\n\t"
		"       pfadd 104(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfadd 108(%%"
	REG_c
	"), %%mm0\n\t"
		"       movd  %%mm0, 104(%%"
	REG_d
	")\n\t"

		"       movd  124(%%"
	REG_c
	"), %%mm1\n\t"
		"       pfsub 120(%%"
	REG_c
	"), %%mm1\n\t"
		"       pfmul 120(%%"
	REG_b
	"), %%mm1\n\t"
		"       movd  %%mm1, 124(%%"
	REG_d
	")\n\t"
		"       pfadd 120(%%"
	REG_c
	"), %%mm1\n\t"
		"       pfadd 124(%%"
	REG_c
	"), %%mm1\n\t"
		"       movq  %%mm1, %%mm0\n\t"

		"       pfadd 112(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfadd 116(%%"
	REG_c
	"), %%mm0\n\t"
		"       movd  %%mm0, 112(%%"
	REG_d
	")\n\t"

		"       movd  112(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfsub 116(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfmul 120(%%"
	REG_b
	"), %%mm0\n\t"
		"       pfadd %%mm0,%%mm1\n\t"
		"       pfadd 124(%%"
	REG_d
	"), %%mm0\n\t"
		"       punpckldq %%mm1, %%mm0\n\t"
		"       movq  %%mm0, 116(%%"
	REG_d
	")\n\t"

		// this code is broken, there is nothing modifying the z flag above.
#if 0
		"       jnz .L01\n\t"

		/* Phase 7*/
		/* Code below is coded in scalar mode. Should be optimized */

		"       movd      (%%"REG_c"), %%mm0\n\t"
		"       pfadd    4(%%"REG_c"), %%mm0\n\t"
		"       movd     %%mm0, 1024(%%"REG_S")\n\t"

		"       movd      (%%"REG_c"), %%mm0\n\t"
		"       pfsub    4(%%"REG_c"), %%mm0\n\t"
		"       pfmul  120(%%"REG_b"), %%mm0\n\t"
		"       movd      %%mm0, (%%"REG_S")\n\t"
		"       movd      %%mm0, (%%"REG_D")\n\t"

		"       movd   12(%%"REG_c"), %%mm0\n\t"
		"       pfsub   8(%%"REG_c"), %%mm0\n\t"
		"       pfmul 120(%%"REG_b"), %%mm0\n\t"
		"       movd    %%mm0, 512(%%"REG_D")\n\t"
		"       pfadd   12(%%"REG_c"), %%mm0\n\t"
		"       pfadd   8(%%"REG_c"), %%mm0\n\t"
		"       movd    %%mm0, 512(%%"REG_S")\n\t"

		"       movd   16(%%"REG_c"), %%mm0\n\t"
		"       pfsub  20(%%"REG_c"), %%mm0\n\t"
		"       pfmul 120(%%"REG_b"), %%mm0\n\t"
		"       movq    %%mm0, %%mm3\n\t"

		"       movd   28(%%"REG_c"), %%mm0\n\t"
		"       pfsub  24(%%"REG_c"), %%mm0\n\t"
		"       pfmul 120(%%"REG_b"), %%mm0\n\t"
		"       movd    %%mm0, 768(%%"REG_D")\n\t"
		"       movq    %%mm0, %%mm2\n\t"

		"       pfadd  24(%%"REG_c"), %%mm0\n\t"
		"       pfadd  28(%%"REG_c"), %%mm0\n\t"
		"       movq    %%mm0, %%mm1\n\t"

		"       pfadd  16(%%"REG_c"), %%mm0\n\t"
		"       pfadd  20(%%"REG_c"), %%mm0\n\t"
		"       movd   %%mm0, 768(%%"REG_S")\n\t"
		"       pfadd  %%mm3, %%mm1\n\t"
		"       movd   %%mm1, 256(%%"REG_S")\n\t"
		"       pfadd  %%mm3, %%mm2\n\t"
		"       movd   %%mm2, 256(%%"REG_D")\n\t"

		/* Phase 8*/

		"       movq   32(%%"REG_d"), %%mm0\n\t"
		"       movq   48(%%"REG_d"), %%mm1\n\t"
		"       pfadd  48(%%"REG_d"), %%mm0\n\t"
		"       pfadd  40(%%"REG_d"), %%mm1\n\t"
		"       movd   %%mm0, 896(%%"REG_S")\n\t"
		"       movd   %%mm1, 640(%%"REG_S")\n\t"
		"       psrlq  $32, %%mm0\n\t"
		"       psrlq  $32, %%mm1\n\t"
		"       movd   %%mm0, 128(%%"REG_D")\n\t"
		"       movd   %%mm1, 384(%%"REG_D")\n\t"

		"       movd   40(%%"REG_d"), %%mm0\n\t"
		"       pfadd  56(%%"REG_d"), %%mm0\n\t"
		"       movd   %%mm0, 384(%%"REG_S")\n\t"

		"       movd   56(%%"REG_d"), %%mm0\n\t"
		"       pfadd  36(%%"REG_d"), %%mm0\n\t"
		"       movd   %%mm0, 128(%%"REG_S")\n\t"

		"       movd   60(%%"REG_d"), %%mm0\n\t"
		"       movd   %%mm0, 896(%%"REG_D")\n\t"
		"       pfadd  44(%%"REG_d"), %%mm0\n\t"
		"       movd   %%mm0, 640(%%"REG_D")\n\t"

		"       movq   96(%%"REG_d"), %%mm0\n\t"
		"       movq   112(%%"REG_d"), %%mm2\n\t"
		"       movq   104(%%"REG_d"), %%mm4\n\t"
		"       pfadd  112(%%"REG_d"), %%mm0\n\t"
		"       pfadd  104(%%"REG_d"), %%mm2\n\t"
		"       pfadd  120(%%"REG_d"), %%mm4\n\t"
		"       movq   %%mm0, %%mm1\n\t"
		"       movq   %%mm2, %%mm3\n\t"
		"       movq   %%mm4, %%mm5\n\t"
		"       pfadd  64(%%"REG_d"), %%mm0\n\t"
		"       pfadd  80(%%"REG_d"), %%mm2\n\t"
		"       pfadd  72(%%"REG_d"), %%mm4\n\t"
		"       movd   %%mm0, 960(%%"REG_S")\n\t"
		"       movd   %%mm2, 704(%%"REG_S")\n\t"
		"       movd   %%mm4, 448(%%"REG_S")\n\t"
		"       psrlq  $32, %%mm0\n\t"
		"       psrlq  $32, %%mm2\n\t"
		"       psrlq  $32, %%mm4\n\t"
		"       movd   %%mm0, 64(%%"REG_D")\n\t"
		"       movd   %%mm2, 320(%%"REG_D")\n\t"
		"       movd   %%mm4, 576(%%"REG_D")\n\t"
		"       pfadd  80(%%"REG_d"), %%mm1\n\t"
		"       pfadd  72(%%"REG_d"), %%mm3\n\t"
		"       pfadd  88(%%"REG_d"), %%mm5\n\t"
		"       movd   %%mm1, 832(%%"REG_S")\n\t"
		"       movd   %%mm3, 576(%%"REG_S")\n\t"
		"       movd   %%mm5, 320(%%"REG_S")\n\t"
		"       psrlq  $32, %%mm1\n\t"
		"       psrlq  $32, %%mm3\n\t"
		"       psrlq  $32, %%mm5\n\t"
		"       movd   %%mm1, 192(%%"REG_D")\n\t"
		"       movd   %%mm3, 448(%%"REG_D")\n\t"
		"       movd   %%mm5, 704(%%"REG_D")\n\t"

		"       movd   120(%%"REG_d"), %%mm0\n\t"
		"       pfadd  100(%%"REG_d"), %%mm0\n\t"
		"       movq   %%mm0, %%mm1\n\t"
		"       pfadd  88(%%"REG_d"), %%mm0\n\t"
		"       movd   %%mm0, 192(%%"REG_S")\n\t"
		"       pfadd  68(%%"REG_d"), %%mm1\n\t"
		"       movd   %%mm1, 64(%%"REG_S")\n\t"

		"       movd  124(%%"REG_d"), %%mm0\n\t"
		"       movd  %%mm0, 960(%%"REG_D")\n\t"
		"       pfadd  92(%%"REG_d"), %%mm0\n\t"
		"       movd  %%mm0, 832(%%"REG_D")\n\t"

		"       jmp     .L_bye\n\t"
		".L01:  \n\t"
#endif
		/* Phase 9*/

		"       movq    (%%"
	REG_c
	"), %%mm0\n\t"
		"       movq    %%mm0, %%mm1\n\t"
		"       pxor    %%mm7, %%mm1\n\t"
		"       pfacc   %%mm1, %%mm0\n\t"
		"       pfmul   %%mm6, %%mm0\n\t"
		"       pf2iw   %%mm0, %%mm0\n\t"
		"       movd    %%mm0, %%"
	REG_a
	"\n\t"
		"       movw    %%ax, 512(%%"
	REG_S
	")\n\t"
		"       psrlq   $32, %%mm0\n\t"
		"       movd    %%mm0, %%"
	REG_a
	"\n\t"
		"       movw    %%ax, (%%"
	REG_S
	")\n\t"

		"       movd    12(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfsub    8(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfmul  120(%%"
	REG_b
	"), %%mm0\n\t"
		"       pf2iw    %%mm0, %%mm7\n\t"
		"       movd     %%mm7, %%"
	REG_a
	"\n\t"
		"       movw     %%ax, 256(%%"
	REG_D
	")\n\t"
		"       pfadd   12(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfadd    8(%%"
	REG_c
	"), %%mm0\n\t"
		"       pf2iw    %%mm0, %%mm0\n\t"
		"       movd     %%mm0, %%"
	REG_a
	"\n\t"
		"       movw     %%ax, 256(%%"
	REG_S
	")\n\t"

		"       movd   16(%%"
	REG_c
	"), %%mm3\n\t"
		"       pfsub  20(%%"
	REG_c
	"), %%mm3\n\t"
		"       pfmul  120(%%"
	REG_b
	"), %%mm3\n\t"
		"       movq   %%mm3, %%mm2\n\t"

		"       movd   28(%%"
	REG_c
	"), %%mm2\n\t"
		"       pfsub  24(%%"
	REG_c
	"), %%mm2\n\t"
		"       pfmul 120(%%"
	REG_b
	"), %%mm2\n\t"
		"       movq   %%mm2, %%mm1\n\t"

		"       pf2iw  %%mm2, %%mm7\n\t"
		"       movd   %%mm7, %%"
	REG_a
	"\n\t"
		"       movw   %%ax, 384(%%"
	REG_D
	")\n\t"

		"       pfadd  24(%%"
	REG_c
	"), %%mm1\n\t"
		"       pfadd  28(%%"
	REG_c
	"), %%mm1\n\t"
		"       movq   %%mm1, %%mm0\n\t"

		"       pfadd  16(%%"
	REG_c
	"), %%mm0\n\t"
		"       pfadd  20(%%"
	REG_c
	"), %%mm0\n\t"
		"       pf2iw  %%mm0, %%mm0\n\t"
		"       movd   %%mm0, %%"
	REG_a
	"\n\t"
		"       movw   %%ax, 384(%%"
	REG_S
	")\n\t"
		"       pfadd  %%mm3, %%mm1\n\t"
		"       pf2iw  %%mm1, %%mm1\n\t"
		"       movd   %%mm1, %%"
	REG_a
	"\n\t"
		"       movw   %%ax, 128(%%"
	REG_S
	")\n\t"
		"       pfadd  %%mm3, %%mm2\n\t"
		"       pf2iw  %%mm2, %%mm2\n\t"
		"       movd   %%mm2, %%"
	REG_a
	"\n\t"
		"       movw   %%ax, 128(%%"
	REG_D
	")\n\t"

		/* Phase 10*/

		"       movq    32(%%"
	REG_d
	"), %%mm0\n\t"
		"       movq    48(%%"
	REG_d
	"), %%mm1\n\t"
		"       pfadd   48(%%"
	REG_d
	"), %%mm0\n\t"
		"       pfadd   40(%%"
	REG_d
	"), %%mm1\n\t"
		"       pf2iw   %%mm0, %%mm0\n\t"
		"       pf2iw   %%mm1, %%mm1\n\t"
		"       movd    %%mm0, %%"
	REG_a
	"\n\t"
		"       movd    %%mm1, %%"
	REG_c
	"\n\t"
		"       movw    %%ax, 448(%%"
	REG_S
	")\n\t"
		"       movw    %%cx, 320(%%"
	REG_S
	")\n\t"
		"       psrlq   $32, %%mm0\n\t"
		"       psrlq   $32, %%mm1\n\t"
		"       movd    %%mm0, %%"
	REG_a
	"\n\t"
		"       movd    %%mm1, %%"
	REG_c
	"\n\t"
		"       movw    %%ax, 64(%%"
	REG_D
	")\n\t"
		"       movw    %%cx, 192(%%"
	REG_D
	")\n\t"

		"       movd   40(%%"
	REG_d
	"), %%mm3\n\t"
		"       movd   56(%%"
	REG_d
	"), %%mm4\n\t"
		"       movd   60(%%"
	REG_d
	"), %%mm0\n\t"
		"       movd   44(%%"
	REG_d
	"), %%mm2\n\t"
		"       movd  120(%%"
	REG_d
	"), %%mm5\n\t"
		"       punpckldq %%mm4, %%mm3\n\t"
		"       punpckldq 124(%%"
	REG_d
	"), %%mm0\n\t"
		"       pfadd 100(%%"
	REG_d
	"), %%mm5\n\t"
		"       punpckldq 36(%%"
	REG_d
	"), %%mm4\n\t"
		"       punpckldq 92(%%"
	REG_d
	"), %%mm2\n\t"
		"       movq  %%mm5, %%mm6\n\t"
		"       pfadd  %%mm4, %%mm3\n\t"
		"       pf2iw  %%mm0, %%mm1\n\t"
		"       pf2iw  %%mm3, %%mm3\n\t"
		"       pfadd  88(%%"
	REG_d
	"), %%mm5\n\t"
		"       movd   %%mm1, %%"
	REG_a
	"\n\t"
		"       movd   %%mm3, %%"
	REG_c
	"\n\t"
		"       movw   %%ax, 448(%%"
	REG_D
	")\n\t"
		"       movw   %%cx, 192(%%"
	REG_S
	")\n\t"
		"       pf2iw  %%mm5, %%mm5\n\t"
		"       psrlq  $32, %%mm1\n\t"
		"       psrlq  $32, %%mm3\n\t"
		"       movd   %%mm5, %%"
	REG_b
	"\n\t"
		"       movd   %%mm1, %%"
	REG_a
	"\n\t"
		"       movd   %%mm3, %%"
	REG_c
	"\n\t"
		"       movw   %%bx, 96(%%"
	REG_S
	")\n\t"
		"       movw   %%ax, 480(%%"
	REG_D
	")\n\t"
		"       movw   %%cx, 64(%%"
	REG_S
	")\n\t"
		"       pfadd  %%mm2, %%mm0\n\t"
		"       pf2iw  %%mm0, %%mm0\n\t"
		"       movd   %%mm0, %%"
	REG_a
	"\n\t"
		"       pfadd  68(%%"
	REG_d
	"), %%mm6\n\t"
		"       movw   %%ax, 320(%%"
	REG_D
	")\n\t"
		"       psrlq  $32, %%mm0\n\t"
		"       pf2iw  %%mm6, %%mm6\n\t"
		"       movd   %%mm0, %%"
	REG_a
	"\n\t"
		"       movd   %%mm6, %%"
	REG_b
	"\n\t"
		"       movw   %%ax, 416(%%"
	REG_D
	")\n\t"
		"       movw   %%bx, 32(%%"
	REG_S
	")\n\t"

		"       movq   96(%%"
	REG_d
	"), %%mm0\n\t"
		"       movq  112(%%"
	REG_d
	"), %%mm2\n\t"
		"       movq  104(%%"
	REG_d
	"), %%mm4\n\t"
		"       pfadd %%mm2, %%mm0\n\t"
		"       pfadd %%mm4, %%mm2\n\t"
		"       pfadd 120(%%"
	REG_d
	"), %%mm4\n\t"
		"       movq  %%mm0, %%mm1\n\t"
		"       movq  %%mm2, %%mm3\n\t"
		"       movq  %%mm4, %%mm5\n\t"
		"       pfadd  64(%%"
	REG_d
	"), %%mm0\n\t"
		"       pfadd  80(%%"
	REG_d
	"), %%mm2\n\t"
		"       pfadd  72(%%"
	REG_d
	"), %%mm4\n\t"
		"       pf2iw  %%mm0, %%mm0\n\t"
		"       pf2iw  %%mm2, %%mm2\n\t"
		"       pf2iw  %%mm4, %%mm4\n\t"
		"       movd   %%mm0, %%"
	REG_a
	"\n\t"
		"       movd   %%mm2, %%"
	REG_c
	"\n\t"
		"       movd   %%mm4, %%"
	REG_b
	"\n\t"
		"       movw   %%ax, 480(%%"
	REG_S
	")\n\t"
		"       movw   %%cx, 352(%%"
	REG_S
	")\n\t"
		"       movw   %%bx, 224(%%"
	REG_S
	")\n\t"
		"       psrlq  $32, %%mm0\n\t"
		"       psrlq  $32, %%mm2\n\t"
		"       psrlq  $32, %%mm4\n\t"
		"       movd   %%mm0, %%"
	REG_a
	"\n\t"
		"       movd   %%mm2, %%"
	REG_c
	"\n\t"
		"       movd   %%mm4, %%"
	REG_b
	"\n\t"
		"       movw   %%ax, 32(%%"
	REG_D
	")\n\t"
		"       movw   %%cx, 160(%%"
	REG_D
	")\n\t"
		"       movw   %%bx, 288(%%"
	REG_D
	")\n\t"
		"       pfadd  80(%%"
	REG_d
	"), %%mm1\n\t"
		"       pfadd  72(%%"
	REG_d
	"), %%mm3\n\t"
		"       pfadd  88(%%"
	REG_d
	"), %%mm5\n\t"
		"       pf2iw  %%mm1, %%mm1\n\t"
		"       pf2iw  %%mm3, %%mm3\n\t"
		"       pf2iw  %%mm5, %%mm5\n\t"
		"       movd   %%mm1, %%"
	REG_a
	"\n\t"
		"       movd   %%mm3, %%"
	REG_c
	"\n\t"
		"       movd   %%mm5, %%"
	REG_b
	"\n\t"
		"       movw   %%ax, 416(%%"
	REG_S
	")\n\t"
		"       movw   %%cx, 288(%%"
	REG_S
	")\n\t"
		"       movw   %%bx, 160(%%"
	REG_S
	")\n\t"
		"       psrlq  $32, %%mm1\n\t"
		"       psrlq  $32, %%mm3\n\t"
		"       psrlq  $32, %%mm5\n\t"
		"       movd   %%mm1, %%"
	REG_a
	"\n\t"
		"       movd   %%mm3, %%"
	REG_c
	"\n\t"
		"       movd   %%mm5, %%"
	REG_b
	"\n\t"
		"       movw   %%ax, 96(%%"
	REG_D
	")\n\t"
		"       movw   %%cx, 224(%%"
	REG_D
	")\n\t"
		"       movw   %%bx, 352(%%"
	REG_D
	")\n\t"

		"       movsw\n\t"

		".L_bye:\n\t"
		"       femms\n\t"
	:
	:
	"m"(a), "m"(b), "m"(c), "m"(tmp[0])
	:
	"memory", "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi"
	)
}
