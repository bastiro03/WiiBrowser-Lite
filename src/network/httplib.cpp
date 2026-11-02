/*
 * Created by gave92. Copyright (C) 2012-2013.
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation. Distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>
#include "httplib.h"
#include "main.h"
#include "menu.h"
#include "safety.h"
#include "file_io.h"

const block emptyblock = {NULL, 0, {0}};

namespace {
struct Response {
    char *bytes = NULL;
    size_t length = 0;
    size_t headerBytes = 0;
    long status = 0;
    bool attachment = false;
    bool stoppedForAttachment = false;
    char type[TYPE] = {};
    char filename[256] = {};
    ~Response() { free(bytes); }
};

// Only the navigation thread accesses this handoff. Passive image/download
// requests cannot interpret URL text as a command or consume form bodies.
std::string pendingUrl;
std::string pendingBody;
struct Cancellation { bool (*test)(void *); void *context; };

size_t receiveBody(void *contents, size_t size, size_t count, void *opaque)
{
    Response &response = *static_cast<Response *>(opaque);
    if (size && count > wbl::MaxDocument / size) return 0;
    const size_t length = size * count;
    if (response.length >= wbl::MaxDocument ||
        length > wbl::MaxDocument - response.length - 1) return 0;
    char *next = static_cast<char *>(realloc(response.bytes, response.length + length + 1));
    if (!next) return 0;
    response.bytes = next;
    memcpy(next + response.length, contents, length);
    response.length += length;
    next[response.length] = 0;
    return length;
}

size_t receiveHeader(void *contents, size_t size, size_t count, void *opaque)
{
    try {
    Response &response = *static_cast<Response *>(opaque);
    if (size && count > 65536 / size) return 0;
    const size_t length = size * count;
    if (length > 65536 - response.headerBytes) return 0;
    response.headerBytes += length;
    std::string line(static_cast<const char *>(contents), length);
    if (line.compare(0, 5, "HTTP/") == 0) {
        const size_t space = line.find(' ');
        response.status = space == std::string::npos ? 0 : strtol(line.c_str() + space + 1, NULL, 10);
        response.type[0] = response.filename[0] = 0;
        response.attachment = response.stoppedForAttachment = false;
        return length;
    }
    const size_t colon = line.find(':');
    std::string key = line.substr(0, colon);
    for (char &c : key) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (key == "content-type") {
        const size_t start = line.find_first_not_of(" \t", colon + 1);
        const size_t end = line.find_first_of(";\r\n \t", start);
        if (start != std::string::npos) {
            std::string type = line.substr(start, end - start);
            for (char &c : type) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (!wbl::copyString(response.type, sizeof(response.type), type.c_str())) return 0;
            response.attachment = type != "text/html" && type != "application/xhtml+xml" &&
                                  type.compare(0, 6, "image/") != 0 && type.compare(0, 5, "text/") != 0;
        }
    } else if (key == "content-disposition") {
        const size_t field = line.find("filename=");
        if (field != std::string::npos) {
            size_t start = field + 9;
            const bool quoted = start < line.size() && line[start] == '"';
            if (quoted) ++start;
            const size_t end = line.find_first_of(quoted ? "\"\r\n" : ";\r\n", start);
            const std::string name = line.substr(start, end - start);
            // Invalid suggestions are not allowed to become filesystem paths.
            if (!wbl::safeFilename(name) ||
                !wbl::copyString(response.filename, sizeof(response.filename), name.c_str())) return 0;
        }
    }
    if (line == "\r\n" && response.status >= 200 && response.status < 300 && response.attachment) {
        response.stoppedForAttachment = true;
        return 0;
    }
    return length;
    } catch (...) { return 0; } // Never unwind a C++ exception through libcurl.
}

int progress(void *opaque, double, double, double, double)
{
    Cancellation *cancellation = static_cast<Cancellation *>(opaque);
    return App_Exiting() || (cancellation && cancellation->test && cancellation->test(cancellation->context)) || CancelDownload();
}

block request(CURL *handle, const char *url, const char *body, FILE *file,
              bool (*cancelled)(void *) = NULL, void *context = NULL)
{
    if (!handle || !url || !wbl::httpUrl(url) ||
        (body && strlen(body) > wbl::MaxDocument)) return emptyblock;
    curl_easy_reset(handle);
    ConfigureHttp(handle, url);
    Response response;
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, progress);
    Cancellation cancellation = {cancelled, context};
    curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, &cancellation);
    if (body) {
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, static_cast<long>(strlen(body)));
        // Never replay a credential-bearing body to a redirect target.
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 0L);
    }
    if (file) {
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteHttpFile);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
    } else {
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, receiveBody);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, receiveHeader);
        curl_easy_setopt(handle, CURLOPT_HEADERDATA, &response);
    }
    const CURLcode result = curl_easy_perform(handle);
    block output = emptyblock;
    if (result == CURLE_WRITE_ERROR && response.stoppedForAttachment && !body) {
        output.size = -1;
        output.data = strdup(response.filename);
        wbl::copyString(output.type, sizeof(output.type), response.type);
        return output;
    }
    if (result != CURLE_OK) return output;
    long status = 0;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status);
    if (status < 200 || status >= 300) return output;
    if (file) {
        if (fflush(file) == 0 && !ferror(file)) output.size = -2;
        return output; // Stream remains caller-owned.
    }
    char *type = NULL;
    curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &type);
    std::string media = type ? type : "text/plain";
    media.erase(media.find(';') == std::string::npos ? media.size() : media.find(';'));
    if (!wbl::copyString(output.type, sizeof(output.type), media.c_str())) return emptyblock;
    output.data = response.bytes;
    output.size = static_cast<int>(response.length);
    response.bytes = NULL;
    return output;
}
}

void ConfigureHttp(CURL *handle, const char *url)
{
    std::string certificates(Settings.AppPath);
    if (certificates.empty() || certificates.back() != '/') certificates += '/';
    certificates += "cacert.pem";
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "WiiBrowser-Lite/0.1");
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(handle, CURLOPT_CAINFO, certificates.c_str());
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, 32L);
    curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, 10L);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(handle, CURLOPT_AUTOREFERER, 0L);
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
#if LIBCURL_VERSION_NUM >= 0x075500
    curl_easy_setopt(handle, CURLOPT_PROTOCOLS_STR, "http,https");
    // HTTPS requests must not redirect to plaintext.
    curl_easy_setopt(handle, CURLOPT_REDIR_PROTOCOLS_STR, strncmp(url, "https:", 6) == 0 ? "https" : "http,https");
#else
    curl_easy_setopt(handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    curl_easy_setopt(handle, CURLOPT_REDIR_PROTOCOLS, strncmp(url, "https:", 6) == 0 ? CURLPROTO_HTTPS : CURLPROTO_HTTP | CURLPROTO_HTTPS);
#endif
    if (validProxy()) curl_easy_setopt(handle, CURLOPT_PROXY, Settings.Proxy);
}

size_t WriteHttpFile(void *bytes, size_t size, size_t count, void *stream)
{
    if (!stream || (size && count > static_cast<size_t>(-1) / size)) return 0;
    return fwrite(bytes, 1, size * count, static_cast<FILE *>(stream));
}

block downloadfile(CURL *handle, const char *url, FILE *file, bool (*cancelled)(void *), void *context)
{
    return request(handle, url, NULL, file, cancelled, context);
}

block downloadpost(CURL *handle, const char *url, const char *body)
{
    return request(handle, url, body ? body : "", NULL);
}

void SetNavigationPost(const std::string &url, const std::string &body)
{
    pendingUrl = url;
    pendingBody = body;
}

bool TakeNavigationPost(const char *url, std::string &body)
{
    const bool match = url && pendingUrl == url;
    if (match) body.swap(pendingBody);
    pendingUrl.clear();
    pendingBody.clear();
    return match;
}

bool postcomment(CURL *, char *, char *)
{
    // The inherited third-party HTTP comment endpoint is not a release bug
    // reporting service. Do not transmit reports or console identifiers to it.
    return false;
}

bool save(block *data, FILE *file)
{
    return wbl::writeStreamAndClose(file, data ? data->data : NULL,
                                    data && data->size > 0 ? data->size : 0);
}

bool validProxy() { return Settings.Proxy[0] != 0; }
char *findChr(char *text, char character) { char *p = text ? strchr(text, character) : NULL; if (p) *p = 0; return p; }
char *findRchr(char *text, char character) { char *p = text ? strrchr(text, character) : NULL; if (p) *p = 0; return p; }
void Debug(const char *) {}
void DebugInt(u32) {}
