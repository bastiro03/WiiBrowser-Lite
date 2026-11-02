#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "stringop.h"
#include "safety.h"

static std::string suggestedName(const block *html, const char *url)
{
    if (html && html->data && html->data[0]) return html->data;
    if (!url) return "";
    std::string name(url);
    name.erase(name.find_first_of("?#") == std::string::npos ? name.size() :
               name.find_first_of("?#"));
    const size_t slash = name.rfind('/');
    return slash == std::string::npos ? "" : name.substr(slash + 1);
}

static bool completePath(block *html, const char *url, char *path,
                         size_t capacity, const char *placeholder)
{
    if (!path || !capacity || strnlen(path, capacity) == capacity ||
        !wbl::storagePath(path)) return false;
    const char *slash = strrchr(path, '/');
    if (!slash) return false;
    std::string suggestion = suggestedName(html, url);
    // A server suggestion is a single component, never a relative path. Reject
    // suspicious names instead of silently flattening them into another name.
    if (!suggestion.empty() && !wbl::safeFilename(suggestion)) return false;
    const char *extension = html ? mime2ext(html->type) : NULL;
    if (!slash[1]) {
        if (suggestion.empty()) suggestion = std::string(placeholder) + (extension ? extension : "");
        if (!wbl::safeFilename(suggestion) ||
            !wbl::appendString(path, capacity, suggestion.c_str())) return false;
    } else if (!strchr(slash + 1, '.') && extension) {
        if (!wbl::appendString(path, capacity, extension)) return false;
    }
    return wbl::storagePath(path);
}

bool downloadPath(block *html, const char *url, char *path, size_t capacity)
{
    return completePath(html, url, path, capacity, "download");
}

bool correctPath(char *path, size_t capacity, const char *type, int which)
{
    block html = {};
    if (type && !wbl::copyString(html.type, sizeof(html.type), type)) return false;
    const char *names[] = {"page", "image", "screenshot", "download"};
    if (which < PAGE || which > OTHER) return false;
    return completePath(&html, NULL, path, capacity, names[which]);
}

int strtokcmp(const char *value, const char *compare, const char *separator)
{
    if (!value || !compare || !separator) return -1;
    std::string remaining(compare);
    size_t start = 0;
    while (start < remaining.size()) {
        const size_t end = remaining.find_first_of(separator, start);
        if (!strcasecmp(value, remaining.substr(start, end - start).c_str())) return 0;
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return -1;
}

struct Size imageSize(Tag *tag, GuiImage *img)
{
    Size result = {INT_MAX, INT_MAX};
    if (img && img->GetImage()) result = {img->GetWidth(), img->GetHeight()};
    if (!tag) return result;
    int *dimensions[] = {&result.width, &result.height};
    for (size_t i = 0; i < 2 && i < tag->value.size(); ++i) {
        std::string text = tag->value[i].text;
        const bool percent = !text.empty() && text.back() == '%';
        if (percent) text.pop_back();
        int value;
        if (!wbl::parseInteger(text.c_str(), 1, 4096, value)) continue;
        if (!percent) *dimensions[i] = value;
        else if (*dimensions[i] != INT_MAX)
            *dimensions[i] = static_cast<int>(static_cast<long long>(*dimensions[i]) * value / 100);
    }
    return result;
}
