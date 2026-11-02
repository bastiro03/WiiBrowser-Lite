#include "file_io.h"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace wbl {
bool commitStagedFile(const char *path, const char *pending, bool replaceExisting)
{
    if (!path || !pending) return false;
    struct stat status;
    const std::string backup = std::string(path) + ".bak";
    if (stat(path, &status) == 0) {
        if (!replaceExisting || !S_ISREG(status.st_mode)) return false;
        if (stat(backup.c_str(), &status) == 0 || errno != ENOENT) return false;
        if (std::rename(path, backup.c_str()) != 0) return false;
    } else if (errno != ENOENT) return false;
    return std::rename(pending, path) == 0;
}

bool writeUserFile(const char *path, const void *bytes, std::size_t length, bool replaceExisting)
{
    if (!path || (!bytes && length)) return false;
    const std::string pending = std::string(path) + ".new";
    const int fd = open(pending.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) return false;
    FILE *file = fdopen(fd, "wb");
    if (!file) { close(fd); std::remove(pending.c_str()); return false; }
    if (!writeStreamAndClose(file, bytes, length)) { std::remove(pending.c_str()); return false; }
    return commitStagedFile(path, pending.c_str(), replaceExisting);
}

bool readBounded(const char *path, std::string &bytes, std::size_t maximum)
{
    bytes.clear();
    FILE *file = std::fopen(path, "rb");
    if (!file) return false;
    char buffer[4096];
    bool ok = true;
    try { for (;;) {
        const std::size_t count = std::fread(buffer, 1, sizeof(buffer), file);
        if (count > maximum - bytes.size()) { ok = false; break; }
        bytes.append(buffer, count);
        if (count != sizeof(buffer)) { ok = !std::ferror(file); break; }
    } } catch (...) { ok = false; }
    if (std::fclose(file) != 0) ok = false;
    if (!ok) bytes.clear();
    return ok;
}

bool writeStreamAndClose(void *stream, const void *bytes, std::size_t length)
{
    FILE *file = static_cast<FILE *>(stream);
    if (!file) return false;
    bool ok = (bytes || !length) &&
              (!length || std::fwrite(bytes, 1, length, file) == length);
    if (std::fflush(file) != 0 || std::ferror(file)) ok = false;
    if (std::fclose(file) != 0) ok = false;
    return ok;
}

bool writeRecoverable(const char *path, const void *bytes, std::size_t length)
{
    if (!path || (!bytes && length)) return false;
    const std::string pending = std::string(path) + ".new";
    const std::string backup = std::string(path) + ".bak";
    // Never truncate an unfinished generation left by interrupted saving.
    const int fd = open(pending.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) return false;
    FILE *file = fdopen(fd, "wb");
    if (!file) { close(fd); std::remove(pending.c_str()); return false; }
    if (!writeStreamAndClose(file, bytes, length)) {
        std::remove(pending.c_str());
        return false;
    }
    struct stat status;
    if (stat(path, &status) == 0) {
        if (!S_ISREG(status.st_mode)) return false;
        if (stat(backup.c_str(), &status) == 0) {
            if (!S_ISREG(status.st_mode)) return false;
        } else if (errno != ENOENT) return false;
        if (std::remove(backup.c_str()) != 0 && errno != ENOENT) return false;
        if (std::rename(path, backup.c_str()) != 0) return false;
    } else if (errno != ENOENT) return false;
    // libfat cannot rename over an existing destination. The live path is now
    // absent, and the previous generation remains available even on failure.
    return std::rename(pending.c_str(), path) == 0;
}
}
