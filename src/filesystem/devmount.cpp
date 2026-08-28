#include <fat.h>
#include <ogcsys.h>
#include <sdcard/wiisd_io.h>
#include <ogc/disc_io.h>
#include <ogc/usbstorage.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "devmount.h"

#define CACHE 8
#define SECTORS 64

static bool sd_mounted = false;
static bool usb_mounted = false;

bool MountManager_Init(void) {
    // Try SD
    if(!__io_wiisd.startup() || !__io_wiisd.isInserted()) {
        // try still
    }
    if(fatMountSimple("sd", &__io_wiisd)) {
        sd_mounted = true;
    } else {
        // fallback fatInitDefault style
        fatInitDefault();
        sd_mounted = true;
    }
    // USB mount attempted best-effort (optional)
    if(__io_usbstorage.isInserted()) {
        if(fatMountSimple("usb", &__io_usbstorage)) usb_mounted = true;
    }
    return sd_mounted;
}

bool MountManager_IsInserted(const char* dev) {
    if(!dev) return false;
    if(strncmp(dev, "sd", 2)==0) return __io_wiisd.isInserted();
    if(strncmp(dev, "usb", 3)==0) return __io_usbstorage.isInserted();
    return false;
}

void MountManager_Poll(void) {
    static int counter=0;
    if((counter++ % 60) !=0) return; // ~1s at 60fps
    // Re-mount if card reinserted but not mounted
    if(!sd_mounted && __io_wiisd.isInserted()) {
        if(fatMountSimple("sd", &__io_wiisd)) sd_mounted = true;
    }
    if(sd_mounted && !__io_wiisd.isInserted()) {
        fatUnmount("sd:/");
        sd_mounted = false;
    }
}
