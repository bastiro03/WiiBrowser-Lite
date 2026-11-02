#ifndef WIIBROWSER_FILE_IO_H
#define WIIBROWSER_FILE_IO_H
#include <cstddef>
#include <string>

namespace wbl {
// Leaves a previous generation at .bak. A failed final rename retains both
// .new and .bak; it never removes the only verified generation.
bool writeRecoverable(const char *path, const void *bytes, std::size_t length);
bool readBounded(const char *path, std::string &bytes, std::size_t maximum);
bool writeStreamAndClose(void *stream, const void *bytes, std::size_t length);
bool commitStagedFile(const char *path, const char *pending, bool replaceExisting);
bool writeUserFile(const char *path, const void *bytes, std::size_t length, bool replaceExisting);
}
#endif
