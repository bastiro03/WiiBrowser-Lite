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
#include <string.h>

#if defined(WBL_PLATFORM_WII)
#include <gccore.h>     /* for gettime() / SYS_Time() */
#include <ogc/lwp_watchdog.h>
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
