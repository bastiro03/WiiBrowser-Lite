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
		/* Android 9 (Pie) with Chrome 92 — last Chrome version on Android 9.
		 * Matches our approximate capabilities: modern TLS 1.3, HTTP/1.x,
		 * CSS3, ES5 JavaScript. Avoids x-safari-* deep-link redirects that
		 * watchOS triggers on x.com. Generic "Android 9" without device
		 * model reduces fingerprinting. */
		"Mozilla/5.0 (Linux; Android 9) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.159 Mobile Safari/537.36",
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
}

// -----------------------------------------------------------
// DATA HANDLING
// -----------------------------------------------------------

struct HeaderStruct
{
	char *memory;
	u32 size;
	bool download;
	bool in_redirect;   /* true while parsing a 3xx response's headers */
	char filename[256];
	char location[512]; /* captured Location header for x-safari-* rewrite */
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

				/* FIX for wikipedia.org→www.wikipedia.org and x.com→www.x.com
				 * redirects: also add www.HOSTNAME variant (or bare HOSTNAME if
				 * the input already starts with www.). This covers 90%+ of
				 * real-world redirects without dynamic DNS resolution.
				 *
				 * Example: wikipedia.org (bare) → add www.wikipedia.org entries.
				 * Example: www.x.com (www.) → add x.com (bare) entries.
				 *
				 * Use the same IP we already resolved for the initial hostname. */
				char www_host[256];
				if (strncmp(host, "www.", 4) == 0) {
					/* Input is www.HOST → also add bare HOST. */
					snprintf(www_host, sizeof(www_host), "%s", host + 4);
				} else {
					/* Input is bare HOST → also add www.HOST. */
					snprintf(www_host, sizeof(www_host), "www.%s", host);
				}

				for (int i = 0; common_ports[i] != 0; i++)
				{
					char entry[256];
					snprintf(entry, sizeof(entry), "%s:%u:%u.%u.%u.%u",
					         www_host, (unsigned)common_ports[i],
					         ip[0], ip[1], ip[2], ip[3]);
					list = curl_slist_append(list, entry);
					fprintf(stderr, "wbl_build_resolve_list: %s\n", entry);
				}
				if (url_port != 80 && url_port != 443 &&
				    url_port > 0 && url_port < 65536)
				{
					char entry[256];
					snprintf(entry, sizeof(entry), "%s:%lu:%u.%u.%u.%u",
					         www_host, url_port, ip[0], ip[1], ip[2], ip[3]);
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

/* Check whether the resolve list already has an entry for host:port.
 * Used by the redirect hook to avoid duplicate entries. */
static bool resolve_list_has(struct curl_slist *list, const char *host, unsigned port)
{
	char prefix[256];
	int n = snprintf(prefix, sizeof(prefix), "%s:%u:", host, port);
	if (n <= 0 || n >= (int)sizeof(prefix))
		return false;
	for (struct curl_slist *p = list; p; p = p->next)
	{
		if (strncmp(p->data, prefix, n) == 0)
			return true;
	}
	return false;
}

/* Resolve a hostname via libogc and append entries to httplib_resolve_list
 * for common ports (80, 443). Called when we detect a redirect target with
 * a hostname we haven't yet resolved. Returns true if entries were added. */
static bool append_resolve_for_host(CURL *handle, const char *host)
{
	/* Skip if host is already an IPv4 literal. */
	if (host[0] >= '0' && host[0] <= '9')
		return false;

	/* Skip if we already have entries for this host at port 443. */
	if (resolve_list_has(httplib_resolve_list, host, 443))
		return false;

	struct hostent *he = net_gethostbyname(host);
	if (!he || !he->h_addr_list || !he->h_addr_list[0])
	{
		fprintf(stderr, "append_resolve_for_host: net_gethostbyname(%s) failed\n", host);
		fflush(stderr);
		return false;
	}
	const unsigned char *ip = (const unsigned char *)he->h_addr_list[0];

	static const unsigned short common_ports[] = {80, 443, 0};
	for (int i = 0; common_ports[i] != 0; i++)
	{
		char entry[256];
		snprintf(entry, sizeof(entry), "%s:%u:%u.%u.%u.%u",
		         host, (unsigned)common_ports[i],
		         ip[0], ip[1], ip[2], ip[3]);
		httplib_resolve_list = curl_slist_append(httplib_resolve_list, entry);
		fprintf(stderr, "append_resolve_for_host: %s\n", entry);
	}
	fflush(stderr);

	/* Re-set CURLOPT_RESOLVE so curl picks up the new entries for the
	 * upcoming redirect hop. curl re-processes this option on each new
	 * connection attempt. */
	curl_easy_setopt(handle, CURLOPT_RESOLVE, httplib_resolve_list);
	return true;
}

/* Extract hostname from a curl debug log message of the form:
 *   "Issue another request to this URL: 'https://host/path'"
 * Returns true on success with hostname in out_host. */
static bool extract_redirect_host(const char *data, size_t size,
                                  char *out_host, size_t out_size)
{
	static const char MARKER[] = "Issue another request to this URL: '";
	size_t marker_len = sizeof(MARKER) - 1;

	if (size <= marker_len)
		return false;
	if (memcmp(data, MARKER, marker_len) != 0)
		return false;

	const char *url_start = data + marker_len;
	const char *url_end = (const char *)memchr(url_start, '\'', size - marker_len);
	if (!url_end || url_end == url_start)
		return false;

	size_t url_len = url_end - url_start;
	char url[512];
	if (url_len >= sizeof(url))
		return false;
	memcpy(url, url_start, url_len);
	url[url_len] = 0;

	CURLU *u = curl_url();
	if (!u) return false;
	if (curl_url_set(u, CURLUPART_URL, url, 0) != CURLUE_OK)
	{
		curl_url_cleanup(u);
		return false;
	}
	char *host = NULL;
	if (curl_url_get(u, CURLUPART_HOST, &host, 0) != CURLUE_OK || !host)
	{
		curl_url_cleanup(u);
		return false;
	}
	bool ok = (strlen(host) < out_size);
	if (ok)
		snprintf(out_host, out_size, "%s", host);
	curl_free(host);
	curl_url_cleanup(u);
	return ok;
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

	/* Status line ("HTTP/1.1 302 ...", "HTTP/2 302 ...") starts a new
	 * response. curl calls HEADERFUNCTION once per header line INCLUDING
	 * intermediate redirect responses. Two regressions to avoid:
	 *   1. Leaking download=true from a 3xx hop's Content-Type to a later
	 *      hop's end-of-headers — reset download on every status line.
	 *   2. Setting download=true on a 3xx hop (e.g. x.com's 302 response
	 *      has Content-Type: text/plain) which would abort curl's redirect
	 *      follow — track in_redirect and skip Content-Type classification
	 *      while in a 3xx response. */
	if (!strncmp(line, "http/", 5))
	{
		mem->download = false;
		int code = 0;
		sscanf(line, "http/%*s %d", &code);
		mem->in_redirect = (code >= 300 && code < 400);
	}
	else if (!strncmp(line, "content-type", 12))
	{
		/* Skip Content-Type classification on 3xx responses: the Content-Type
		 * describes the redirect-explainer body, not the final target. */
		if (!mem->in_redirect)
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
	}

	else if (!strncmp(line, "content-disposition", 19))
	{
		/* Was strcpy() which can overflow filename[256] if the header
		 * line is long (e.g. long RFC 5987 filename*=UTF-8''...). */
		snprintf(mem->filename, sizeof(mem->filename), "%s", line);
	}

	else if (!strncmp(line, "location:", 9))
	{
		/* Capture Location header for x-safari-* URL rewriting.
		 * x.com returns "x-safari-https://redirect.x.com/..." when it
		 * detects watchOS User-Agent — a non-standard Apple scheme for
		 * app deep-linking. curl rejects it with CURLE_UNSUPPORTED_PROTOCOL.
		 * We'll strip the x-safari- prefix later if curl bails. */
		snprintf(mem->location, sizeof(mem->location), "%s", line);

		fprintf(stderr, "[REDIRECT] Location header: %s", line);
		if (realsize > 0 && line[realsize-1] != '\n')
			fprintf(stderr, "\n");
		fflush(stderr);
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

		/* Detect URL parsing errors during redirect to diagnose what's
		 * causing CURLE_UNSUPPORTED_PROTOCOL. Common issue: x.com returns
		 * uppercase "HTTP://" or protocol-relative "//" Location headers
		 * that older curl versions can't handle. */
		if (strstr(data, "could not be parsed") || strstr(data, "Unsupported")) {
			fprintf(stderr, "[REDIRECT_ERROR] curl URL parsing failed - check Location header above\n");
			fflush(stderr);
		}

		/* Hook for redirect chains with arbitrary subdomain changes.
		 * Example chain we must handle:
		 *   http://x.com → https://x.com → https://www.x.com
		 *                → https://ssl.x.com → https://www.x.com/help
		 *
		 * The static wbl_build_resolve_list() seeds entries for the
		 * INITIAL hostname + its www/bare variant. But curl follows
		 * redirects to arbitrary subdomains (ssl.x.com, mobile.x.com,
		 * accounts.google.com, etc.) that we can't predict. Without a
		 * resolve entry for the new host, curl tries c-ares (which we've
		 * disabled via IPPROTO_IP workarounds) and fails with
		 * "Could not resolve host".
		 *
		 * When curl prints "Issue another request to this URL: '...'",
		 * we extract the target hostname, resolve via net_gethostbyname,
		 * and append to the resolve list BEFORE curl attempts the new
		 * connection. curl re-reads CURLOPT_RESOLVE on each new
		 * connection so the added entries take effect immediately. */
		char new_host[256];
		if (extract_redirect_host(data, size, new_host, sizeof(new_host)))
		{
			append_resolve_for_host(handle, new_host);
		}
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

	/* Disable automatic redirect following — we follow redirects manually
	 * in getrequest() to ensure connections close between hops. With
	 * FOLLOWLOCATION=1, curl keeps earlier connections alive in the pool
	 * while opening new ones, causing mbedTLS context corruption (-0x7100).
	 * Manual loop: perform → check 3xx → extract Location → loop. */
	curl_easy_setopt(curl_handle, CURLOPT_AUTOREFERER, 1);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0);
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
	field, so we provide one.
	 *
	 * Special case: x.com returns x-safari-* redirects (Apple deep-link
	 * scheme) when it sees watchOS User-Agent, creating an infinite loop.
	 * Use Android Mobile Safari UA for x.com to avoid this. */
	int ua_index = Settings.UserAgent;
	if (url && strstr(url, "x.com")) {
		ua_index = 5;  /* Android Mobile Safari - no x-safari-* triggers */
		fprintf(stderr, "[UA] x.com detected, using Android UA (index %d)\n", ua_index);
		fflush(stderr);
	}
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, Agents[ua_index]);

	/* Accept any compression curl was built with (gzip via libz here).
	 * Without this, CloudFlare-fronted sites like x.com send gzip
	 * anyway and we'd receive unreadable bytes. "" = use all supported
	 * encodings and auto-decompress before delivery to the write cb. */
	curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");

	/* Cap redirects at a sane limit (enforced in getrequest() manual loop). */
	curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10L);

	/* Force connection close after each request. With manual redirect
	 * following, we need to ensure the connection closes completely before
	 * the next hop opens. Without this, curl keeps connections alive in the
	 * pool across redirect hops, causing overlapping SSL contexts and
	 * mbedTLS corruption (-0x7100). Cost: extra TLS handshakes. */
	curl_easy_setopt(curl_handle, CURLOPT_FORBID_REUSE, 1L);

	/* Disable TCP_NODELAY: Dolphin's IOS doesn't support IPPROTO_TCP
	 * socket options (level=6), and curl closes the socket when
	 * setsockopt(TCP_NODELAY) fails. Buffering TCP writes is fine for
	 * our page-fetch use case (downloads have their own settings). */
	curl_easy_setopt(curl_handle, CURLOPT_TCP_NODELAY, 0L);

	/* Enable SSL certificate verification for HTTPS security.
	 * On Wii/DS/DSi, mbedTLS is built without MBEDTLS_FS_IO (no file I/O),
	 * so we use CURLOPT_CAINFO_BLOB to load the CA bundle from memory
	 * instead of disk. cacert_pem is embedded binary data from filelist.h */
	setup_cacert();
#if WBL_HAS_HTTPS
	struct curl_blob cacert_blob;
	cacert_blob.data = (void *)cacert_pem;
	cacert_blob.len = cacert_pem_size;
	cacert_blob.flags = CURL_BLOB_NOCOPY;  /* cacert_pem is const global */

	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
	/* curl 7.66+ treats CURLOPT_SSL_VERIFYHOST as boolean (0 or 1);
	 * value 2 (the legacy "check name matches cert") is accepted but
	 * emits a warning. Use 1 which has the same semantics today. */
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_CAINFO_BLOB, &cacert_blob);
#endif

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
	head.in_redirect = false;					  /* set on each 3xx status line */
	head.filename[0] = 0;						  /* read by fillstruct() via strstr */
	head.location[0] = 0;						  /* captured for x-safari-* rewrite */

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
	long http_code = 0;
	int redirect_count = 0;
	char current_url[512];
	char visited_urls[10][512];  /* Track visited URLs to detect loops */
	int visited_count = 0;
	const int MAX_REDIRECTS = 10;

	snprintf(current_url, sizeof(current_url), "%s", url);

	/* Manual redirect loop: With CURLOPT_FOLLOWLOCATION=0, we handle redirects
	 * ourselves to ensure each connection closes before opening the next. This
	 * prevents mbedTLS context corruption (-0x7100) from overlapping SSL contexts. */
	while (redirect_count < MAX_REDIRECTS)
	{
		/* Detect redirect loops (e.g., x.com ↔ redirect.x.com). */
		for (int i = 0; i < visited_count; i++)
		{
			if (strcmp(visited_urls[i], current_url) == 0)
			{
				fprintf(stderr, "[REDIRECT] Loop detected: already visited %s (hop %d)\n", current_url, i + 1);
				fflush(stderr);
				record_dl_error(curl_handle, 0);
				return emptyblock;
			}
		}

		/* Record this URL as visited. */
		if (visited_count < MAX_REDIRECTS)
		{
			snprintf(visited_urls[visited_count], sizeof(visited_urls[visited_count]), "%s", current_url);
			visited_count++;
		}

		struct HeaderStruct head;
		struct MemoryStruct chunk;

		chunk.memory = static_cast<char *>(malloc(1)); /* will be grown as needed by the realloc above */
		chunk.size = 0;								   /* no data at this point */

		head.memory = static_cast<char *>(malloc(1)); /* will be grown as needed by the realloc above */
		head.size = 0;								  /* no data at this point */
		head.download = false;						  /* not yet known at this point */
		head.in_redirect = false;					  /* set on each 3xx status line */
		head.filename[0] = 0;						  /* read by fillstruct() via strstr */
		head.location[0] = 0;						  /* captured for redirect extraction */

		/* Reset curl handle to completely clean state before each redirect hop.
		 * This closes all connections, frees internal buffers, and resets all
		 * options. Critical for preventing mbedTLS context corruption (-0x7100)
		 * on platforms where curl's connection management interacts poorly with
		 * the SSL backend. */
		if (redirect_count > 0) {
			curl_easy_reset(curl_handle);
		}

		setmainheaders(curl_handle, current_url);
		setrequestheaders(curl_handle, GET);

		if (!curl_handle)
		{
			free(chunk.memory);
			free(head.memory);
			return emptyblock;
		}

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

		res = curl_easy_perform(curl_handle);

		if (res != CURLE_OK)
		{
			if (res == CURLE_ABORTED_BY_CALLBACK)
			{
				free(chunk.memory);
				free(head.memory);
				h.size = DSTOPPED;
				return h;
			}

			if (res == CURLE_WRITE_ERROR)
			{
				fillstruct(curl_handle, &head, &h);
				free(chunk.memory);
				free(head.memory);
				return h;
			}

			/* x.com returns "Location: x-safari-https://redirect.x.com/..."
			 * when it detects watchOS User-Agent. This is Apple's non-standard
			 * deep-link scheme; curl rejects it with CURLE_UNSUPPORTED_PROTOCOL.
			 * Strip the x-safari- prefix and retry. */
			if (res == CURLE_UNSUPPORTED_PROTOCOL && head.location[0] != '\0')
			{
				/* Parse "location: x-safari-https://host/path" */
				const char *loc_value = head.location + 9; /* skip "location:" */
				while (*loc_value == ' ') loc_value++;     /* skip whitespace */

				/* Detect x-safari-http[s]:// and extract the real URL. */
				const char *real_url = nullptr;
				if (strncmp(loc_value, "x-safari-https://", 17) == 0)
					real_url = loc_value + 9; /* skip "x-safari-", keep "https://..." */
				else if (strncmp(loc_value, "x-safari-http://", 16) == 0)
					real_url = loc_value + 9; /* skip "x-safari-", keep "http://..." */

				if (real_url)
				{
					fprintf(stderr, "[X-SAFARI-REWRITE] Stripping x-safari- prefix, retrying with: %s\n", real_url);
					fflush(stderr);

					/* Copy fixed URL and strip trailing whitespace. */
					snprintf(current_url, sizeof(current_url), "%s", real_url);
					size_t len = strlen(current_url);
					while (len > 0 && (current_url[len-1] == '\r' || current_url[len-1] == '\n' || current_url[len-1] == ' '))
						current_url[--len] = '\0';

					/* Free this iteration's memory and retry in next loop. */
					free(chunk.memory);
					free(head.memory);
					redirect_count++;
					continue;
				}
			}

			Debug(curl_easy_strerror(static_cast<CURLcode>(res)));
			record_dl_error(curl_handle, res);
			free(chunk.memory);
			free(head.memory);
			return emptyblock;
		}

		/* Get HTTP response code to check for redirects. */
		curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

		/* Handle 3xx redirects manually. */
		if (http_code >= 300 && http_code < 400)
		{
			if (head.location[0] == '\0')
			{
				fprintf(stderr, "[REDIRECT] 3xx response (%ld) but no Location header\n", http_code);
				fflush(stderr);
				record_dl_error(curl_handle, 0);
				free(chunk.memory);
				free(head.memory);
				return emptyblock;
			}

			/* Extract Location header value. */
			const char *loc_value = head.location + 9; /* skip "location:" */
			while (*loc_value == ' ') loc_value++;     /* skip whitespace */

			/* Handle x-safari-* rewriting within redirect chain. */
			const char *next_url = loc_value;
			if (strncmp(loc_value, "x-safari-https://", 17) == 0)
			{
				next_url = loc_value + 9;
				fprintf(stderr, "[REDIRECT] x-safari-* detected in Location, rewriting\n");
				fflush(stderr);
			}
			else if (strncmp(loc_value, "x-safari-http://", 16) == 0)
			{
				next_url = loc_value + 9;
				fprintf(stderr, "[REDIRECT] x-safari-* detected in Location, rewriting\n");
				fflush(stderr);
			}

			fprintf(stderr, "[REDIRECT] %ld → %s (hop %d/%d)\n", http_code, next_url, redirect_count + 1, MAX_REDIRECTS);
			fflush(stderr);

			/* Copy next URL and strip trailing whitespace. */
			snprintf(current_url, sizeof(current_url), "%s", next_url);
			size_t len = strlen(current_url);
			while (len > 0 && (current_url[len-1] == '\r' || current_url[len-1] == '\n' || current_url[len-1] == ' '))
				current_url[--len] = '\0';

			/* Free this iteration's memory and continue to next hop. */
			free(chunk.memory);
			free(head.memory);
			redirect_count++;
			continue;
		}

		/* Non-redirect response: success. Extract content type and return. */
		if (CURLE_OK != curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct) || !ct)
		{
			record_dl_error(curl_handle, 0);
			free(chunk.memory);
			free(head.memory);
			return emptyblock;
		}

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

	/* Too many redirects. */
	fprintf(stderr, "[REDIRECT] Too many redirects (max=%d)\n", MAX_REDIRECTS);
	fflush(stderr);
	record_dl_error(curl_handle, 0);
	return emptyblock;
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
