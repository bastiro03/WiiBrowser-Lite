#include "crash.h"
#include <gccore.h>
#include <ogcsys.h>
#include <ogc/system.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mem2_manager.h"

extern "C" void __exception_setreload(int t);

static bool crash_inited = false;

static void write_crash_log(const char* reason, u32 srr0, u32 srr1, u32 lr) {
    FILE* f = fopen("sd:/wiibrowser-crash.log", "a");
    if(!f) f = fopen("sd:/apps/wiibrowser/wiibrowser-crash.log", "a");
    if(!f) return;
    fprintf(f, "=== WiiBrowser Crash ===\n");
    fprintf(f, "Reason: %s\n", reason ? reason : "exception");
    fprintf(f, "SRR0: 0x%08x SRR1: 0x%08x LR: 0x%08x\n", srr0, srr1, lr);
    for(int i=0;i<MEM2_MAX;i++) {
        u32 free = mem2_size(i);
        u32 total = mem2_total_size(i);
        if(total) fprintf(f, "MEM2[%d] free %u / total %u\n", i, free, total);
    }
    fclose(f);
}

static void show_crash_screen(const char* reason, u32 srr0) {
    // Best-effort red screen + text via console
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    // Console overlay if video already inited
    printf("\x1b[2J\x1b[37;41m");
    printf("WiiBrowser has crashed!\n");
    printf("Reason: %s SRR0: %08x\n", reason ? reason : "exception", srr0);
    printf("A log was written to sd:/wiibrowser-crash.log\n");
    printf("Press RESET to return to menu, POWER to shutdown.\n");
    fflush(stdout);
}

extern "C" void Crash_Dump(const char* reason, u32 srr0, u32 srr1, u32 lr) {
    write_crash_log(reason, srr0, srr1, lr);
    show_crash_screen(reason, srr0);
}

// libogc installs default DSI/ISI/ALI handlers at startup which dump
// registers and honor __exception_setreload(). There is no public per-type
// hook in the bundled libogc, so Crash_Init keeps the default handlers and
// only arms the reload watchdog. Use Crash_Dump() for manual diagnostics.
void Crash_Init(void) {
    if(crash_inited) return;
    crash_inited = true;
    // Note: SYS_SetResetCallback etc already set in main; keep them.
    // At least set reload timeout 10s so we don't hang black.
    __exception_setreload(10);
}
