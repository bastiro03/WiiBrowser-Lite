/*
 * Network layer for MPlayer
 *
 * Copyright (C) 2001 Bertrand Baudet <bertrand_baudet@yahoo.com>
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPLAYER_NETWORK_H
#define MPLAYER_NETWORK_H

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "config.h"
#if !defined(GEKKO)
#if !HAVE_WINSOCK2_H
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#else
#include <errno.h>
#include <network.h>

#define MSG_OOB 0x01

static inline int _net_result(s32 ret)
{
	if (ret < 0) errno = -ret;
	return ret < 0 ? SOCKET_ERROR : ret;
}

#define socket(domain, type, protocol) \
	_net_result(net_socket(domain, type, IPPROTO_IP))
#define bind(sockfd, my_addr, addrlen) \
	_net_result(net_bind(sockfd, my_addr, addrlen))
#define connect(sockfd, serv_addr, addrlen) \
	_net_result(net_connect(sockfd, serv_addr, addrlen))
#define send(s, buf, len, flags) \
	_net_result(net_send(s, buf, len, flags))
#define sendto(s, buf, len, flags, to, tolen) \
	_net_result(net_sendto(s, buf, len, flags, to, tolen))
#define recv(s, buf, len, flags) \
	_net_result(net_recv(s, buf, len, flags))
#define recvfrom(s, buf, len, flags, from, fromlen) \
	_net_result(net_recvfrom(s, buf, len, flags, from, fromlen))
#define closesocket(sockfd) \
	_net_result(net_close(sockfd))
#define select(nfds, readfds, writefds, exceptfds, timeout) \
	_net_result(net_select(nfds, readfds, writefds, exceptfds, timeout))
#define setsockopt(s, level, optname, optval, optlen) \
	_net_result(net_setsockopt(s, level, optname, optval, optlen))
#define gethostbyname(name) net_gethostbyname(name)
#endif

#include "stream.h"
#include "url.h"
#include "http.h"

#ifdef MSG_NOSIGNAL
#define DEFAULT_SEND_FLAGS MSG_NOSIGNAL
#else
#define DEFAULT_SEND_FLAGS 0
#endif

#if !HAVE_CLOSESOCKET
#define closesocket close
#endif
#if !HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

#define BUFFER_SIZE		2048

typedef struct {
	const char *mime_type;
	int demuxer_type;
} mime_struct_t;

extern const mime_struct_t mime_type_table[];

extern char *cookies_file;
extern char *network_password;
extern char *network_referrer;
extern char *network_useragent;
extern char *network_username;

extern int   network_bandwidth;
extern int   network_cookies_enabled;
extern int   network_ipv4_only_proxy;

streaming_ctrl_t *streaming_ctrl_new(void);
int streaming_bufferize( streaming_ctrl_t *streaming_ctrl, char *buffer, int size);

int nop_streaming_read( int fd, char *buffer, int size, streaming_ctrl_t *stream_ctrl );
int nop_streaming_seek( int fd, off_t pos, streaming_ctrl_t *stream_ctrl );
void streaming_ctrl_free( streaming_ctrl_t *streaming_ctrl );

int http_send_request(URL_t *url, off_t pos);
HTTP_header_t *http_read_response(int fd);

int http_authenticate(HTTP_header_t *http_hdr, URL_t *url, int *auth_retry);
URL_t* check4proxies(URL_t *url);

void fixup_network_stream_cache(stream_t *stream);
int http_seek(stream_t *stream, off_t pos);

#endif /* MPLAYER_NETWORK_H */
