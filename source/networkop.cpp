#include "networkop.h"

u8 networkstack[GUITH_STACK] ATTRIBUTE_ALIGN (32);
lwp_t networkthread = LWP_THREAD_NULL;

static int networkThreadHalt = 0;
GuiDownloadManager* manager = NULL;

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
    int i = *(int *)bar;
    manager->SetProgress(i, done*100.0/total);

    if(manager->CancelDownload(i))
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

void AddHandle(CURLM *cm, char *url, FILE *file)
{
    CURL *eh = curl_easy_init();
    if(!file || !cm)
        return;

    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, writedata);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *)file);
    curl_easy_setopt(eh, CURLOPT_URL, strdup(url));

    /* follow redirects */
    curl_easy_setopt(eh, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1);

    int *bar = manager->CreateBar();
    if(!bar)
        return;

    curl_easy_setopt(eh, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(eh, CURLOPT_PROGRESSFUNCTION, showmultiprogress);
    curl_easy_setopt(eh, CURLOPT_PROGRESSDATA, bar);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, bar);

    /* some servers don't like requests that are made without a user-agent
    field, so we provide one */
    curl_easy_setopt(eh, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);

    /* proper function to close sockets */
    curl_easy_setopt(eh, CURLOPT_CLOSESOCKETFUNCTION, netclose_callback);

    curl_multi_add_handle(cm, eh);
    LWP_ResumeThread(networkthread);
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

    while (1)
    {
        if(!U)
            LWP_SuspendThread(networkthread);
        if (networkThreadHalt)
            break;

        curl_multi_perform(curl_multi, &U);
		usleep(10*1000);

        while ((msg = curl_multi_info_read(curl_multi, &Q)))
        {
            if (msg->msg == CURLMSG_DONE)
            {
                int *url;
                CURL *e = msg->easy_handle;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
                curl_multi_remove_handle(curl_multi, e);
                curl_easy_cleanup(e);
                manager->RemoveBar(url);
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
