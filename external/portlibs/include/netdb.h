/*   netdb.h

	netport
 */

#ifndef _NETDB_H_
#define _NETDB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>

struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  u16     h_addrtype;     /* host address type */
  u16     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses from name server */
  #define h_addr  h_addr_list[0]
};


struct netent {
	char     *n_name  ;		/* Official, fully-qualified (including the domain) name of the host.  */
	char    **n_aliases; 	/*  A pointer to an array of pointers to
							alternative network names, terminated by a
							null pointer. */
	int       n_addrtype;  	/* The address type of the network. */
	uint32_t  n_net;       	/* The network number, in host byte order. */
};

struct protoent {
	char   *p_name;     /* Official name of the protocol. */
	char  **p_aliases;  /* A pointer to an array of pointers to
						alternative protocol names, terminated by
						a null pointer. */
	int     p_proto;    /* The protocol number. */
};

struct servent {
	char   *s_name;     	/* Official name of the service. */
	char  **s_aliases;  	/* A pointer to an array of pointers to
						alternative service names, terminated by
						a null pointer. */
	int     s_port; 		/*The port number at which the service
						resides, in network byte order. */
	char   *s_proto; 	/* The name of the protocol to use when
						contacting the service. */
};

struct addrinfo {
    int     ai_flags;
    int     ai_family;
    int     ai_socktype;
    int     ai_protocol;
    size_t  ai_addrlen;
    struct sockaddr *ai_addr;
    char   *ai_canonname;
    struct addrinfo *ai_next;
};

#if(0)
#define	NETDB_SUCCESS   0 /* no problem */
#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */
#endif

void              endhostent(void);
void              endnetent(void);
void              endprotoent(void);
void              endservent(void);
void              freeaddrinfo(struct addrinfo *);
const char       *gai_strerror(int);
int               getaddrinfo(const char *__restrict__, const char *__restrict__,
                      const struct addrinfo *__restrict__,
                      struct addrinfo **__restrict__);
struct hostent   *gethostbyaddr(const void *, socklen_t, int);
struct hostent   *gethostbyname(const char *);
struct hostent   *gethostent(void);
int               getnameinfo(const struct sockaddr *__restrict__, socklen_t,
                      char *__restrict__, socklen_t, char *__restrict__,
                      socklen_t, int);
struct netent    *getnetbyaddr(uint32_t, int);
struct netent    *getnetbyname(const char *);
struct netent    *getnetent(void);
struct protoent  *getprotobyname(const char *);
struct protoent  *getprotobynumber(int);
struct protoent  *getprotoent(void);
struct servent   *getservbyname(const char *, const char *);
struct servent   *getservbyport(int, const char *);
struct servent   *getservent(void);
void              sethostent(int);
void              setnetent(int);
void              setprotoent(int);
void              setservent(int);


#ifdef __cplusplus
};
#endif

#endif /* !_NETDB_H_ */

