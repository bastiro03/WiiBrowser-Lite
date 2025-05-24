/* sys/select.h

   netport
*/

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Get fd_set, and macros like FD_SET */
#include <sys/types.h>
#include <gctypes.h>

/* Get definition of timeval.  */
#include <sys/time.h>
#include <time.h>

/* Get definition of sigset_t. */
#include <signal.h>

/*#undef  FD_SETSIZE
#define FD_SETSIZE		16
#define FD_SET(n, p)		((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
#define FD_CLR(n, p)		((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
#define FD_ISSET(n,p)		((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
#define FD_ZERO(p)		memset((void*)(p),0,sizeof(*(p)))*/

/*typedef struct fd_set {
	u8 fd_bits [(FD_SETSIZE+7)/8];
} fd_set;*/

struct pollsd {
	s32 socket;
	u32 events;
	u32 revents;
};

int  pselect(int, fd_set *__restrict__, fd_set *__restrict__, fd_set *__restrict__,
         const struct timespec *__restrict__, const sigset_t *__restrict__);
int  select(int, fd_set *__restrict__, fd_set *__restrict__, fd_set *__restrict__,
         struct timeval *__restrict__);
s32 poll(struct pollsd *sds,s32 nsds,s32 timeout);

#ifdef __cplusplus
};
#endif

#endif /* sys/select.h */
