#include <atomic>
#include <fat.h>
#include <ogcsys.h>
#include <sdcard/wiisd_io.h>
#include <ogc/usbstorage.h>
#include <ogc/disc_io.h>
#include <string.h>
#include "devmount.h"
#include "main.h"

namespace {
bool sdMounted = false;
bool usbMounted = false;
std::atomic<bool> mediaLost(false);
}

bool MountManager_Init()
{
    mediaLost = false;
    sdMounted = __io_wiisd.startup() && __io_wiisd.isInserted() && fatMountSimple("sd", &__io_wiisd);
    usbMounted = __io_usbstorage.startup() && __io_usbstorage.isInserted() && fatMountSimple("usb", &__io_usbstorage);
    return sdMounted || usbMounted;
}

bool MountManager_IsInserted(const char *device)
{
    if (!device || mediaLost) return false;
    bool inserted = false;
    if (strncmp(device, "sd:/", 4) == 0) {
        if (!sdMounted) return false;
        inserted = __io_wiisd.isInserted();
    } else if (strncmp(device, "usb:/", 5) == 0) {
        if (!usbMounted) return false;
        inserted = __io_usbstorage.isInserted();
    } else return false;
    if (!inserted) { mediaLost = true; ExitRequested = true; }
    return inserted;
}

void MountManager_Poll()
{
    // Detection only: never auto-remount a different volume under live handles.
    // Writers are stopped and main performs teardown. Physical replacement/
    // libfat dirty-cache behavior still requires the disposable-media tests.
    if (mediaLost) return;
    if ((sdMounted && !__io_wiisd.isInserted()) || (usbMounted && !__io_usbstorage.isInserted())) {
        mediaLost = true;
        ExitRequested = true;
    }
}

void MountManager_Deinit()
{
    // Do not deliberately flush an old cache to a newly inserted volume.
    if (!mediaLost) {
        if (sdMounted && __io_wiisd.isInserted()) fatUnmount("sd:/");
        if (usbMounted && __io_usbstorage.isInserted()) fatUnmount("usb:/");
    }
    sdMounted = usbMounted = false;
}
