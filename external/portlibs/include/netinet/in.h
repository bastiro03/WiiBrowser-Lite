/* netinet/in.h

   netport
   */

#ifndef _NETINET_IN_H
#define _NETINET_IN_H

#include <inttypes.h>

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

#include <sys/socket.h>

struct in_addr {
  in_addr_t s_addr;
};

struct sockaddr_in {
  u8 sin_len;
  sa_family_t sin_family;
  in_port_t sin_port;
  struct in_addr sin_addr;
  s8 sin_zero[8];
};

#define IPPROTO_IP			0
#define IPPROTO_TCP			6
#define IPPROTO_UDP			17

#define INADDR_ANY			0
#define INADDR_BROADCAST	0xffffffff
#define INADDR_NONE ((unsigned long) -1)

#define INET_ADDRSTRLEN		16

#include <arpa/inet.h>

#endif /* _NETINET_IN_H */
