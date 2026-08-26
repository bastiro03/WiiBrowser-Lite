#ifndef _URL_HELPER_H_
#define _URL_HELPER_H_

#include <string>

// Sprint 3.3: extracted from src/ui/menu.cpp (getHost/adjustUrl family)
// These are pure, host-testable functions — no GX/CURL dependency.

/**
 * getHost — returns "http://host/" portion of url, or original url if no path.
 * Legacy impl used strndup without free and `strchr(url,'/')+2` without null check.
 * New impl is safe and returns std::string (caller owns).
 */
std::string urlGetHost(const char *url);
char *getHost(char *url); // legacy wrapper, returns strndup (caller must free) for compat

/**
 * getRoot — returns directory portion of url (up to last '/').
 */
std::string urlGetRoot(const char *url);
std::string getRoot(char *url); // compat wrapper

/**
 * adjustUrl — resolves `link` against `url` (handles http, //, /, relative).
 * Safe for empty link (returns url).
 */
std::string urlAdjust(const std::string& link, const char *url);
std::string adjustUrl(std::string link, const char* url); // compat wrapper

std::string urlJump(const std::string& link, const char *url);
std::string jumpUrl(std::string link, const char* url);

std::string urlParse(const std::string& link, const char *url);
std::string parseUrl(std::string link, const char* url);

#endif
