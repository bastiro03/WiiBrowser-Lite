#include <list>
#include "main.h"
#include "fileop.h"
#include "config.h"

#include "transfer.h"
#include "networkop.h"
#include "filelist.h"
#include "httplib.h"

using namespace std;
extern GuiToolbar *App;
off_t GetFileSize(const char *filename);

u8 downloadstack[GUITH_STACK] ATTRIBUTE_ALIGN(32);
lwp_t downloadthread = LWP_THREAD_NULL;
GuiDownloadManager *manager = nullptr;

static int downloadThreadHalt = 0;
static list<Private *> queue;
static char dl_cacert_path[256] = {0};

// -----------------------------------------------------------
// SSL CERTIFICATE SETUP
// -----------------------------------------------------------

static void setup_dl_cacert()
{
	if (dl_cacert_path[0] != 0)
		return; // Already initialized

	// Write the embedded CA certificate bundle to SD card
	snprintf(dl_cacert_path, sizeof(dl_cacert_path), "%s/cacert.pem", Settings.AppPath);

	FILE *f = fopen(dl_cacert_path, "wb");
	if (f)
	{
		fwrite(cacert_pem, 1, cacert_pem_size, f);
		fclose(f);
	}
	else
	{
		// Fallback: try writing to /tmp
		snprintf(dl_cacert_path, sizeof(dl_cacert_path), "/tmp/cacert.pem");
		f = fopen(dl_cacert_path, "wb");
		if (f)
		{
			fwrite(cacert_pem, 1, cacert_pem_size, f);
			fclose(f);
		}
		else
		{
			dl_cacert_path[0] = 0; // Failed to write cert
		}
	}
}

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
					  curl_off_t dltotal,
					  curl_off_t dlnow,
					  curl_off_t ultotal,
					  curl_off_t ulnow)
{
	auto data = static_cast<Private *>(bar);
	double total_bytes = (double)dltotal + (double)data->bytes;
	if (total_bytes > 0)
		manager->SetProgress(data, ((double)dlnow + (double)data->bytes) * 100.0 / total_bytes);

	if (manager->CancelDownload(data->bar))
		return 1;
	return 0;
}

int netclose_callback(void *clientp, curl_socket_t item)
{
	return net_close(item);
}

static curl_socket_t netopen_callback(void *clientp,
                                      curlsocktype purpose,
                                      struct curl_sockaddr *address)
{
	(void)clientp;
	(void)purpose;
	// Force protocol=0 (IPPROTO_IP) for Dolphin compatibility
	int fd = net_socket(address->family, address->socktype, 0);
	fprintf(stderr, "netopen_callback: net_socket(%d, %d, 0) = %d\n",
	        address->family, address->socktype, fd);
	return fd;
}

static size_t writedata(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written = fwrite(ptr, size, nmemb, static_cast<FILE *>(stream));
	return written;
}

Private *PushQueue(CURLM *cm, char *url, file *file, bool keep)
{
	if (!file || !cm)
		return nullptr;

	auto data = new Private;
	data->bar = nullptr;
	data->url = strdup(url);
	data->keep = keep;
	data->resolve = nullptr;

	data->bytes = 0;
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
	while (!queue.empty())
	{
		if (AddHandle(queue.front()))
			PopQueue();
		else
			break;
	}
}

void CompleteDownload(CURLMsg *msg)
{
	char *name;
	char message[256];
	long http_response;

	curl_off_t dl_bytes_total = 0;
	curl_off_t dl_bytes_size = 0;
	int retval = -1;

	Private *data;
	curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &data);
	curl_easy_getinfo(msg->easy_handle, CURLINFO_SIZE_DOWNLOAD_T, &dl_bytes_size);
	curl_easy_getinfo(msg->easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &dl_bytes_total);
	data->bytes = (double)dl_bytes_size;
	curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_response);
	curl_easy_cleanup(msg->easy_handle);

	if (data->resolve)
	{
		curl_slist_free_all(data->resolve);
		data->resolve = nullptr;
	}

	manager->RemoveBar(data->bar);
	data->code = msg->data.result;
	fclose(data->save.file);

	switch (data->code)
	{
	case CURLE_OK:
		name = strrchr(data->save.name, '.');
		retval = COMPLETE;

		if (Settings.ZipFile && !strtokcmp(name, ArchiveFiles, ","))
		{
			if (Settings.ZipFile == UNZIP ||
				WindowPrompt("Download complete", "You downloaded a zip archive. Unzip it?", "Yes", "No"))
			{
#ifdef WIIFLOW
				ShowAction("Extracting files...");
#endif
				if (UnzipArchive(data->save.name))
				{
					App->SaveTooltip->SetTimeout("File unzipped", 2);
					remove(data->save.name);
				}
				else
					App->SaveTooltip->SetTimeout("Error unzipping file", 2);

				CancelAction();
			}
		}
		break;

	case CURLE_ABORTED_BY_CALLBACK:
		retval = CANCELLED;
		remove(data->save.name);
		break;

	case CURLE_PARTIAL_FILE:
	case CURLE_OPERATION_TIMEDOUT:
		switch (http_response)
		{
		case 0:	  // eg connection down from kick-off: suggest retrying till some max limit
		case 200: // yay we at least got to our url
		case 206: // Partial Content
			data->save.file = nullptr;
			queue.push_back(data);
			InitNetwork();
			LWP_ResumeThread(downloadthread);
			retval = PARTIAL;
			break;

		case 416:
			// cannot d/l range: either cos no server support
			// or cos we're asking for an invalid range (ie: we already d/ld the file)
			if (GetFileSize(data->save.name) < dl_bytes_total)
				retval = FAILED;
			else
				retval = COMPLETE;
			break;

		default: // suggest quitting on an unhandled error
			retval = FAILED;
			break;
		}
		break;

	default:
		retval = FAILED;
		break;
	}

	if (retval == FAILED)
	{
		save_mem(curl_easy_strerror(static_cast<CURLcode>(data->code)));
		name = strrchr(data->save.name, '/');

		if (name)
			sprintf(message, "The download of %s failed. Try again?", name + 1);
		else
			sprintf(message, "The download of %s failed. Try again?", data->save.name);

		if (WindowPrompt("Download failed", message, "Yes", "No"))
		{
			data->save.file = fopen(data->save.name, "wb");
			queue.push_back(data);
			InitNetwork();
			LWP_ResumeThread(downloadthread);
			retval = PARTIAL;
		}
		else
			remove(data->save.name);
	}

#ifdef WIIFLOW
	else if (retval == COMPLETE)
	{
		WindowPrompt("Download complete", "The app will now exit...", "Ok", NULL);
		ExitRequested = true;
	}
#endif

	if (retval != PARTIAL)
	{
		if (!data->keep)
		{
			free(data->url);
			delete (data);
		}
	}
}

bool AddHandle(Private *data)
{
	data->bar = manager->CreateBar();
	if (!data->bar)
		return false;

	CURL *eh = curl_easy_init();
	if (!eh)
		return false;

	/* get the new filesize & sanity check for file */
	if (!data->save.file)
	{
		curl_easy_setopt(eh, CURLOPT_RESUME_FROM, GetFileSize(data->save.name));
		data->save.file = fopen(data->save.name, "ab");
	}

	curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, writedata);
	curl_easy_setopt(eh, CURLOPT_WRITEDATA, static_cast<void *>(data->save.file));
	curl_easy_setopt(eh, CURLOPT_URL, data->url);

	/* set proxy if specified */
	if (validProxy())
		curl_easy_setopt(eh, CURLOPT_PROXY, Settings.Proxy);

	/* the follwoing sets up min download speed threshold & time endured before aborting */
	curl_easy_setopt(eh, CURLOPT_LOW_SPEED_LIMIT, 10); // bytes/sec
	curl_easy_setopt(eh, CURLOPT_LOW_SPEED_TIME, 10);  // seconds while below low speed limit before aborting

	/* Hard bounds on connect and overall transfer time so a stuck IOS
	 * socket (e.g. Dolphin's async-connect path returning EALREADY) can
	 * never hang the download thread forever. These are belt-and-braces
	 * on top of LOW_SPEED_TIME above. */
	curl_easy_setopt(eh, CURLOPT_CONNECTTIMEOUT, 15L); // seconds for TCP + TLS handshake
	curl_easy_setopt(eh, CURLOPT_TIMEOUT, 300L);       // 5 min max for a single download

	/* follow redirects */
	curl_easy_setopt(eh, CURLOPT_AUTOREFERER, 1);
	curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(eh, CURLOPT_NOPROGRESS, 0);

	/* setup cookies engine */
	curl_easy_setopt(eh, CURLOPT_COOKIEFILE, "");
	curl_easy_setopt(eh, CURLOPT_SHARE, curl_share);

	curl_easy_setopt(eh, CURLOPT_XFERINFOFUNCTION, showmultiprogress);
	curl_easy_setopt(eh, CURLOPT_XFERINFODATA, data);
	curl_easy_setopt(eh, CURLOPT_PRIVATE, data);

	/* some servers don't like requests that are made without a user-agent
	field, so we provide one */
	curl_easy_setopt(eh, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	/* Enable SSL certificate verification for HTTPS security */
	setup_dl_cacert();
	if (dl_cacert_path[0] != 0)
	{
		curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(eh, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(eh, CURLOPT_CAINFO, dl_cacert_path);
	}
	else
	{
		/* Fallback: disable verification if cert bundle couldn't be written */
		curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);
	}

	/* override socket creation to use IPPROTO=0 (Dolphin compatibility) */
	curl_easy_setopt(eh, CURLOPT_OPENSOCKETFUNCTION, netopen_callback);
	curl_easy_setopt(eh, CURLOPT_CLOSESOCKETFUNCTION, netclose_callback);

	/* bypass c-ares: resolve hostname via libogc and feed CURLOPT_RESOLVE.
	 * Slist is owned by the Private data and freed in CompleteDownload. */
	data->resolve = wbl_build_resolve_list(data->url);
	if (data->resolve)
		curl_easy_setopt(eh, CURLOPT_RESOLVE, data->resolve);

	curl_multi_add_handle(curl_multi, eh);
	return true;
}

void *DownloadThread(void *arg)
{
	CURLMsg *msg;
	int Q, U = 0;

	LWP_SuspendThread(downloadthread);
	curl_multi = curl_multi_init();
	manager = new GuiDownloadManager();

	/* we can optionally limit the total amount of connections this multi handle
	   uses */
	curl_multi_setopt(curl_multi, CURLMOPT_MAXCONNECTS, static_cast<long>(MAXD));

	while (true)
	{
		if (!U && networkinit)
			LWP_SuspendThread(downloadthread);
		if (downloadThreadHalt)
			break;

		// Wait for networking to come back up. Same flicker/CPU-peg
		// fix as menu.cpp: 50ms poll (not 1ms) + bounded timeout so
		// a permanently-stuck network thread can't freeze the whole
		// download queue. Do NOT auto-restart the thread when it
		// suspends - that creates a deinit/init feedback cycle that
		// thrashes the heap (see menu.cpp for the detailed explanation).
		int net_timeout = 100; // 100 * 50ms = 5 s
		while (!networkinit && net_timeout > 0)
		{
			usleep(50000);
			net_timeout--;
		}
		if (!networkinit)
		{
			// Network never came back after 5s. Rather than looping
			// forever (and pegging CPU), suspend ourselves so the
			// next AddHandle() will resume us and we'll retry. This
			// mirrors what happens at line 350 when U==0.
			fprintf(stderr, "downloadthread: network init timed out, "
			                "suspending until next request\n");
			LWP_SuspendThread(downloadthread);
			continue;
		}

		if (ExitRequested)
		{
			ExitRequested = false;

			if (U && !WindowPrompt("Exiting app", "Some downloads are still in progress. Exit anyway?", "Yes", "No"))
				App->SaveTooltip->SetTimeout("Click here", 2);
			else
				ExitAccepted = true;
		}

		UpdateQueue();
		while (curl_multi_perform(curl_multi, &U) == CURLM_CALL_MULTI_PERFORM)
			;
		usleep(100);

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
	return nullptr;
}

void StopDownload()
{
	if (downloadthread == LWP_THREAD_NULL)
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
