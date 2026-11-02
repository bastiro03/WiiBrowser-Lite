#ifndef WIIBROWSER_CONFIG_H
#define WIIBROWSER_CONFIG_H

// Sprint 4.3: MPLAYER disabled by default (external/mplayer submodule placeholder).
// Enable by defining MPLAYER in Makefile CFLAGS (-DMPLAYER) and populating external/mplayer.
#undef  MPLAYER
#define DOCWRITE
#define IFRAME
#define WIIFLOW

#define WBL_ENABLE_UPLOADS 0

#endif // WIIBROWSER_CONFIG_H
