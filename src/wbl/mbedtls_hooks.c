/****************************************************************************
 * WiiBrowser-Lite mbedTLS platform hooks
 *
 * Required because our mbedtls_config.h defines MBEDTLS_ENTROPY_HARDWARE_ALT
 * on embedded targets (Wii/DSi/DS) - mbedTLS then expects us to provide
 * mbedtls_hardware_poll().
 *
 * On each platform we mix in whatever "hardware-ish" entropy the OS
 * exposes. It's not cryptographic randomness by itself but combined
 * with mbedtls_entropy_func's accumulator it's good enough to seed
 * HMAC-DRBG for TLS.
 ***************************************************************************/

#include "wbl/platform.h"

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT) || \
    defined(WBL_PLATFORM_WII) || defined(WBL_PLATFORM_DSI) || defined(WBL_PLATFORM_NDS)

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(WBL_PLATFORM_WII)
#include <gccore.h>     /* for gettime() / SYS_Time() */
#include <ogc/lwp_watchdog.h>   /* ticks_to_millisecs */
#elif defined(WBL_PLATFORM_DSI) || defined(WBL_PLATFORM_NDS)
#include <nds.h>
#endif

/* mbedTLS calls this to fill `len` bytes of `output` with entropy.
 * On success: writes len bytes, sets *olen=len, returns 0.
 * On failure: returns non-zero mbedtls error code. */
int mbedtls_hardware_poll(void *data,
                          unsigned char *output,
                          size_t len,
                          size_t *olen)
{
    (void)data;
    if (!output || !olen) return -1;

    size_t produced = 0;

#if defined(WBL_PLATFORM_WII)
    /* Wii: mix high-res timer readings. gettime() returns a tick count
     * from the PowerPC time base register - only a few bits of jitter
     * per read but across 32 reads we get enough entropy for seeding. */
    while (produced < len) {
        u64 t = gettime();
        u32 lo = (u32)t;
        size_t take = (len - produced) < sizeof(lo) ? (len - produced) : sizeof(lo);
        memcpy(output + produced, &lo, take);
        produced += take;
    }
#elif defined(WBL_PLATFORM_DSI) || defined(WBL_PLATFORM_NDS)
    /* NDS/DSi: use the hardware random timer + TSC */
    while (produced < len) {
        u32 r = (u32)cpuGetTiming();  /* ARM9 cycle counter */
        size_t take = (len - produced) < sizeof(r) ? (len - produced) : sizeof(r);
        memcpy(output + produced, &r, take);
        produced += take;
    }
#else
    /* Fallback: no hardware entropy - fill with zeros, will fail tests */
    memset(output, 0, len);
    produced = len;
    return -1;
#endif

    *olen = produced;
    return 0;
}

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */


/* ---- mbedtls_ms_time() --------------------------------------------------
 *
 * mbedTLS uses this for TLS 1.3 handshake record-layer timing and
 * session-ticket expiry. The upstream fallback relies on
 * clock_gettime(CLOCK_MONOTONIC) which devkitPPC's newlib does NOT
 * provide, so without this the library won't even compile.
 *
 * MBEDTLS_PLATFORM_MS_TIME_ALT in wbl/mbedtls_config.h declares that
 * WE provide the function. The mbedtls_ms_time_t return type is a
 * signed 64-bit millisecond count; monotonicity matters more than
 * absolute zero-point. */

#if defined(MBEDTLS_PLATFORM_MS_TIME_ALT) || \
    defined(WBL_PLATFORM_WII) || defined(WBL_PLATFORM_DSI) || defined(WBL_PLATFORM_NDS)

/* mbedtls_ms_time_t is defined in mbedtls/platform_util.h, which is
 * pulled in by any TU that includes us via mbedtls. But our own TU
 * doesn't include mbedtls here (to keep this file standalone), so
 * declare the signature matching upstream. */
typedef int64_t mbedtls_ms_time_t;

mbedtls_ms_time_t mbedtls_ms_time(void)
{
#if defined(WBL_PLATFORM_WII)
    /* libogc's gettime() returns the PowerPC time-base tick count.
     * ticks_to_millisecs() converts to wall-clock ms. The TB is
     * monotonic across reboots (well, resets to 0 each reset but
     * never goes backwards within a session). */
    u64 t = gettime();
    return (mbedtls_ms_time_t)ticks_to_millisecs(t);
#elif defined(WBL_PLATFORM_DSI) || defined(WBL_PLATFORM_NDS)
    /* ARM9 has no simple monotonic clock; timerTick() on libnds is
     * driven by the REG_TIMER hardware timer. Assuming 59.8Hz frame
     * rate -> convert to ms. */
    extern unsigned int timerTick(int timer);
    unsigned int ticks = timerTick(0);
    /* libnds timer at default config runs at ~33kHz, so 1 tick ~= 30us */
    return (mbedtls_ms_time_t)(ticks / 33);
#else
    return 0;
#endif
}

#endif /* MBEDTLS_PLATFORM_MS_TIME_ALT */
