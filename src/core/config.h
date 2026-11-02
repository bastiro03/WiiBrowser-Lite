#ifndef WIIBROWSER_CONFIG_H
#define WIIBROWSER_CONFIG_H

// Standalone Homebrew Channel build. These legacy integrations are deliberately
// unavailable, including when an older configuration requests them.
#undef MPLAYER
#undef DOCWRITE
#undef IFRAME
#undef WIIFLOW
#define WBL_ENABLE_UPDATER 0
#define WBL_ENABLE_ARCHIVES 0
#define WBL_ENABLE_UPLOADS 0
#define WBL_VERSION "0.1.0-dev"

#endif // WIIBROWSER_CONFIG_H
