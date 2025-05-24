#include "networkop.h"
#define host "www.google.com"

u8 networkstack[GUITH_STACK] ATTRIBUTE_ALIGN(32);
lwp_t networkthread = LWP_THREAD_NULL;

int networkThreadHalt = 0;
bool networkinit = true;

/****************************************************************************
 * NetworkThread
 *
 * Thread to handle connection.
 ***************************************************************************/
void *NetworkThread(void *arg)
{
	s32 res = -1;
	int retry;
	int wait;
	static bool prevInit = false;

	while (!networkThreadHalt)
	{
		retry = 5;

		while (retry > 0 && !networkThreadHalt)
		{
			net_deinit();

			if (prevInit)
			{
				prevInit = false;  // only call net_wc24cleanup once
				net_wc24cleanup(); // kill wc24
				usleep(10000);
			}

			res = net_init_async(NULL, NULL);

			if (res != 0)
			{
				sleep(1);
				retry--;
				continue;
			}

			res = net_get_status();
			wait = 500; // only wait 10 sec

			while (res == -EBUSY && wait > 0 && !networkThreadHalt)
			{
				usleep(20 * 1000);
				res = net_get_status();
				wait--;
			}

			if (res == 0)
			{
				// struct in_addr hostip;
				// hostip.s_addr = net_gethostip();

				if (CheckConnection())
				{
					networkinit = true;
					prevInit = true;
					break;
				}
			}

			retry--;
			usleep(2000);
		}

		if (!networkThreadHalt)
			LWP_SuspendThread(networkthread);
	}
	return nullptr;
}

void InitNetwork()
{
	networkThreadHalt = 0;
	networkinit = false;

	if (networkthread == LWP_THREAD_NULL)
		LWP_CreateThread(&networkthread, NetworkThread, NULL, networkstack, GUITH_STACK, 30);
	else
		LWP_ResumeThread(networkthread);
}

void StopNetwork()
{
	if (networkthread == LWP_THREAD_NULL)
		return;

	networkThreadHalt = 1;
	LWP_ResumeThread(networkthread);

	LWP_JoinThread(networkthread, NULL);
	networkthread = LWP_THREAD_NULL;
}

bool CheckConnection()
{
	s32 s = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	struct sockaddr_in sa;

	if (s < 0)
		return false;

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_len = sizeof(struct sockaddr_in);
	sa.sin_port = htons(80);
	inet_aton("216.127.94.127", &sa.sin_addr); // store IP in sa

	int res = net_connect(s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
	net_close(s);

	return (res >= 0);
}
