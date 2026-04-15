# mbedTLS patch stack for WiiBrowser-Lite

Applied to `third_party/mbedtls` (pinned to `mbedtls-3.6.6` LTS).

## Patches

### 0001-config-allow-wbl_mbedtls_config-override.patch

**Title:** config: allow #include "wbl_mbedtls_config.h" override

**Fix type:** build / embedding-story

**Summary:** mbedTLS's documented way to use a custom config is
`-DMBEDTLS_CONFIG_FILE='"custom.h"'`. That works for simple build
systems, but through nested make-to-make or make-to-autoconf
invocations (which we use for the curl-with-mbedTLS build), one
layer of shell quoting is stripped per invocation, so the value of
the macro ends up without its surrounding double-quotes and the
resulting `#include MBEDTLS_CONFIG_FILE` directive is invalid.

Add a `__has_include("wbl_mbedtls_config.h")` check at the top of
`include/mbedtls/mbedtls_config.h` that prefers the project's
override if available, and falls back to the default config if not.
No preprocessor macro juggling required.

**Upstream status:** not submitted yet. The feature is uncontroversial
(it's purely opt-in) and could reasonably ship in mbedTLS 3.7. An
alternative would be `-DMBEDTLS_USER_CONFIG_FILE='"..."'` which already
exists in mbedTLS but only *appends* to the default config; this patch
lets the project *replace* the config without preprocessor quoting.

---

See `../curl-patches/README.md` for patch stack conventions.
