#include "networkop.h"
#include <atomic>
#include "main.h"

u8 networkstack[GUITH_STACK] ATTRIBUTE_ALIGN(32);
lwp_t networkthread = LWP_THREAD_NULL;
volatile bool networkinit = false;
namespace {
std::atomic<bool> stop(false);
std::atomic<bool> requested(false);
void *networkWorker(void *)
{
    while (!stop) {
        if (requested.exchange(false)) {
            networkinit = false;
            const s32 result = net_init_async(NULL, NULL);
            if (result >= 0 || result == -EAGAIN || result == -EBUSY) {
                for (int wait = 0; wait < 500 && !stop && !App_Exiting(); ++wait) {
                    const s32 status = net_get_status();
                    if (status == 0 && net_gethostip() != 0) {
                        // Registers libogc's POSIX socket devoptab as well as
                        // querying the already-established IOS configuration.
                        networkinit = if_config(NULL, NULL, NULL, true, 1) == 0;
                        break;
                    }
                    if (status < 0 && status != -EBUSY && status != -EAGAIN) break;
                    usleep(20000);
                }
            }
        }
        usleep(20000);
    }
    return NULL;
}
}

bool CheckConnection() { return networkinit && net_gethostip() != 0; }
void InitNetwork()
{
    if (CheckConnection() || App_Exiting()) return;
    stop = false;
    requested = true;
    if (networkthread == LWP_THREAD_NULL &&
        LWP_CreateThread(&networkthread, networkWorker, NULL, networkstack, GUITH_STACK, 30) != 0)
        requested = false;
}
void StopNetwork()
{
    stop = true;
    if (networkthread != LWP_THREAD_NULL) {
        LWP_JoinThread(networkthread, NULL);
        networkthread = LWP_THREAD_NULL;
        net_deinit();
    }
    networkinit = false;
}
