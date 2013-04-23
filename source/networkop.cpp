#include <list>
#include "main.h"
#include "networkop.h"

using namespace std;

u8 networkstack[GUITH_STACK] ATTRIBUTE_ALIGN (32);
lwp_t networkthread = LWP_THREAD_NULL;
GuiDownloadManager *manager = NULL;

static int networkThreadHalt = 0;
static list<Private *> queue;

bool AddHandle(Private *data);
void PopQueue();

/****************************************************************************
 * NetworkThread
 *
 * Thread to handle network connections.
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

Private *PushQueue(CURLM *cm, char *url, FILE *file, bool keep)
{
    if(!file || !cm)
        return NULL;

    Private *data = new Private;
    data->bar = NULL;
    data->file = file;
    data->url = strdup(url);
    data->keep = keep;
    data->code = -1;

    queue.push_back(data);
    LWP_ResumeThread(networkthread);
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
    Private *data;
    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &data);

    manager->RemoveBar(data->bar);
    data->code = msg->data.result;
    fclose(data->file);

    if(!data->keep)
    {
        free(data->url);
        delete(data);
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

    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, writedata);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *)data->file);
    curl_easy_setopt(eh, CURLOPT_URL, data->url);

    /* set proxy if specified */
    if(validProxy())
        curl_easy_setopt(eh, CURLOPT_PROXY, Settings.Proxy);

    /* follow redirects */
    curl_easy_setopt(eh, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1);

    curl_easy_setopt(eh, CURLOPT_NOPROGRESS, 0);
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

void *NetworkThread (void *arg)
{
    CURLMsg *msg;
    int Q, U = 0;

    LWP_SuspendThread(networkthread);
    curl_multi = curl_multi_init();
    manager = new GuiDownloadManager();

    /* we can optionally limit the total amount of connections this multi handle
       uses */
    curl_multi_setopt(curl_multi, CURLMOPT_MAXCONNECTS, (long)MAXD);

    while(1)
    {
        if(!U)
            LWP_SuspendThread(networkthread);
        if (networkThreadHalt)
            break;

        UpdateQueue();
        curl_multi_perform(curl_multi, &U);
		usleep(50*1000);

        while ((msg = curl_multi_info_read(curl_multi, &Q)))
        {
            if (msg->msg == CURLMSG_DONE)
            {
                CURL *e = msg->easy_handle;
                curl_multi_remove_handle(curl_multi, e);
                curl_easy_cleanup(e);
                CompleteDownload(msg);
            }
        }
    }

    curl_multi_cleanup(curl_multi);
    return NULL;
}

void StopNetwork()
{
    if(networkthread == LWP_THREAD_NULL)
        return;

    LWP_ResumeThread(networkthread);
    networkThreadHalt = 1;
}

Private *AddDownload(CURLM *cm, char *url, FILE *file)
{
    return PushQueue(cm, url, file, false);
}

Private *AddUpdate(CURLM *cm, char *url, FILE *file)
{
    return PushQueue(cm, url, file, true);
}
