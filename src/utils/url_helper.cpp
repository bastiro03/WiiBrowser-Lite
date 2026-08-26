#include "url_helper.h"
#include <cstring>
#include <cstdlib>

// Safe getHost — returns std::string, never derefs null
std::string urlGetHost(const char *url) {
    if (!url || !url[0]) return "";
    const char *p = strchr(url, '/');
    if (!p) return std::string(url); // no slash at all
    // Expect "http://host/..." — need "://"
    const char *proto = strstr(url, "://");
    if (!proto) {
        // No protocol, treat first '/' as host end
        const char *c = strchr(url, '/');
        if (!c) return std::string(url);
        return std::string(url, (c + 1) - url);
    }
    p = proto + 3; // skip "://"
    const char *c = strchr(p, '/');
    if (c) {
        // include trailing '/'
        return std::string(url, (c + 1) - url);
    }
    return std::string(url);
}

// Legacy wrapper — retains strndup semantics for old callers (must free)
char *getHost(char *url) {
    std::string s = urlGetHost(url);
    // If original logic would have returned `url` directly (no alloc), we mimic by strndup as well
    // to give caller ownership consistently. Caller must free.
    return strndup(s.c_str(), s.size());
}

std::string urlGetRoot(const char *url) {
    if (!url || !url[0]) return "/";
    const char *firstSlash = strchr(url, '/');
    if (!firstSlash) return std::string(url) + "/";
    const char *proto = strstr(url, "://");
    const char *start = proto ? proto + 3 : url;
    // Find last '/' after start
    const char *last = strrchr(start, '/');
    if (last) {
        return std::string(url, (last + 1) - url);
    }
    // No slash after host — return url + "/"
    std::string r(url);
    if (!r.empty() && r.back() != '/') r += "/";
    return r;
}
std::string getRoot(char *url) {
    return urlGetRoot(url);
}

std::string urlAdjust(const std::string& link, const char *url) {
    if (link.empty()) return url ? std::string(url) : "";
    if (link.rfind("http://", 0) == 0 || link.rfind("https://", 0) == 0)
        return link;
    if (link.size() >= 2 && link[0] == '/' && link[1] == '/') {
        return std::string("http:") + link; // TODO: preserve https if base is https
    }
    if (link[0] == '/') {
        std::string host = urlGetHost(url);
        std::string l = link;
        if (!host.empty() && host.back() == '/')
            l.erase(l.begin()); // avoid "//"
        return host + l;
    }
    // relative
    std::string root = urlGetRoot(url);
    return root + link;
}

// Compat wrappers
std::string adjustUrl(std::string link, const char* url) {
    return urlAdjust(link, url);
}

std::string urlJump(const std::string& link, const char *url) {
    if (!url) return link;
    std::string r(url);
    r += link;
    return r;
}
std::string jumpUrl(std::string link, const char* url) {
    return urlJump(link, url);
}

std::string urlParse(const std::string& link, const char *url) {
    if (link.empty()) return url ? std::string(url) : "";
    if (link[0] == '#')
        return urlJump(link, url);
    return urlAdjust(link, url);
}
std::string parseUrl(std::string link, const char* url) {
    return urlParse(link, url);
}
