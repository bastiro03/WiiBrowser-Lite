/****************************************************************************
 * Copyright (C) 2009
 * by Dimok
 * modified by gave92
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * Settings.cpp
 *
 * Settings Class
 * for WiiBrowser 2012
 ***************************************************************************/
// Altered for the approved release-safety remediation, 2026.
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <new>
#include <sstream>
#include <string>
#include <vector>
#include <mxml.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "settings.h"
#include "menu.h"
#include "config.h"
#include "fileop.h"
#include "safety.h"
#include "file_io.h"
#include "devmount.h"

static mxml_node_t *xml = NULL;
extern std::string ParseList(char *buffer);

namespace {
const size_t MaxBookmarks = 256;
const size_t MaxBookmarkBytes = 512 * 1024;
bool exists(const char *path)
{
    struct stat status;
    return stat(path, &status) == 0;
}
bool directory(const char *path)
{
    struct stat status;
    return stat(path, &status) == 0 && S_ISDIR(status.st_mode);
}
bool noControls(const char *text)
{
    if (!text) return false;
    for (const unsigned char *p = reinterpret_cast<const unsigned char *>(text); *p; ++p)
        if (*p < 32 || *p == 127) return false;
    return true;
}
}

SSettings::SSettings()
{
    strcpy(BootDevice, "sd:/");
    strcpy(AppPath, "sd:/apps/wiibrowser/");
    strcpy(ConfigPath, "sd:/apps/wiibrowser/wiibrowser.cfg");
    SetDefault();
}

SSettings::~SSettings()
{
    free(Favorites);
    if (xml) mxmlDelete(xml);
}

void SSettings::SetDefault()
{
    Language = LANG_ENGLISH;
    UserAgent = LIBCURL;
    Autoupdate = DISABLED;
    ZipFile = NEVER;
    ShowTooltip = Restore = CleanExit = true;
    ShowThumbs = false; // Large raw thumbnails are opt-in, not startup allocations.
    DocWrite = IFrame = ExecLua = MuteSound = false;
    strcpy(Homepage, "https://www.google.com/");
    strcpy(DefaultFolder, "apps/wiibrowser/");
    snprintf(UserFolder, sizeof(UserFolder), "%s", AppPath);
    strcpy(Uuid, "WIIB-00000000");
    strcpy(Revision, "Rev000");
    RevInt = 0;
    Proxy[0] = DownloadFolder[0] = StartPage[0] = 0;
    for (int i = 0; i < N; ++i) {
        TopSites[i] = topSiteStorage[i];
        TopSites[i][0] = 0;
        free(Thumbnails[i]);
        Thumbnails[i] = NULL;
    }
    free(Favorites);
    Favorites = NULL;
    num_fav = 0;
    bookmarksWritable = false;
}

bool SSettings::SetAppPath(const char *executable)
{
    if (!executable || !wbl::storagePath(executable)) return false;
    const std::string path(executable);
    const size_t slash = path.rfind('/');
    if (slash == std::string::npos) return false;
    const std::string parent = path.substr(0, slash + 1);
    const std::string config = parent + "wiibrowser.cfg";
    if (parent.size() >= sizeof(AppPath) || config.size() >= sizeof(ConfigPath) ||
        !directory(parent.c_str())) return false;
    wbl::copyString(AppPath, sizeof(AppPath), parent.c_str());
    wbl::copyString(ConfigPath, sizeof(ConfigPath), config.c_str());
    wbl::copyString(BootDevice, sizeof(BootDevice), path.compare(0, 4, "sd:/") == 0 ? "sd:/" : "usb:/");
    wbl::copyString(UserFolder, sizeof(UserFolder), AppPath);
    appPathChosen = true;
    return true;
}

bool SSettings::FindConfig()
{
    if (appPathChosen) return true;
    const char *paths[] = {"apps/wiibrowser/", "apps/wiibrowser-lite/"};
    for (int device = SD; device <= USB; ++device) {
        for (const char *path : paths) {
            char executable[256];
            snprintf(executable, sizeof(executable), "%s:/%sboot.dol", DeviceName[device], path);
            char candidate[256];
            snprintf(candidate, sizeof(candidate), "%s:/%swiibrowser.cfg", DeviceName[device], path);
            if (exists(candidate) && SetAppPath(executable)) return true;
        }
    }
    for (int device = SD; device <= USB; ++device) {
        for (const char *path : paths) {
            char executable[256];
            snprintf(executable, sizeof(executable), "%s:/%sboot.dol", DeviceName[device], path);
            if (SetAppPath(executable)) return true;
        }
    }
    return false;
}

bool SSettings::Load()
{
    if (!FindConfig()) { configWritable = false; CreateXMLFile(); return false; }
    configWritable = true;
    std::string content;
    const bool present = exists(ConfigPath);
    std::string source(ConfigPath);
    if (!CheckIntegrity(source.c_str())) {
        if (present) configWritable = false; // Never auto-repair over user state.
        const char *suffixes[] = {".new", ".bak"};
        for (const char *suffix : suffixes) {
            const std::string candidate = std::string(ConfigPath) + suffix;
            if (CheckIntegrity(candidate.c_str())) { source = candidate; configWritable = false; break; }
        }
    }
    if (CheckIntegrity(source.c_str()) && wbl::readBounded(source.c_str(), content, 64 * 1024)) {
        std::istringstream input(content);
        std::string line;
        while (std::getline(input, line)) {
            if (line.size() >= 1024) { configWritable = false; continue; }
            char buffer[1024];
            wbl::copyString(buffer, sizeof(buffer), line.c_str());
            if (buffer[0] != '#') ParseLine(buffer);
        }
    } else if (present) configWritable = false;
    ChangeFolder();
    LoadFavorites();
    // Raw thumbnails are a fixed 640x480 format. Do not delete unknown/stale
    // files while loading, and never allocate all nine thumbnails by default.
    return configWritable;
}

bool SSettings::Save(bool clean)
{
    if (!configWritable || !FindConfig() || !MountManager_IsInserted(AppPath)) return false;
    std::ostringstream data;
    data << "# WiiBrowser Settingsfile\n# Format version 1\n"
         << "Language = " << Language << "\nRevision = " << RevInt
         << "\nAutoupdate = 0\nUnzipFile = 0\nMuteSound = " << MuteSound
         << "\nShowTooltip = " << ShowTooltip << "\nShowThumbnails = " << ShowThumbs
         << "\nRestoreSession = " << Restore << "\nDefaultFolder = " << DefaultFolder
         << "\nHomepage = " << Homepage << '\n';
    for (int i = 0; i < N; ++i) data << "Favorite(" << i << ") = " << TopSites[i] << '\n';
    data << "UserAgent = " << UserAgent << "\nIFrame = 0\nDocWrite = 0\nExecLua = 0\nCleanExit = "
         << clean << "\nProxy = " << Proxy << '\n';
    const std::string serialized = data.str();
    if (!wbl::writeRecoverable(ConfigPath, serialized.data(), serialized.size())) return false;
    return !bookmarksWritable || SaveFavorites();
}

bool SSettings::Reset()
{
    // Explicit recovery must be performed by the user on the storage device;
    // resetting the running UI must not authorize overwriting invalid files.
    SetDefault();
    CreateXMLFile();
    return false;
}

bool SSettings::SetStartup(char *name, char *value)
{
    if (!name || !value || !noControls(value)) return false;
    if (!strcmp(name, "StartPage")) return wbl::copyString(StartPage, sizeof(StartPage), value);
    if (!strcmp(name, "DownloadFolder")) return wbl::copyString(DownloadFolder, sizeof(DownloadFolder), value);
    return false;
}

bool SSettings::SetSetting(char *name, char *value)
{
    if (!name || !value || !noControls(value)) return false;
    int number = 0;
    if (!strcmp(name, "Language")) return wbl::parseInteger(value, 0, LANG_LENGTH - 1, Language);
    if (!strcmp(name, "UserAgent")) return wbl::parseInteger(value, 0, MAXAGENTS - 1, UserAgent);
    if (!strcmp(name, "Revision")) return wbl::parseInteger(value, 0, INT_MAX, RevInt);
    if (!strcmp(name, "Autoupdate") || !strcmp(name, "UnzipFile") || !strcmp(name, "IFrame") ||
        !strcmp(name, "DocWrite") || !strcmp(name, "ExecLua")) return true; // Disabled regardless of file contents.
    bool *setting = NULL;
    if (!strcmp(name, "MuteSound")) setting = &MuteSound;
    else if (!strcmp(name, "ShowTooltip")) setting = &ShowTooltip;
    else if (!strcmp(name, "ShowThumbnails")) setting = &ShowThumbs;
    else if (!strcmp(name, "RestoreSession")) setting = &Restore;
    else if (!strcmp(name, "CleanExit")) setting = &CleanExit;
    if (setting) {
        if (!wbl::parseInteger(value, 0, 1, number)) return false;
        *setting = number != 0;
        return true;
    }
    if (!strcmp(name, "Homepage")) return wbl::copyString(Homepage, sizeof(Homepage), value);
    if (!strcmp(name, "Proxy")) return wbl::copyString(Proxy, sizeof(Proxy), value);
    if (!strcmp(name, "DefaultFolder")) {
        if (!wbl::copyString(DefaultFolder, sizeof(DefaultFolder), value)) return false;
        ChangeFolder();
        return true;
    }
    const std::string key(name);
    if (key.compare(0, 9, "Favorite(") == 0 && key.back() == ')') {
        const std::string index = key.substr(9, key.size() - 10);
        if (!wbl::parseInteger(index.c_str(), 0, N - 1, number)) return false;
        return wbl::copyString(TopSites[number], sizeof(topSiteStorage[number]), value);
    }
    return false;
}

void SSettings::ParseLine(char *line)
{
    if (!line) return;
    char *equals = strchr(line, '=');
    if (!equals) return;
    *equals = 0;
    char name[1024], value[1024];
    TrimLine(name, line, sizeof(name));
    TrimLine(value, equals + 1, sizeof(value));
    if (!SetSetting(name, value)) SetStartup(name, value);
}

void SSettings::TrimLine(char *destination, char *source, int capacity)
{
    if (!destination || !source || capacity <= 0) return;
    while (*source == ' ' || *source == '\t') ++source;
    size_t length = strlen(source);
    while (length && strchr(" \t\r\n", source[length - 1])) --length;
    if (length >= static_cast<size_t>(capacity)) { destination[0] = 0; return; }
    memcpy(destination, source, length);
    destination[length] = 0;
}

bool SSettings::CheckFile(const char *path) { return path && exists(path); }
bool SSettings::IsWritable(const char *path)
{
    if (!path) return false;
    const std::string probe = std::string(path) + ".probe";
    const int fd = open(probe.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) return false;
    const bool closed = close(fd) == 0;
    const bool removed = remove(probe.c_str()) == 0;
    return closed && removed;
}

bool SSettings::CheckIntegrity(const char *path)
{
    std::string data;
    return path && wbl::readBounded(path, data, 64 * 1024) &&
           data.compare(0, strlen("# WiiBrowser Settingsfile"), "# WiiBrowser Settingsfile") == 0 &&
           data.find('\0') == std::string::npos;
}

favorite *SSettings::GetFav(int index) { return index >= 0 && index < num_fav ? &Favorites[index] : NULL; }
char *SSettings::GetUrl(int index) { return index >= 0 && index < N ? TopSites[index] : NULL; }
int SSettings::FindUrl(char *url)
{
    if (url && *url) for (int i = 0; i < N; ++i) if (!strcmp(TopSites[i], url)) return i;
    return -1;
}
void SSettings::Remove(int index, bool update)
{
    if (index < 0 || index >= N) return;
    if (!update) TopSites[index][0] = 0;
    free(Thumbnails[index]);
    Thumbnails[index] = NULL;
}

void SSettings::ChangeFolder()
{
    std::string folder(DefaultFolder);
    if (!wbl::storagePath(folder)) {
        if (!folder.empty() && folder[0] == '/') folder.erase(0, 1);
        folder = std::string(BootDevice) + folder;
    }
    if (!wbl::storagePath(folder) || !wbl::copyString(UserFolder, sizeof(UserFolder), folder.c_str()))
        wbl::copyString(UserFolder, sizeof(UserFolder), AppPath);
    const size_t length = strlen(UserFolder);
    if (length > strlen(BootDevice) && UserFolder[length - 1] == '/') UserFolder[length - 1] = 0;
}

void SSettings::SetStartPage(char *page)
{
    if (!page) return;
    if (!strcasecmp(page, "homepage")) wbl::copyString(StartPage, sizeof(StartPage), Homepage);
    else if (wbl::httpUrl(page)) wbl::copyString(StartPage, sizeof(StartPage), page);
    else {
        const std::string text(page);
        int index;
        if (text.compare(0, 9, "bookmark_") == 0 &&
            wbl::parseInteger(text.c_str() + 9, 0, N - 1, index))
            wbl::copyString(StartPage, sizeof(StartPage), TopSites[index]);
    }
    // Do not interpret arbitrary argv files as writable configuration commands.
}
int SSettings::GetStartPage(char *destination)
{
    if (!StartPage[0] || !wbl::copyString(destination, 512, StartPage)) return MENU_HOME;
    return MENU_BROWSE;
}

bool SSettings::LoadFavorites()
{
    const std::string path = std::string(AppPath) + "appdata/bookmarks.html";
    bookmarksWritable = !exists(path.c_str());
    const char *suffixes[] = {"", ".new", ".bak"};
    for (const char *suffix : suffixes) {
        std::string data;
        if (!wbl::readBounded((path + suffix).c_str(), data, MaxBookmarkBytes) ||
            data.empty() || data.find('\0') != std::string::npos) continue;
        if ((!strcasestr(data.c_str(), "<!doctype netscape-bookmark-file-1>") && data.find("<?xml") == std::string::npos) ||
            !strcasestr(data.c_str(), "<dl") || !strcasestr(data.c_str(), "</dl>")) continue;
        mxml_node_t *loaded = NULL;
        try {
            const std::string normalized = ParseList(&data[0]);
            const size_t marker = normalized.find("<?xml");
            if (marker == std::string::npos) continue;
            loaded = mxmlLoadString(NULL, normalized.c_str() + marker, MXML_OPAQUE_CALLBACK);
        } catch (...) { continue; }
        if (!loaded) continue;
        std::vector<favorite> values;
        bool valid = true;
        for (mxml_node_t *entry = mxmlFindElement(loaded, loaded, "a", "href", NULL, MXML_DESCEND);
             entry; entry = mxmlFindElement(entry, loaded, "a", "href", NULL, MXML_DESCEND)) {
            const char *url = mxmlElementGetAttr(entry, "href");
            mxml_node_t *child = mxmlGetFirstChild(entry);
            const char *title = child ? mxmlGetOpaque(child) : NULL;
            if (!title && child) title = mxmlGetText(child, NULL);
            favorite value = {};
            if (values.size() >= MaxBookmarks || !url || !wbl::httpUrl(url) ||
                !wbl::copyString(value.url, sizeof(value.url), url) ||
                !wbl::copyString(value.name, sizeof(value.name), title ? title : "")) { valid = false; break; }
            values.push_back(value);
        }
        if (!valid) { mxmlDelete(loaded); continue; }
        favorite *allocation = values.empty() ? NULL : static_cast<favorite *>(malloc(values.size() * sizeof(favorite)));
        if (!values.empty() && !allocation) { mxmlDelete(loaded); break; }
        if (allocation) memcpy(allocation, values.data(), values.size() * sizeof(favorite));
        free(Favorites);
        Favorites = allocation;
        num_fav = static_cast<int>(values.size());
        if (xml) mxmlDelete(xml);
        xml = loaded;
        bookmarksWritable = !*suffix;
        return true;
    }
    const bool fresh = !exists(path.c_str()) && !exists((path + ".new").c_str()) && !exists((path + ".bak").c_str());
    CreateXMLFile();
    bookmarksWritable = fresh;
    return false;
}

int SSettings::IsBookmarked(char *url)
{
    if (url) for (int i = 0; i < num_fav; ++i) if (!strcmp(Favorites[i].url, url)) return i;
    return -1;
}

bool SSettings::AddFavorite(char *url, char *title)
{
    if (!bookmarksWritable || !xml || !url || !wbl::httpUrl(url) || num_fav >= static_cast<int>(MaxBookmarks)) return false;
    favorite value = {};
    if (!wbl::copyString(value.url, sizeof(value.url), url) ||
        !wbl::copyString(value.name, sizeof(value.name), title ? title : "")) return false;
    favorite *next = static_cast<favorite *>(realloc(Favorites, (num_fav + 1) * sizeof(favorite)));
    if (!next) return false;
    Favorites = next;
    mxml_node_t *list = mxmlFindElement(xml, xml, "dl", NULL, NULL, MXML_DESCEND);
    if (!list) return false;
    mxml_node_t *item = mxmlNewElement(list, "dt");
    if (!item) return false;
    mxml_node_t *anchor = mxmlNewElement(item, "a");
    if (!anchor) { mxmlDelete(item); return false; }
    mxmlElementSetAttr(anchor, "href", value.url);
    if (!mxmlNewText(anchor, 0, value.name)) { mxmlDelete(item); return false; }
    Favorites[num_fav++] = value;
    return SaveFavorites();
}

void SSettings::DelFavorite(char *url)
{
    if (!bookmarksWritable || !xml) return;
    const int index = IsBookmarked(url);
    if (index < 0) return;
    for (int i = index; i + 1 < num_fav; ++i) Favorites[i] = Favorites[i + 1];
    --num_fav;
    mxml_node_t *entry = mxmlFindElement(xml, xml, "a", "href", url, MXML_DESCEND);
    if (entry) mxmlDelete(mxmlGetParent(entry));
    SaveFavorites();
}

bool SSettings::SaveFavorites()
{
    if (!bookmarksWritable || !xml || !MountManager_IsInserted(AppPath)) return false;
    char *serialized = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
    if (!serialized) return false;
    const std::string directoryPath = std::string(AppPath) + "appdata/";
    const std::string path = directoryPath + "bookmarks.html";
    const std::string data = std::string("<!DOCTYPE NETSCAPE-Bookmark-file-1>\n") + serialized;
    free(serialized);
    return data.size() <= MaxBookmarkBytes && CreateSubfolder(directoryPath.c_str()) &&
           wbl::writeRecoverable(path.c_str(), data.data(), data.size());
}

bool SSettings::CreateXMLFile()
{
    if (xml) mxmlDelete(xml);
    xml = mxmlNewXML("1.0");
    if (!xml) return false;
    mxmlSetWrapMargin(0);
    mxml_node_t *list = mxmlNewElement(xml, "dl");
    return list != NULL;
}

int CheckFolder(const char *folder)
{
    if (!folder || !*folder) return FILENAME;
    if (wbl::storagePath(folder)) return ABSOLUTE;
    return folder[0] == '/' ? RELATIVE : FILENAME;
}

off_t GetFileSize(const char *filename)
{
    struct stat status;
    return filename && stat(filename, &status) == 0 && S_ISREG(status.st_mode) ? status.st_size : 0;
}

void *LoadFile(char *path, off_t length)
{
    if (!path || length < 0 || static_cast<unsigned long long>(length) > wbl::MaxImageBytes) return NULL;
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;
    unsigned char *bytes = static_cast<unsigned char *>(malloc(static_cast<size_t>(length) + 1));
    if (!bytes) { fclose(file); return NULL; }
    const bool read = fread(bytes, 1, length, file) == static_cast<size_t>(length) && !ferror(file);
    const bool closed = fclose(file) == 0;
    if (!read || !closed) { free(bytes); return NULL; }
    bytes[length] = 0;
    return bytes;
}

void WriteFile(char *path, int length, void *bytes)
{
    if (length >= 0) wbl::writeRecoverable(path, bytes, static_cast<size_t>(length));
}
