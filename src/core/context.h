#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <string>
#include "settings.h"
#include "liste.h"

// Forward declarations to avoid heavy includes
struct _mxml_node_s;
typedef struct _mxml_node_s mxml_node_t;
class GuiToolbar;
class GuiWindow;
struct _gxrmodeobj;
typedef struct _gxrmodeobj GXRModeObj;
struct _CURL;
typedef struct _CURL CURL;
struct _CURLM;
typedef struct _CURLM CURLM;

/**
 * AppContext — single source of truth for global state (Sprint 3.1).
 *
 * Historically globals were scattered:
 *   main.cpp:      SSettings Settings; int ExitRequested/ExitAccepted; u8 HWButton
 *   menu.cpp:      History history; CURL *curl_handle; GuiToolbar *App; char new_page[]
 *   video.cpp:     GXRModeObj *vmode; u8 *videoScreenshot; int screenwidth
 *   handle.cpp:    bool hidden;
 *
 * New code should accept `AppContext&` instead of touching globals directly.
 * Existing globals remain as aliases for incremental migration.
 */
struct AppContext {
    // Core settings & lifecycle
    SSettings* settings = nullptr;
    int* exitRequested = nullptr;
    int* exitAccepted = nullptr;
    unsigned char* hwButton = nullptr;

    // Navigation history
    History* history = nullptr;
    char* newPage = nullptr;
    char* prevPage = nullptr;

    // Network
    CURL** curlHandle = nullptr;
    CURLM** curlMulti = nullptr;

    // Video / GX
    GXRModeObj** vmode = nullptr;
    unsigned char** videoScreenshot = nullptr;
    int* screenWidth = nullptr;
    int* screenHeight = nullptr;

    // UI
    GuiToolbar** appToolbar = nullptr;
    GuiWindow** mainWindow = nullptr;
    GuiWindow** guiWindow = nullptr;

    bool* guiHalt = nullptr;
    bool* drawGui = nullptr;

    // Validation
    bool isValid() const {
        return settings && history && curlHandle && appToolbar;
    }
};

// Global accessor — returns singleton context backed by legacy globals.
// Implemented in src/core/context.cpp to avoid circular includes.
AppContext GetAppContext();
void SetAppContext(const AppContext& ctx);

#endif
