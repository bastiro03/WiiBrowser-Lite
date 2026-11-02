#include "url_helper.h"
#include "safety.h"
#include <cstring>
#include <cstdlib>
#include <vector>

std::string urlGetHost(const char *url)
{
    if (!url || !wbl::httpUrl(url)) return "";
    const std::string base(url);
    const size_t authority = base.find("://") + 3;
    const size_t end = base.find_first_of("/?#", authority);
    return base.substr(0, end) + "/";
}
char *getHost(char *url) { const std::string value = urlGetHost(url); return strdup(value.c_str()); }

std::string urlGetRoot(const char *url)
{
    if (!url || !wbl::httpUrl(url)) return "";
    std::string base(url);
    const size_t suffix = base.find_first_of("?#");
    if (suffix != std::string::npos) base.erase(suffix);
    const size_t authority = base.find("://") + 3;
    const size_t slash = base.rfind('/');
    return slash < authority ? base + "/" : base.substr(0, slash + 1);
}
std::string getRoot(char *url) { return urlGetRoot(url); }

static std::string normalize(const std::string &url)
{
    if (!wbl::httpUrl(url)) return "";
    const size_t authority = url.find("://") + 3;
    const size_t pathStart = url.find_first_of("/?#", authority);
    if (pathStart == std::string::npos || url[pathStart] != '/') return url;
    const size_t suffix = url.find_first_of("?#", pathStart);
    const std::string path = url.substr(pathStart, suffix - pathStart);
    std::vector<std::string> components;
    size_t position = 1;
    while (position < path.size()) {
        const size_t end = path.find('/', position);
        const std::string part = path.substr(position, end - position);
        if (part == "..") { if (!components.empty()) components.pop_back(); }
        else if (!part.empty() && part != ".") components.push_back(part);
        if (end == std::string::npos) break;
        position = end + 1;
    }
    std::string result = url.substr(0, pathStart) + "/";
    for (size_t i = 0; i < components.size(); ++i) {
        if (i) result += '/';
        result += components[i];
    }
    if (!components.empty() && (path.back() == '/' ||
        (path.size() >= 2 && path.compare(path.size() - 2, 2, "/.") == 0) ||
        (path.size() >= 3 && path.compare(path.size() - 3, 3, "/..") == 0))) result += '/';
    if (suffix != std::string::npos) result += url.substr(suffix);
    return result;
}

std::string urlAdjust(const std::string &link, const char *url)
{
    if (link.empty()) return url ? normalize(url) : "";
    if (wbl::httpUrl(link)) return normalize(link);
    if (!url || !wbl::httpUrl(url)) return "";
    const std::string base(url);
    if (link.compare(0, 2, "//") == 0) return normalize(base.substr(0, base.find(':') + 1) + link);
    // Unknown schemes, control characters and command-like backslashes are not
    // relative URLs. In particular, never turn file:/ or javascript: into HTTP.
    const size_t colon = link.find(':');
    const size_t separator = link.find_first_of("/?#");
    if (colon != std::string::npos && (separator == std::string::npos || colon < separator)) return "";
    if (link[0] == '#') return normalize(base.substr(0, base.find('#')) + link);
    if (link[0] == '?') return normalize(base.substr(0, base.find_first_of("?#")) + link);
    if (link[0] == '/') {
        const std::string origin = urlGetHost(url);
        return normalize(origin.substr(0, origin.size() - 1) + link);
    }
    return normalize(urlGetRoot(url) + link);
}

std::string adjustUrl(std::string link, const char *url) { return urlAdjust(link, url); }
std::string urlJump(const std::string &link, const char *url) { return urlAdjust(link, url); }
std::string jumpUrl(std::string link, const char *url) { return urlJump(link, url); }
std::string urlParse(const std::string &link, const char *url) { return urlAdjust(link, url); }
std::string parseUrl(std::string link, const char *url) { return urlParse(link, url); }
