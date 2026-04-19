/*
**  Created by gave92
**
**  Copyright (C) 2012 - 2013
**  ==================
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, version 2.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**  History:
**   20-11-2008  Start programming
**   30-11-2008  Added google analysic http request
**   07-12-2008  Add user_var parameter in google analysic http request
**   30-12-2008  Added store highscore on internet.
**   06-01-2009  Google analysic function improved.
**   15-01-2009  Add http chunked frame support.
**   21-01-2009  Create one unique cookie during startup.
**   26-01-2009  Increase internal buffersize to 8kB.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config.h"
#include "httplib.h"

#include "main.h"
#include "menu.h"
#include "filelist.h"

#include "wbl/platform.h"
#if WBL_HAS_HTTPS
#include "psa/crypto.h"   /* psa_crypto_init for TLS 1.3 */
#endif

extern "C"
{
#include "urlcode.h"
}

// -----------------------------------------------------------
// VARIABLES
// -----------------------------------------------------------

enum
{
	HEAD,
	POST,
	GET,
	REQUESTS
};

enum
{
	DFOUND = -1,
	DCOMPLETE = -2,
	DSTOPPED = -3,
};

const char Agents[MAXAGENTS][256] =
	{
		"",
		"libcurl-agent/1.0",
		"Mozilla/5.0 (Nintendo Wii; U; WiiBrowser rev37) like Gecko Firefox/4.0",
		"Mozilla/5.0 (Windows NT 6.2; WOW64; rv:16.0.1) Gecko/20121011 Firefox/16.0.1",
		"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0)",
		"Mozilla/5.0 (Linux; U; Android 1.1; en-gb; dream) AppleWebKit/525.10+ (KHTML, like Gecko) Version/3.0.4 Mobile Safari/523.12.2",
		/* watchOS 8.8.1 embedded WebKit on Apple Watch Series 3.
		 *
		 * watchOS identifies as iPhone in WebKit for site compatibility,
		 * paired with the iOS build it shipped alongside (iOS 15.6.1,
		 * build 19G82). Crucially we OMIT both "Version/X" and "Safari/X"
		 * — embedded WebKit (link previews in Messages/Mail) is not
		 * full Safari, and many sites serve a stripped-down layout to
		 * this signature to keep JS/DOM bundles small. That matches
		 * the Wii's memory budget (~88MB total) far better than iOS 17
		 * on iPhone 15 which triggers multi-MB JS bundles. */
		"Mozilla/5.0 (iPhone; CPU iPhone OS 15_6_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/19G82",
};

const struct block emptyblock = {nullptr, 0};
static int firstRun = true;
static char cacert_path[256] = {0};

// Last download error, captured from curl_easy_perform() / getinfo() so
// the "Failed" UI modal can show something more actionable than just
// "Failed". Written from getrequest/postrequest, read via
// GetLastDownloadError().
static char last_dl_error[256] = "no error recorded";

static void record_dl_error(CURL *curl_handle, int curl_res)
{
	const char *msg = (curl_res != 0) ? curl_easy_strerror((CURLcode)curl_res) : "empty response";
	long http_code = 0;
	char *effective_url = nullptr;
	if (curl_handle)
	{
		curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
		curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &effective_url);
	}
	snprintf(last_dl_error, sizeof(last_dl_error),
	         "curl: %s\nHTTP: %ld\nURL: %s",
	         msg, http_code, effective_url ? effective_url : "(unknown)");
}

const char *GetLastDownloadError(void)
{
	return last_dl_error;
}

// -----------------------------------------------------------
// SSL CERTIFICATE SETUP
// -----------------------------------------------------------

#if WBL_HAS_HTTPS
/* One-time PSA crypto init. mbedTLS 3.6 routes TLS 1.3 signature and
 * key-derivation operations through PSA. Calling psa_crypto_init()
 * before the first mbedtls_ssl_handshake() is mandatory - without it
 * our ClientHello gets past the wire but the server rejects us at
 * CertificateVerify (unsupported signature scheme) or the local
 * signature computation fails with NOT_SUPPORTED. */
static void psa_init_once(void)
{
	static bool done = false;
	if (done) return;
	psa_status_t s = psa_crypto_init();
	if (s != PSA_SUCCESS) {
		printf("psa_crypto_init failed: %d\n", (int)s);
	}
	done = true;
}
#endif

static void setup_cacert()
{
#if WBL_HAS_HTTPS
	psa_init_once();
#endif
	if (cacert_path[0] != 0)
		return; // Already initialized

	// Write the embedded CA certificate bundle to SD card
	snprintf(cacert_path, sizeof(cacert_path), "%s/cacert.pem", Settings.AppPath);

	FILE *f = fopen(cacert_path, "wb");
	if (f)
	{
		fwrite(cacert_pem, 1, cacert_pem_size, f);
		fclose(f);
	}
	else
	{
		// Fallback: try writing to /tmp
		snprintf(cacert_path, sizeof(cacert_path), "/tmp/cacert.pem");
		f = fopen(cacert_path, "wb");
		if (f)
		{
			fwrite(cacert_pem, 1, cacert_pem_size, f);
			fclose(f);
		}
		else
		{
			cacert_path[0] = 0; // Failed to write cert
		}
	}
}

// -----------------------------------------------------------
// DATA HANDLING
// -----------------------------------------------------------

struct HeaderStruct
{
	char *memory;
	u32 size;
	bool download;
	char filename[256];
};

struct MemoryStruct
{
	char *memory;
	u32 size;
};

int parseline(HeaderStruct *mem, size_t realsize);
void fillstruct(CURL *handle, HeaderStruct *head, struct block *dest);
bool mustdownload(char *content);

int close_callback(void *clientp, curl_socket_t item)
{
	return net_close(item);
}

// Dolphin's IOS network emulation doesn't support IPPROTO_TCP (6);
// net_socket(AF_INET, SOCK_STREAM, 6) returns -EPROTONOSUPPORT (-123).
// Use IPPROTO_IP (0) which works on both real hardware and Dolphin.
static curl_socket_t opensocket_callback(void *clientp,
                                         curlsocktype purpose,
                                         struct curl_sockaddr *address)
{
	(void)clientp;
	(void)purpose;
	// Force protocol=0 (IPPROTO_IP) instead of curl's default IPPROTO_TCP=6
	int fd = net_socket(address->family, address->socktype, 0);
	fprintf(stderr, "opensocket_callback: net_socket(%d, %d, 0) = %d\n",
	        address->family, address->socktype, fd);
	return fd;
}

// curl's bundled c-ares resolver doesn't work on Wii: it tries to read
// /etc/resolv.conf (absent), falls back to querying 127.0.0.1, and also
// creates DNS sockets with IPPROTO_TCP=6 which Dolphin's IOS rejects
// before our opensocket_callback runs. Bypass c-ares entirely by
// resolving via libogc's net_gethostbyname() (which uses IOS's configured
// DNS) and feeding the result to curl via CURLOPT_RESOLVE.
//
// Returned slist (if non-NULL) must be kept alive until the transfer
// completes, then freed with curl_slist_free_all().
struct curl_slist *wbl_build_resolve_list(const char *url)
{
	CURLU *u = curl_url();
	if (!u)
		return NULL;

	if (curl_url_set(u, CURLUPART_URL, url, 0) != CURLUE_OK)
	{
		curl_url_cleanup(u);
		return NULL;
	}

	char *host = NULL, *port = NULL;
	curl_url_get(u, CURLUPART_HOST, &host, 0);
	curl_url_get(u, CURLUPART_PORT, &port, CURLU_DEFAULT_PORT);

	struct curl_slist *list = NULL;
	if (host && port)
	{
		// Skip pre-resolution if host is already an IPv4 literal.
		bool is_ip = (host[0] >= '0' && host[0] <= '9');
		if (!is_ip)
		{
			struct hostent *he = net_gethostbyname(host);
			if (he && he->h_addr_list && he->h_addr_list[0])
			{
				const unsigned char *ip = (const unsigned char *)he->h_addr_list[0];
				// CURLOPT_RESOLVE requires a specific numeric port per
				// entry; there is NO wildcard port syntax. Add entries
				// for BOTH 80 and 443 so an HTTP→HTTPS redirect (the
				// common pattern for x.com, google.com, etc.) finds a
				// DNS entry for the new port without requiring another
				// net_gethostbyname call. Also add the URL's own port
				// if it's not 80/443 (explicit e.g. :8080).
				static const unsigned short common_ports[] = {80, 443, 0};
				unsigned long url_port = strtoul(port, NULL, 10);
				for (int i = 0; common_ports[i] != 0; i++)
				{
					char entry[256];
					snprintf(entry, sizeof(entry), "%s:%u:%u.%u.%u.%u",
					         host, (unsigned)common_ports[i],
					         ip[0], ip[1], ip[2], ip[3]);
					list = curl_slist_append(list, entry);
					fprintf(stderr, "wbl_build_resolve_list: %s\n", entry);
				}
				if (url_port != 80 && url_port != 443 &&
				    url_port > 0 && url_port < 65536)
				{
					char entry[256];
					snprintf(entry, sizeof(entry), "%s:%lu:%u.%u.%u.%u",
					         host, url_port, ip[0], ip[1], ip[2], ip[3]);
					list = curl_slist_append(list, entry);
					fprintf(stderr, "wbl_build_resolve_list: %s\n", entry);
				}
				fflush(stderr);
			}
			else
			{
				fprintf(stderr,
				        "wbl_build_resolve_list: net_gethostbyname(%s) failed\n",
				        host);
				fflush(stderr);
			}
		}
	}

	if (host) curl_free(host);
	if (port) curl_free(port);
	curl_url_cleanup(u);
	return list;
}

// httplib reuses one CURL easy handle across all page fetches, so a
// single static slist is fine here — freeing it at the top of each new
// request is safe because the previous request has already completed.
static struct curl_slist *httplib_resolve_list = NULL;

static void pre_resolve(CURL *handle, const char *url)
{
	if (httplib_resolve_list)
	{
		curl_slist_free_all(httplib_resolve_list);
		httplib_resolve_list = NULL;
	}
	httplib_resolve_list = wbl_build_resolve_list(url);
	if (httplib_resolve_list)
		curl_easy_setopt(handle, CURLOPT_RESOLVE, httplib_resolve_list);
}

/* Cap a single page body at 8 MiB. Wii has 88 MB total RAM, much of
 * which is already committed to GX framebuffers, fonts, app state,
 * and libcurl/mbedtls. A runaway page (chunked response with no
 * length, content spam) must not be allowed to OOM the whole app. */
#define WBL_MAX_PAGE_BYTES (8u * 1024u * 1024u)

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	auto mem = static_cast<struct MemoryStruct *>(userp);

	/* Reject pages that would exceed our cap. Returning 0 makes curl
	 * treat it as CURLE_WRITE_ERROR and the caller surfaces an error. */
	if (mem->size + realsize > WBL_MAX_PAGE_BYTES)
	{
		Debug("page exceeds WBL_MAX_PAGE_BYTES, aborting transfer\n");
		return 0;
	}

	char *grown = static_cast<char *>(realloc(mem->memory, mem->size + realsize + 1));
	if (grown == nullptr)
	{
		/* Previously called exit(EXIT_FAILURE) which killed the whole
		 * app on any transient alloc failure. Returning 0 aborts just
		 * the transfer (CURLE_WRITE_ERROR) so the UI can report an
		 * error and the user can continue browsing. */
		Debug("realloc failed in WriteMemoryCallback\n");
		return 0;
	}
	mem->memory = grown;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->memory[mem->size += realsize] = 0;
	return realsize;
}

static size_t writedata(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written = fwrite(ptr, size, nmemb, static_cast<FILE *>(stream));
	return written;
}

static size_t parseheader(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	auto mem = static_cast<struct HeaderStruct *>(userp);
	char *grown = static_cast<char *>(realloc(mem->memory, mem->size + realsize + 1));
	if (grown == nullptr)
	{
		/* See WriteMemoryCallback: graceful abort beats killing the app. */
		Debug("realloc failed in parseheader\n");
		return 0;
	}
	mem->memory = grown;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->memory[mem->size += realsize] = 0;
	return parseline(mem, realsize);
}

int parseline(HeaderStruct *mem, size_t realsize)
{
	/* Fix: upper bound was `<= mem->size` which touches one byte past the
	 * newly-added data (the trailing NUL). tolower(0)==0 so it's benign,
	 * but flag-and-fix so future refactors don't widen the off-by-one. */
	unsigned int i;
	for (i = mem->size - realsize; i < mem->size; i++)
		mem->memory[i] = tolower(mem->memory[i]);

	char *line = &mem->memory[mem->size - realsize];
	char buff[128];
	bzero(buff, sizeof(buff));

	if (!strncmp(line, "content-type", 12))
	{
		/* sscanf %s skips leading whitespace but stops at whitespace so
		 * trailing "\r\n" or trailing spaces before ';' do not end up
		 * in buff. Defensively cap to buff size-1 so a pathological
		 * Content-Type never overruns. */
		sscanf(line, "content-type: %127s", buff);
		findChr(buff, ';');

		if (mustdownload(buff))
			mem->download = true;
	}

	else if (!strncmp(line, "content-disposition", 19))
	{
		/* Was strcpy() which can overflow filename[256] if the header
		 * line is long (e.g. long RFC 5987 filename*=UTF-8''...). */
		snprintf(mem->filename, sizeof(mem->filename), "%s", line);
	}

	else if (!strncmp(line, "\r\n", 2))
		return (!mem->download) * realsize;
	return realsize;
}

// -----------------------------------------------------------
// SET HEADERS
// -----------------------------------------------------------

static int wbl_curl_debug_cb(CURL *handle, curl_infotype type,
                             char *data, size_t size, void *userptr)
{
	if (type == CURLINFO_TEXT) {
		fprintf(stderr, "[CURL] %.*s", (int)size, data);
		if (size > 0 && data[size-1] != '\n')
			fprintf(stderr, "\n");
		fflush(stderr);
	}
	return 0;
}

void setmainheaders(CURL *curl_handle, const char *url)
{
	if (!curl_handle)
		return;

	/* send all data to this function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* enable verbose debugging to diagnose DNS issues */
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_DEBUGFUNCTION, wbl_curl_debug_cb);

	/* set proxy if specified */
	if (validProxy())
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, Settings.Proxy);

	/* follow redirects */
	curl_easy_setopt(curl_handle, CURLOPT_AUTOREFERER, 1);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);

	/* Bounded time limits: without these, a stuck IOS socket (e.g.
	 * Dolphin's async-connect path returning EALREADY, or real
	 * hardware with a flaky Wi-Fi AP) would block curl_easy_perform()
	 * in the UI thread indefinitely. 15s connect + 60s overall are
	 * reasonable for page fetches; downloads get a longer timeout
	 * set separately in transfer.cpp. */
	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 15L);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 60L);

	/* some servers don't like requests that are made without a user-agent
	field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, Agents[Settings.UserAgent]);

	/* Accept any compression curl was built with (gzip via libz here).
	 * Without this, CloudFlare-fronted sites like x.com send gzip
	 * anyway and we'd receive unreadable bytes. "" = use all supported
	 * encodings and auto-decompress before delivery to the write cb. */
	curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");

	/* Cap redirects at a sane limit so a pathological server can't loop
	 * us until CURLOPT_TIMEOUT. */
	curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10L);

	/* Disable TCP_NODELAY: Dolphin's IOS doesn't support IPPROTO_TCP
	 * socket options (level=6), and curl closes the socket when
	 * setsockopt(TCP_NODELAY) fails. Buffering TCP writes is fine for
	 * our page-fetch use case (downloads have their own settings). */
	curl_easy_setopt(curl_handle, CURLOPT_TCP_NODELAY, 0L);

	/* Enable SSL certificate verification for HTTPS security */
	setup_cacert();
	if (cacert_path[0] != 0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(curl_handle, CURLOPT_CAINFO, cacert_path);
	}
	else
	{
		/* Fallback: disable verification if cert bundle couldn't be written */
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
	}

	/* override socket creation to use IPPROTO=0 (Dolphin compatibility) */
	curl_easy_setopt(curl_handle, CURLOPT_OPENSOCKETFUNCTION, opensocket_callback);
	curl_easy_setopt(curl_handle, CURLOPT_CLOSESOCKETFUNCTION, close_callback);

	/* bypass c-ares: resolve hostname via libogc and feed CURLOPT_RESOLVE */
	pre_resolve(curl_handle, url);

	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
}

int showprogress(void *bar,
				 curl_off_t dltotal,
				 curl_off_t dlnow,
				 curl_off_t ultotal,
				 curl_off_t ulnow)
{
	char msg[20];
	if (dltotal > 0)
		sprintf(msg, "Loading...%2.2f%%", (double)dlnow * 100.0 / (double)dltotal);
	else
		sprintf(msg, "Loading...");
	SetMessage(msg);

	if (CancelDownload())
		return 1;
	return 0;
}

void setrequestheaders(CURL *curl_handle, int request)
{
	if (!curl_handle)
		return;

	if (request == HEAD)
	{
		/* get header only */
		curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);
	}

	else if (request == GET)
	{
		/* reset handle to perform get */
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 0);  /* ensure body is fetched */
		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, showprogress);
	}

	else if (request == POST)
	{
	}
}

// -----------------------------------------------------------
// REQUEST FUNCTIONS
// -----------------------------------------------------------

struct block postrequest(CURL *curl_handle, const char *url, curl_mime *data)
{
	char *ct = nullptr;
	char *post = findRchr(const_cast<char *>(url), '?');
	struct block b, h;
	int res;

	struct HeaderStruct head;
	struct MemoryStruct chunk;

	chunk.memory = static_cast<char *>(malloc(1)); /* will be grown as needed by the realloc above */
	chunk.size = 0;								   /* no data at this point */

	head.memory = static_cast<char *>(malloc(1)); /* will be grown as needed by the realloc above */
	head.size = 0;								  /* no data at this point */
	head.download = false;						  /* not yet known at this point */
	head.filename[0] = 0;						  /* read by fillstruct() via strstr */

	setmainheaders(curl_handle, url);
	setrequestheaders(curl_handle, POST);

	if (curl_handle)
	{
		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, parseheader);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, static_cast<void *>(&chunk));
		curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, static_cast<void *>(&head));

		if (data == nullptr)
			curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post + 1);
		else
			curl_easy_setopt(curl_handle, CURLOPT_MIMEPOST, data);

		if ((res = curl_easy_perform(curl_handle)) != 0) /*error!*/
		{
			if (data)
				curl_mime_free(data);

			if (res == CURLE_WRITE_ERROR)
			{
				fillstruct(curl_handle, &head, &h);
				free(head.memory);
				return h;
			}

			Debug(curl_easy_strerror(static_cast<CURLcode>(res)));
			record_dl_error(curl_handle, res);
			return emptyblock;
		}

		if (data)
			curl_mime_free(data);

		if (CURLE_OK != curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct) || !ct)
		{
			record_dl_error(curl_handle, 0);
			return emptyblock;
		}
	}
	else
		return emptyblock;

	b.data = chunk.memory;
	b.size = chunk.size;
	free(head.memory);

	findChr(ct, ';');
	strcpy(b.type, ct);
	return b;
}

bool postcomment(CURL *curl_handle, const char *name, const char *content)
{
	char request[512];
	char *r = request;

	r += sprintf(r, "http://www.htmlcommentbox.com/post?");
	r += sprintf(r, "ajax=true&page=http%%3A%%2F%%2Fwiibrowser.altervista.org%%2Fmainsite%%2Ftracker.php");
	r += sprintf(
		r, "&refer=http%%3A%%2F%%2Fwiibrowser.altervista.org%%2Fmainsite%%2Ftracker.php%%23HCB_comment_box&opts=16862");
	r += sprintf(r, "&mod=%%241%%24wq1rdBcg%%249ZNKlgbkqbvBdR6hqvyy91&replies_to=&name=%s //%s&content=%s", name,
				 Settings.Uuid, content);

	struct block html = postrequest(curl_handle, request, nullptr);
	free(html.data);
	return (html.size > 0);
}

struct block getrequest(CURL *curl_handle, const char *url, FILE *hfile)
{
	char *ct = nullptr;
	struct block b, h;
	int res;

	struct HeaderStruct head;
	struct MemoryStruct chunk;

	chunk.memory = static_cast<char *>(malloc(1)); /* will be grown as needed by the realloc above */
	chunk.size = 0;								   /* no data at this point */

	head.memory = static_cast<char *>(malloc(1)); /* will be grown as needed by the realloc above */
	head.size = 0;								  /* no data at this point */
	head.download = false;						  /* not yet known at this point */
	head.filename[0] = 0;						  /* read by fillstruct() via strstr */

	setmainheaders(curl_handle, url);
	setrequestheaders(curl_handle, GET);

	if (curl_handle)
	{
		/* we pass our 'chunk' struct or 'hfile' to the callback function */
		if (hfile)
		{
			/* send all data to this function */
			curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writedata);
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, static_cast<void *>(hfile));
		}
		else
		{
			curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, parseheader);
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, static_cast<void *>(&chunk));
			curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, static_cast<void *>(&head));
		}

		if ((res = curl_easy_perform(curl_handle)) != 0) /*error!*/
		{
			if (res == CURLE_ABORTED_BY_CALLBACK)
			{
				h.size = DSTOPPED;
				return h;
			}

			if (res == CURLE_WRITE_ERROR)
			{
				fillstruct(curl_handle, &head, &h);
				free(head.memory);
				return h;
			}

			Debug(curl_easy_strerror(static_cast<CURLcode>(res)));
			record_dl_error(curl_handle, res);
			return emptyblock;
		}

		if (CURLE_OK != curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct) || !ct)
		{
			record_dl_error(curl_handle, 0);
			return emptyblock;
		}
	}
	else
		return emptyblock;

	b.data = chunk.memory;
	b.size = chunk.size;

	findChr(ct, ';');
	strcpy(b.type, ct);
	free(head.memory);

	if (hfile)
	{
		h.size = DCOMPLETE;
		fclose(hfile);
		return h;
	}
	return b;
}

curl_mime *multipartform(CURL *curl_handle, const char *url)
{
	curl_mime *form = curl_mime_init(curl_handle);

	char *temp = strrchr(url, '?');
	char *begin = url_decode(temp);
	char *mid = nullptr;

	char *name, *value;
	char *file, *path;

	/* Fill in the filename field */
	do
	{
		mid = strchr(begin, '=');
		begin++;
		name = strndup(begin, mid - begin);

		begin = strchr(begin, '&');
		mid++;

		if (!begin)
			value = strdup(mid);
		else
			value = strndup(mid, begin - mid);

		curl_mimepart *part = curl_mime_addpart(form);
		curl_mime_name(part, name);

		if (strstr(value, "_UPLOAD"))
		{
			path = strrchr(value, '_');
			file = strndup(value, path - value);

			curl_mime_filedata(part, file);

			free(file);
		}
		else
		{
			curl_mime_data(part, value, CURL_ZERO_TERMINATED);
		}

		free(name);
		free(value);
	} while (begin);

	free(begin);
	return form;
}

struct block downloadfile(CURL *curl_handle, const char *url, FILE *hfile)
{
	const char *mode = strrchr(url, '\\');
	/* Splits the URL at '\\': part before becomes null-terminated, `mode`
	 * points to the new NUL, mode+1 addresses the suffix (e.g. "post"). */
	findRchr(const_cast<char *>(url), '\\');

	if (firstRun)
		firstRun = false;
	else
		curl_easy_reset(curl_handle);

	if (!mode)
		return getrequest(curl_handle, url, hfile);

	if (strcasestr(mode + 1, "post"))
		return postrequest(curl_handle, url, nullptr);
	if (strcasestr(mode + 1, "multipart"))
	{
		curl_mime *data = multipartform(curl_handle, url);
		return postrequest(curl_handle, url, data);
	}

	return getrequest(curl_handle, url, hfile);
}

// -----------------------------------------------------------
// DEBUG METHODES
// -----------------------------------------------------------

void Debug(const char *msg)
{
#ifdef DEBUG_LEVEL
	FILE *f = fopen("debug.txt", "a");
	fputs(msg, f);
	fputs("\r\n", f);
	fclose(f);
#endif
}

void DebugInt(u32 msg)
{
#ifdef DEBUG_LEVEL
	FILE *f = fopen("debug.txt", "a");
	fprintf(f, "%d", msg);
	fputs("\r\n", f);
	fclose(f);
#endif
}

void save(struct block *b, FILE *hfile)
{
	FILE *f = hfile;
	if (!f)
		f = fopen("page.wbr", "wb");
	fwrite(b->data, b->size, 1, f);
	fclose(f);
}

// -----------------------------------------------------------
// EXTENSIONS METHODES
// -----------------------------------------------------------

bool mustdownload(char content[])
{
	/* strcasestr so callers that don't pre-lowercase (e.g. fillstruct
	 * acting on the raw CURLINFO_CONTENT_TYPE value) still classify
	 * "TEXT/HTML" as renderable. parseline tolower()s before calling
	 * this, so both paths converge. */
	if (strcasestr(content, "text/html") || strcasestr(content, "application/xhtml")
		/* || strcasestr(content, "text") */
		|| strcasestr(content, "image")
#ifdef MPLAYER
		|| strcasestr(content, "video")
#endif
	)
		return false;
	return true;
}

/* Note: these mutate the input — parameter is char*, not const char*.
 * Previously typed const char* which was UB when called on mutable
 * buffers (strcmp/strncmp etc. downstream depend on the truncation). */
char *findChr(char *str, char chr)
{
	char *c = strchr(str, chr);
	if (c != nullptr)
		*c = '\0';
	return c;
}

char *findRchr(char *str, char chr)
{
	char *c = strrchr(str, chr);
	if (c != nullptr)
		*c = '\0';
	return c;
}

bool validProxy()
{
	if (strlen(Settings.Proxy))
		return true;

	return false;
}

void trimline(char *init, struct block *dest)
{
	int len = strlen(init);
	while (len > 0 && strchr(" \r\n", init[len - 1]))
		len--;

	dest->data = static_cast<char *>(malloc(256));
	init[len] = 0;

	if (init[0] == '"')
	{
		findChr(init + 1, '"');
		strcpy(dest->data, init + 1);
	}
	else
		strcpy(dest->data, init);
}

void fillstruct(CURL *handle, HeaderStruct *head, struct block *dest)
{
	char *ct = nullptr;
	char *ft;
	dest->size = DFOUND;

	if (CURLE_OK == curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct) && ct)
	{
		findChr(ct, ';');
		strcpy(dest->type, ct);
	}

	if ((ft = strstr(head->filename, "filename=")))
	{
		trimline(ft + 9, dest);
	}
	else
		dest->data = nullptr;
}
