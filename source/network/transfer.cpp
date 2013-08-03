#include <list>
#include "main.h"
#include "transfer.h"
#include "networkop.h"
#include "fileop.h"

using namespace std;
extern GuiToolbar * App;
off_t GetFileSize(const char *filename);

u8 downloadstack[GUITH_STACK] ATTRIBUTE_ALIGN (32);
lwp_t downloadthread = LWP_THREAD_NULL;
GuiDownloadManager *manager = NULL;

static int downloadThreadHalt = 0;
static list<Private *> queue;

bool AddHandle(Private *data);
void PopQueue();

enum
{
    FAILED,
    PARTIAL,
    COMPLETE,
    CANCELLED
} download_status;

/****************************************************************************
 * DownloadThread
 *
 * Thread to handle file downloads.
 ***************************************************************************/
int showmultiprogress(void *bar,
                    double total, /* dltotal */
                    double done, /* dlnow */
                    double ultotal,
                    double ulnow)
{
    Private *data = (Private *)bar;
    manager->SetProgress(data, done*100.0/total);

    if(manager->CancelDownload(data->bar))
        return 1;
    return 0;
}

int netclose_callback (void *clientp, curl_socket_t item) {
	return net_close(item);
}

static size_t writedata(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

Private *PushQueue(CURLM *cm, char *url, file *file, bool keep)
{
    if(!file || !cm)
        return NULL;

    Private *data = new Private;
    data->bar = NULL;
    data->url = strdup(url);
    data->keep = keep;
    data->code = -1;

    data->save.file = file->file;
    strcpy(data->save.name, file->name);

    queue.push_back(data);
    LWP_ResumeThread(downloadthread);
    return data;
}

void PopQueue()
{
    queue.pop_front();
}

void UpdateQueue()
{
    while(!queue.empty())
    {
        if(AddHandle(queue.front()))
            PopQueue();
        else break;
    }
}

void CompleteDownload(CURLMsg *msg)
{
    char *name;
    char message[256];
    long http_response;

    double dl_bytes_total;
    int retval = -1;

    Private *data;
    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &data);
    curl_easy_getinfo(msg->easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dl_bytes_total);
    curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_response);
    curl_easy_cleanup(msg->easy_handle);

    manager->RemoveBar(data->bar);
    data->code = msg->data.result;
    fclose(data->save.file);

    switch(data->code)
    {
        case CURLE_OK:
            name = strrchr(data->save.name, '.');
            retval = COMPLETE;

            if(name && !stricmp(name, ".zip"))
            {
                if(Settings.ZipFile == UNZIP ||
                        WindowPrompt("Download complete", "You downloaded a zip archive. Unzip it?", "Yes", "No"))
                    UnzipArchive(data->save.name);
            }
        break;

        case CURLE_ABORTED_BY_CALLBACK:
            retval = CANCELLED;
            remove(data->save.name);
        break;

        case CURLE_PARTIAL_FILE:
        case CURLE_OPERATION_TIMEDOUT:
            switch(http_response)
            {
                case 0:     // eg connection down from kick-off: suggest retrying till some max limit
                case 200:   // yay we at least got to our url
                case 206:   // Partial Content
                    data->save.file = NULL;
                    queue.push_back(data);
                    InitNetwork();
                    LWP_ResumeThread(downloadthread);
                    retval = PARTIAL;
                break;

                case 416:
                    // cannot d/l range: either cos no server support
                    // or cos we're asking for an invalid range (ie: we already d/ld the file)
                    if(GetFileSize(data->save.name) < dl_bytes_total)
                        retval = FAILED;
                    else retval = COMPLETE;
                break;

                default: // suggest quitting on an unhandled error
                    retval = FAILED;
                break;
            };
        break;

        default:
            retval = FAILED;
        break;
    }

    if(retval == FAILED)
    {
        name = strrchr(data->save.name, '/')+1;
        sprintf(message, "The download of %s failed. Try again?", name);

        if(WindowPrompt("Download failed", message, "Yes", "No"))
        {
            data->save.file = fopen(data->save.name, "wb");
            queue.push_back(data);
            InitNetwork();
            LWP_ResumeThread(downloadthread);
            retval = PARTIAL;
        }
        else remove(data->save.name);
    }

    if(retval != PARTIAL)
    {
        if(!data->keep)
        {
            free(data->url);
            delete(data);
        }
    }
}

bool AddHandle(Private *data)
{
    data->bar = manager->CreateBar();
    if(!data->bar)
        return false;

    CURL *eh = curl_easy_init();
    if(!eh)
        return false;

    /* get the new filesize & sanity check for file */
    if(!data->save.file)
    {
        curl_easy_setopt(eh, CURLOPT_RESUME_FROM, GetFileSize(data->save.name));
        data->save.file = fopen(data->save.name, "ab");
    }

    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, writedata);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *)data->save.file);
    curl_easy_setopt(eh, CURLOPT_URL, data->url);

    /* set proxy if specified */
    if(validProxy())
        curl_easy_setopt(eh, CURLOPT_PROXY, Settings.Proxy);

    /* the follwoing sets up min download speed threshold & time endured before aborting */
    curl_easy_setopt(eh, CURLOPT_LOW_SPEED_LIMIT, 10); // bytes/sec
    curl_easy_setopt(eh, CURLOPT_LOW_SPEED_TIME, 10); // seconds while below low speed limit before aborting

    /* follow redirects */
    curl_easy_setopt(eh, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(eh, CURLOPT_NOPROGRESS, 0);

    /* setup cookies engine */
    curl_easy_setopt(eh, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(eh, CURLOPT_SHARE, curl_share);

    curl_easy_setopt(eh, CURLOPT_PROGRESSFUNCTION, showmultiprogress);
    curl_easy_setopt(eh, CURLOPT_PROGRESSDATA, data);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, data);

    /* some servers don't like requests that are made without a user-agent
    field, so we provide one */
    curl_easy_setopt(eh, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);

    /* proper function to close sockets */
    curl_easy_setopt(eh, CURLOPT_CLOSESOCKETFUNCTION, netclose_callback);

    curl_multi_add_handle(curl_multi, eh);
    return true;
}

void *DownloadThread (void *arg)
{
    CURLMsg *msg;
    int Q, U = 0;

    LWP_SuspendThread(downloadthread);
    curl_multi = curl_multi_init();
    manager = new GuiDownloadManager();

    /* we can optionally limit the total amount of connections this multi handle
       uses */
    curl_multi_setopt(curl_multi, CURLMOPT_MAXCONNECTS, (long)MAXD);

    while(1)
    {
        if (!U && networkinit)
            LWP_SuspendThread(downloadthread);
        if (downloadThreadHalt)
            break;

        while(!networkinit)
        {
            // manager->status->SetText("network down");
            if(LWP_ThreadIsSuspended(networkthread))
                InitNetwork();
            usleep(1000);
        }

        if (ExitRequested)
        {
            ExitRequested = false;

            if (U && !WindowPrompt("Exiting app", "Some downloads are still in progress. Exit anyway?", "Yes", "No"))
                App->SaveTooltip->SetTimeout("Click here", 2);
            else ExitAccepted = true;
        }

        UpdateQueue();
        curl_multi_perform(curl_multi, &U);
		usleep(50*1000);

        while ((msg = curl_multi_info_read(curl_multi, &Q)))
        {
            if (msg->msg == CURLMSG_DONE)
            {
                CURL *e = msg->easy_handle;
                curl_multi_remove_handle(curl_multi, e);
                CompleteDownload(msg);
            }
        }
    }

    curl_multi_cleanup(curl_multi);
    return NULL;
}

void StopDownload()
{
    if(downloadthread == LWP_THREAD_NULL)
        return;

    downloadThreadHalt = 1;
    LWP_ResumeThread(downloadthread);
}

Private *AddDownload(CURLM *cm, char *url, file *file)
{
    return PushQueue(cm, url, file, false);
}

Private *AddUpdate(CURLM *cm, char *url, FILE *file)
{
    struct file temp;
    temp.file = file;
    strcpy(temp.name, "update.dol");

    return PushQueue(cm, url, &temp, true);
}
