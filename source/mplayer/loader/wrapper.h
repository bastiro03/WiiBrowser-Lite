#ifndef MPLAYER_WRAPPER_H
#define MPLAYER_WRAPPER_H

#include <inttypes.h>

using reg386_t = struct
{
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

using wrapper_func_t = int(*)(void* stack_base, int stack_size, reg386_t* reg, uint32_t* flags);

extern wrapper_func_t report_entry, report_ret;

extern void (*wrapper_target)(void);

int wrapper(void);
int null_call(void);

#endif /* MPLAYER_WRAPPER_H */
