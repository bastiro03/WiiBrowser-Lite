/****************************************************************************
 * WiiBrowser-Lite Cross-Platform Configuration
 *
 * Common browser tech stack across:
 *   - Nintendo Wii (88MB RAM, PPC @ 729MHz)
 *   - Nintendo DSi (16MB RAM, ARM9 @ 133MHz) - JS optional
 *   - Nintendo DS  (4MB RAM, ARM9 @ 67MHz)   - no JS
 *   - Apple Mac Plus (1-4MB RAM, 68000 @ 8MHz) - no JS
 *
 * Detect platform via build system defines:
 *   -DWBL_PLATFORM_WII
 *   -DWBL_PLATFORM_NDS
 *   -DWBL_PLATFORM_DSI
 *   -DWBL_PLATFORM_MACPLUS
 *
 * Feature flags are derived from platform; override with -DWBL_FORCE_*
 ***************************************************************************/

#ifndef WBL_PLATFORM_H
#define WBL_PLATFORM_H

/* ---- Platform detection ------------------------------------------------ */

#if defined(GEKKO) && !defined(WBL_PLATFORM_WII)
#  define WBL_PLATFORM_WII 1
#endif

#if defined(ARM9) && defined(_NDS) && !defined(WBL_PLATFORM_NDS) && !defined(WBL_PLATFORM_DSI)
#  define WBL_PLATFORM_NDS 1
#endif

#if defined(__MC68000__) && !defined(WBL_PLATFORM_MACPLUS)
#  define WBL_PLATFORM_MACPLUS 1
#endif

/* ---- Platform identification strings ----------------------------------- */

#if defined(WBL_PLATFORM_WII)
#  define WBL_PLATFORM_NAME "Nintendo Wii"
#  define WBL_PLATFORM_RAM_MB 88
#elif defined(WBL_PLATFORM_DSI)
#  define WBL_PLATFORM_NAME "Nintendo DSi"
#  define WBL_PLATFORM_RAM_MB 16
#elif defined(WBL_PLATFORM_NDS)
#  define WBL_PLATFORM_NAME "Nintendo DS"
#  define WBL_PLATFORM_RAM_MB 4
#elif defined(WBL_PLATFORM_MACPLUS)
#  define WBL_PLATFORM_NAME "Apple Mac Plus"
#  define WBL_PLATFORM_RAM_MB 4
#elif defined(WBL_PLATFORM_HOST) || (!defined(GEKKO) && !defined(__MC68000__))
   /* Host development build - for running unit tests and CI smoke tests.
    * Uses system resources freely; not a shipped target. */
#  define WBL_PLATFORM_HOST 1
#  define WBL_PLATFORM_NAME "Host (dev only)"
#  define WBL_PLATFORM_RAM_MB 1024
#else
#  error "Unknown platform - define one of WBL_PLATFORM_{WII,NDS,DSI,MACPLUS,HOST}"
#endif

/* ---- Feature capability flags ------------------------------------------ */

/* JavaScript engine (QuickJS) - needs ~1MB RAM minimum */
#ifndef WBL_HAS_JAVASCRIPT
#  if defined(WBL_PLATFORM_WII) || defined(WBL_PLATFORM_DSI) || defined(WBL_PLATFORM_HOST)
#    define WBL_HAS_JAVASCRIPT 1
#  else
#    define WBL_HAS_JAVASCRIPT 0
#  endif
#endif

/* HTTPS/TLS via mbedTLS.
 *
 * All platforms except Mac Plus. Precedent: Bunjalloo (open-source NDS
 * browser) already ships mbedTLS on vanilla NDS (4MB RAM), so we follow
 * their footprint discipline - tight cipher set + 4KB record buffers. */
#ifndef WBL_HAS_HTTPS
#  if defined(WBL_PLATFORM_MACPLUS)
#    define WBL_HAS_HTTPS 0  /* 1MB + 8MHz 68000 cannot carry TLS state */
#  else
#    define WBL_HAS_HTTPS 1
#  endif
#endif

/* MPlayer/FFmpeg video playback - disabled by default on all platforms.
 * The legacy MPlayer-wii + FFmpeg stack that WiiBrowser used to link
 * against (libmplayerwii.a, libavformat.a, libavcodec.a, libswscale.a,
 * libavutil.a) is no longer in devkitPro portlibs and was previously
 * shipped as pre-built .a files in libs/wii/. Re-enabling requires
 * reviving those as git submodules of upstream FFmpeg + MPlayer-wii.
 * For Wikipedia browsing this is completely unnecessary. */
#ifndef WBL_HAS_MPLAYER
#  define WBL_HAS_MPLAYER 0
#endif

/* Image format support */
#ifndef WBL_HAS_PNG
#  define WBL_HAS_PNG 1
#endif

#ifndef WBL_HAS_JPEG
#  if defined(WBL_PLATFORM_MACPLUS) || defined(WBL_PLATFORM_NDS)
#    define WBL_HAS_JPEG 0  /* libjpeg too heavy for these */
#  else
#    define WBL_HAS_JPEG 1
#  endif
#endif

#ifndef WBL_HAS_GIF
#  define WBL_HAS_GIF 1
#endif

/* CSS support level */
#ifndef WBL_CSS_LEVEL
#  if defined(WBL_PLATFORM_MACPLUS)
#    define WBL_CSS_LEVEL 1  /* CSS1 only - tables, basic styling */
#  elif defined(WBL_PLATFORM_NDS)
#    define WBL_CSS_LEVEL 2  /* CSS2 - no animations/transforms */
#  else
#    define WBL_CSS_LEVEL 3  /* CSS3 minus heavy features */
#  endif
#endif

/* HTTP/networking via libcurl */
#ifndef WBL_HAS_CURL
#  define WBL_HAS_CURL 1
#endif

/* Archive support (zip/rar/7z) */
#ifndef WBL_HAS_ARCHIVES
#  if defined(WBL_PLATFORM_WII)
#    define WBL_HAS_ARCHIVES 1
#  else
#    define WBL_HAS_ARCHIVES 0
#  endif
#endif

/* ---- Memory budgets (in KB) ------------------------------------------- */

#if defined(WBL_PLATFORM_WII)
#  define WBL_MEM_BUDGET_DOM_KB     16384  /* 16MB DOM tree */
#  define WBL_MEM_BUDGET_NETBUF_KB    512  /* network buffers */
#  define WBL_MEM_BUDGET_JS_HEAP_KB  4096  /* JS heap */
#elif defined(WBL_PLATFORM_DSI)
#  define WBL_MEM_BUDGET_DOM_KB      4096
#  define WBL_MEM_BUDGET_NETBUF_KB    128
#  define WBL_MEM_BUDGET_JS_HEAP_KB  1024
#elif defined(WBL_PLATFORM_NDS)
   /* 4 MB total. Reserve:
    *   ~400 KB for libnds/ARM9 runtime + screen buffers
    *   ~300 KB for TLS state (mbedTLS context + cert chain + record buf)
    *   ~800 KB for DOM + layout
    *   ~64 KB  for network buffers (post-TLS decrypt)
    *   rest    for image decode + UI chrome
    * Matches Bunjalloo's working budget at the cost of not loading
    * mega-heavy pages - mobile Wikipedia articles fit easily. */
#  define WBL_MEM_BUDGET_DOM_KB       768
#  define WBL_MEM_BUDGET_NETBUF_KB     64
#  define WBL_MEM_BUDGET_JS_HEAP_KB     0   /* no JS on DS */
#elif defined(WBL_PLATFORM_MACPLUS)
#  define WBL_MEM_BUDGET_DOM_KB       256
#  define WBL_MEM_BUDGET_NETBUF_KB     16
#  define WBL_MEM_BUDGET_JS_HEAP_KB     0
#elif defined(WBL_PLATFORM_HOST)
#  define WBL_MEM_BUDGET_DOM_KB    262144
#  define WBL_MEM_BUDGET_NETBUF_KB  8192
#  define WBL_MEM_BUDGET_JS_HEAP_KB 65536
#endif

/* ---- Endianness (used by mbedTLS, libcurl, etc.) ---------------------- */

#if defined(WBL_PLATFORM_WII)
#  define WBL_BIG_ENDIAN 1  /* PowerPC */
#elif defined(WBL_PLATFORM_NDS) || defined(WBL_PLATFORM_DSI)
#  define WBL_LITTLE_ENDIAN 1  /* ARM */
#elif defined(WBL_PLATFORM_MACPLUS)
#  define WBL_BIG_ENDIAN 1  /* 68000 */
#endif

#endif /* WBL_PLATFORM_H */
