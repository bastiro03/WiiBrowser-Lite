#include <algorithm>
#include <atomic>
#include <list>
#include <map>
#include <new>
#include <sys/stat.h>
#include "main.h"
#include "fileop.h"
#include "transfer.h"
#include "networkop.h"
#include "safety.h"
#include "gui_lock.h"

u8 downloadstack[GUITH_STACK] ATTRIBUTE_ALIGN(32);
lwp_t downloadthread = LWP_THREAD_NULL;
GuiDownloadManager *manager = NULL;
extern GuiToolbar *App;

namespace {
std::atomic<bool> stop(false);
std::atomic<bool> ready(false);
mutex_t queueMutex;
std::list<Private *> queue;
std::map<CURL *, Private *> active;

void releaseJob(Private *job)
{
    if (job->save.file) fclose(job->save.file);
    job->save.file = NULL;
    free(job->url);
    delete job;
}

size_t receiveFile(void *bytes, size_t size, size_t count, void *opaque)
{
    Private *job = static_cast<Private *>(opaque);
    const size_t limit = 1024u * 1024u * 1024u;
    if (!job || !job->save.file || (size && count > limit / size)) return 0;
    const size_t length = size * count;
    if (job->bytes > limit || length > limit - static_cast<size_t>(job->bytes)) return 0;
    const size_t written = WriteHttpFile(bytes, 1, length, job->save.file);
    job->bytes += written;
    return written;
}

int progress(void *opaque, double total, double done, double, double)
{
    if (stop || ExitAccepted || HWButton) return 1;
    Private *job = static_cast<Private *>(opaque);
    GuiScopedLock lock;
    if (!manager || !job->bar) return 1;
    manager->SetProgress(job, total > 0 ? std::max(0.0, std::min(100.0, done * 100.0 / total)) : 0);
    return manager->CancelDownload(job->bar) ? 1 : 0;
}

bool commit(Private *job)
{
    struct stat status;
    const std::string backup = std::string(job->save.name) + ".download.bak";
    if (stat(job->save.name, &status) == 0) {
        if (!job->save.overwrite || !S_ISREG(status.st_mode)) return false;
        // Never overwrite an earlier backup or an unrelated file at that name.
        if (stat(backup.c_str(), &status) == 0 || errno != ENOENT) return false;
        if (rename(job->save.name, backup.c_str()) != 0) return false;
    } else if (errno != ENOENT) return false;
    return rename(job->partial, job->save.name) == 0;
}

void complete(CURL *handle, CURLcode result)
{
    const auto found = active.find(handle);
    if (found == active.end()) return;
    Private *job = found->second;
    long response = 0;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response);
    curl_multi_remove_handle(curl_multi, handle);
    curl_easy_cleanup(handle);
    active.erase(found);
    bool written = job->save.file != NULL;
    if (job->save.file) {
        if (fflush(job->save.file) != 0 || ferror(job->save.file)) written = false;
        if (fclose(job->save.file) != 0) written = false;
        job->save.file = NULL;
    }
    const bool success = result == CURLE_OK && response >= 200 && response < 300 && written && commit(job);
    {
        GuiScopedLock lock;
        if (manager && job->bar) manager->RemoveBar(job->bar);
        job->bar = NULL;
        if (App && !stop && !ExitAccepted)
            App->SaveTooltip->SetTimeout(success ? "Download saved" : "Download failed; partial retained", 3);
    }
    // No implicit extraction, source deletion, automatic resume, stack reset,
    // or application exit. Failed transfers retain their .part file.
    job->code = success ? CURLE_OK : CURLE_WRITE_ERROR;
    releaseJob(job);
}

bool start(Private *job)
{
    CURL *handle = curl_easy_init();
    if (!handle) return false;
    const int fd = open(job->partial, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) { curl_easy_cleanup(handle); return false; }
    job->save.file = fdopen(fd, "wb");
    if (!job->save.file) {
        close(fd);
        remove(job->partial); // Only the file just created by this call.
        curl_easy_cleanup(handle);
        return false;
    }
    setvbuf(job->save.file, NULL, _IONBF, 0);
    {
        GuiScopedLock lock;
        job->bar = manager ? manager->CreateBar() : NULL;
    }
    if (!job->bar) { fclose(job->save.file); job->save.file = NULL; curl_easy_cleanup(handle); return false; }
    ConfigureHttp(handle, job->url);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 0L); // Low-speed and connect deadlines remain active.
    curl_easy_setopt(handle, CURLOPT_MAXFILESIZE_LARGE, static_cast<curl_off_t>(1024) * 1024 * 1024);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, receiveFile);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, job);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, progress);
    curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, job);
    if (curl_multi_add_handle(curl_multi, handle) != CURLM_OK) {
        GuiScopedLock lock;
        manager->RemoveBar(job->bar);
        job->bar = NULL;
        curl_easy_cleanup(handle);
        return false;
    }
    active[handle] = job;
    return true;
}
}

bool InitTransfers()
{
    if (ready) return true;
    if (LWP_MutexInit(&queueMutex, false) != 0) return false;
    curl_multi = curl_multi_init();
    if (!curl_multi) { LWP_MutexDestroy(queueMutex); return false; }
    manager = new (std::nothrow) GuiDownloadManager;
    if (!manager) {
        curl_multi_cleanup(curl_multi);
        curl_multi = NULL;
        LWP_MutexDestroy(queueMutex);
        return false;
    }
    stop = false;
    ready = true;
    return true;
}

Private *AddDownload(CURLM *, char *url, file *destination)
{
    if (!ready || stop || !destination || !url || !wbl::httpUrl(url) ||
        !wbl::storagePath(destination->name)) return NULL;
    Private *job = new (std::nothrow) Private{};
    if (!job) return NULL;
    job->url = strdup(url);
    job->save = *destination;
    job->save.file = NULL;
    job->code = -1;
    const int length = snprintf(job->partial, sizeof(job->partial), "%s.part", destination->name);
    if (!job->url || length < 0 || static_cast<size_t>(length) >= sizeof(job->partial)) {
        releaseJob(job);
        return NULL;
    }
    LWP_MutexLock(queueMutex);
    if (queue.size() >= 16) {
        LWP_MutexUnlock(queueMutex);
        releaseJob(job);
        return NULL;
    }
    queue.push_back(job);
    LWP_MutexUnlock(queueMutex);
    return job; // Ownership transferred. Caller must not retain/use this pointer.
}

Private *AddUpdate(CURLM *, char *, FILE *) { return NULL; }

void *DownloadThread(void *)
{
    while (!ready && !stop) usleep(1000);
    if (!ready) return NULL;
    while (!stop) {
        while (active.size() < MAXD) {
            LWP_MutexLock(queueMutex);
            Private *job = queue.empty() ? NULL : queue.front();
            if (job) queue.pop_front();
            LWP_MutexUnlock(queueMutex);
            if (!job) break;
            if (!start(job)) {
                GuiScopedLock lock;
                if (App) App->SaveTooltip->SetTimeout("Cannot create partial file; choose another name", 3);
                releaseJob(job);
            }
        }
        int running = 0;
        CURLMcode code;
        do { code = curl_multi_perform(curl_multi, &running); } while (code == CURLM_CALL_MULTI_PERFORM && !stop);
        if (code != CURLM_OK && code != CURLM_CALL_MULTI_PERFORM) {
            while (!active.empty()) complete(active.begin()->first, CURLE_RECV_ERROR);
        }
        int pending = 0;
        while (CURLMsg *message = curl_multi_info_read(curl_multi, &pending)) {
            if (message->msg == CURLMSG_DONE) {
                // Copy BEFORE either remove-handle or easy-cleanup invalidates it.
                CURL *handle = message->easy_handle;
                const CURLcode result = message->data.result;
                complete(handle, result);
            }
        }
        usleep(active.empty() ? 20000 : 10000);
    }
    while (!active.empty()) complete(active.begin()->first, CURLE_ABORTED_BY_CALLBACK);
    LWP_MutexLock(queueMutex);
    for (Private *job : queue) releaseJob(job);
    queue.clear();
    LWP_MutexUnlock(queueMutex);
    curl_multi_cleanup(curl_multi);
    curl_multi = NULL;
    { GuiScopedLock lock; delete manager; manager = NULL; }
    ready = false;
    LWP_MutexDestroy(queueMutex);
    return NULL;
}

void StopDownload() { stop = true; }
