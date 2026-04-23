/* arpa/inet.h

   netport
*/

#ifndef _ARPA_INET_H
#define _ARPA_INET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <netinet/in.h>

#ifndef htons
#define htons(x) (x)
#endif
#ifndef ntohs
#define ntohs(x) (x)
#endif
#ifndef htonl
#define htonl(x) (x)
#endif
#ifndef ntohl
#define ntohl(x) (x)
#endif

//int inet_aton (__const char *__cp, struct in_addr *__inp);
in_addr_t    inet_addr(const char *);
//char        *inet_ntoa(struct in_addr);
const char  *inet_ntop(int, const void *__restrict__, char *__restrict__,
                 socklen_t);
int          inet_pton(int, const char *__restrict__, void *__restrict__);
//int inet_aton(const char *cp, struct in_addr *inp);



#ifdef __cplusplus
};
#endif

#endif /* _ARPA_INET_H */
