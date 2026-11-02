#include "safety.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <algorithm>
#include <cctype>
#include <vector>

namespace wbl {
bool copyString(char *destination, std::size_t capacity, const char *source)
{
    if (!destination || !capacity || !source) return false;
    const std::size_t length = std::strlen(source);
    if (length >= capacity) return false;
    std::memmove(destination, source, length + 1);
    return true;
}

bool appendString(char *destination, std::size_t capacity, const char *source)
{
    if (!destination || !capacity || !source) return false;
    std::size_t used = 0;
    while (used < capacity && destination[used]) ++used;
    if (used == capacity) return false;
    return copyString(destination + used, capacity - used, source);
}

bool safeFilename(const std::string &name)
{
    if (name.empty() || name.size() > 255 || name == "." || name == ".." ||
        name.back() == '.' || name.back() == ' ') return false;
    for (unsigned char c : name) {
        if (c < 32 || c == 127 || std::strchr("/\\:*?\"<>|", c)) return false;
    }
    return true;
}

bool storagePath(const std::string &path)
{
    std::size_t start;
    if (path.compare(0, 4, "sd:/") == 0) start = 4;
    else if (path.compare(0, 5, "usb:/") == 0) start = 5;
    else return false;
    if (path.size() >= 512) return false;
    while (start < path.size()) {
        const std::size_t end = path.find('/', start);
        const std::string part = path.substr(start, end - start);
        if (!safeFilename(part)) return false;
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return true;
}

bool httpUrl(const std::string &url)
{
    const std::size_t prefix = url.compare(0, 7, "http://") == 0 ? 7 :
                               url.compare(0, 8, "https://") == 0 ? 8 : 0;
    if (!prefix || url.size() <= prefix || url.size() > MaxUrl) return false;
    const std::size_t hostEnd = url.find_first_of("/?#", prefix);
    if (hostEnd == prefix) return false;
    if (url.substr(prefix, hostEnd - prefix).find('@') != std::string::npos)
        return false; // Do not retain URL-embedded credentials.
    for (unsigned char c : url) if (c <= 32 || c == 127 || c == '\\') return false;
    return true;
}

bool parseInteger(const char *text, int minimum, int maximum, int &value)
{
    if (!text || !*text) return false;
    errno = 0;
    char *end = nullptr;
    const long parsed = std::strtol(text, &end, 10);
    if (errno || !end || *end || parsed < minimum || parsed > maximum) return false;
    value = static_cast<int>(parsed);
    return true;
}

bool voidElement(const std::string &name)
{
    const char *names[] = {"area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "param", "source", "track", "wbr"};
    for (const char *candidate : names) if (name == candidate) return true;
    return false;
}

bool markupBudget(const std::string &html)
{
    if (html.size() >= MaxDocument) return false;
    std::vector<std::string> parents;
    size_t cursor = 0, tokens = 0;
    while ((cursor = html.find('<', cursor)) != std::string::npos) {
        if (++tokens > MaxPageItems) return false;
        if (html.compare(cursor, 4, "<!--") == 0) {
            const size_t end = html.find("-->", cursor + 4);
            if (end == std::string::npos) return false;
            cursor = end + 3;
            continue;
        }
        size_t start = cursor + 1;
        if (start == html.size()) return false;
        const bool closing = html[start] == '/';
        if (closing) ++start;
        size_t endName = start;
        while (endName < html.size() &&
               (std::isalnum(static_cast<unsigned char>(html[endName])) || html[endName] == '-' || html[endName] == ':')) ++endName;
        if (endName == start) { ++cursor; continue; }
        if (endName - start > 64) return false;
        std::string name = html.substr(start, endName - start);
        for (char &c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        char quote = 0;
        size_t end = endName;
        for (; end < html.size(); ++end) {
            const char c = html[end];
            if (quote) { if (c == quote) quote = 0; }
            else if (c == '"' || c == '\'') quote = c;
            else if (c == '>') break;
        }
        if (end == html.size()) return false;
        if (closing) {
            const auto match = std::find(parents.rbegin(), parents.rend(), name);
            if (match != parents.rend()) parents.resize(parents.size() - static_cast<size_t>(match - parents.rbegin()) - 1);
        } else if (!voidElement(name) && !(end > cursor && html[end - 1] == '/')) {
            parents.push_back(name);
            if (parents.size() > 64) return false;
        }
        cursor = end + 1;
    }
    return true;
}

bool imageSize(std::size_t width, std::size_t height, std::size_t bpp,
               std::size_t &bytes)
{
    if (!width || !height || !bpp || width > 4096 || height > 4096 ||
        width > MaxImagePixels / height) return false;
    const std::size_t pixels = width * height;
    if (pixels > MaxImageBytes / bpp) return false;
    bytes = pixels * bpp;
    return true;
}
}
