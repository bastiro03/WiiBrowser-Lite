#include "context.h"
#include "settings.h"
#include "liste.h"

// Legacy globals — declared in their original TUs, referenced here
extern SSettings Settings;
extern int ExitRequested;
extern int ExitAccepted;
extern u8 HWButton;
extern History history;
extern char new_page[];
// prev_page is static in menu.cpp — not extern, will be handled via accessor
extern CURL *curl_handle;
extern CURLM *curl_multi;
extern GXRModeObj *vmode;
extern u8 *videoScreenshot;
extern int screenwidth;
extern int screenheight;
extern GuiToolbar *App;
extern GuiWindow *mainWindow;
// guiWindow and guiHalt are static in menu.cpp — not extern
extern bool drawGui;

static AppContext g_ctx;

AppContext GetAppContext() {
    // Lazy-init singleton from legacy globals if not yet set
    if (!g_ctx.settings) {
        g_ctx.settings = &Settings;
        g_ctx.exitRequested = &ExitRequested;
        g_ctx.exitAccepted = &ExitAccepted;
        g_ctx.hwButton = &HWButton;
        g_ctx.history = &history;
        g_ctx.newPage = new_page;
        // prev_page is static in menu.cpp, not extern — leave null for now
        g_ctx.prevPage = nullptr;
        g_ctx.curlHandle = &curl_handle;
        g_ctx.curlMulti = &curl_multi;
        g_ctx.vmode = &vmode;
        g_ctx.videoScreenshot = &videoScreenshot;
        g_ctx.screenWidth = &screenwidth;
        g_ctx.screenHeight = &screenheight;
        g_ctx.appToolbar = &App;
        g_ctx.mainWindow = &mainWindow;
        g_ctx.drawGui = &drawGui;
    }
    // Static locals in menu.cpp remain null in context until refactored to non-static
    g_ctx.prevPage = nullptr;
    g_ctx.guiWindow = nullptr;
    g_ctx.guiHalt = nullptr;
    return g_ctx;
}

void SetAppContext(const AppContext& ctx) {
    g_ctx = ctx;
}
