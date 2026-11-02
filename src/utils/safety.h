#ifndef WIIBROWSER_SAFETY_H
#define WIIBROWSER_SAFETY_H

#include <cstddef>
#include <string>

namespace wbl {
constexpr std::size_t MaxUrl = 4096;
constexpr std::size_t MaxDocument = 2 * 1024 * 1024;
constexpr std::size_t MaxImageBytes = 4 * 1024 * 1024;
constexpr std::size_t MaxImagePixels = 1024 * 1024;
constexpr std::size_t MaxPageItems = 2048;

bool copyString(char *destination, std::size_t capacity, const char *source);
bool appendString(char *destination, std::size_t capacity, const char *source);
bool safeFilename(const std::string &name);
bool storagePath(const std::string &path);
bool httpUrl(const std::string &url);
bool voidElement(const std::string &name);
bool markupBudget(const std::string &html);
bool parseInteger(const char *text, int minimum, int maximum, int &value);
bool imageSize(std::size_t width, std::size_t height, std::size_t bytesPerPixel,
               std::size_t &bytes);
}

#endif
