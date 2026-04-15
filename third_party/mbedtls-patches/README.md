# mbedTLS patch stack for WiiBrowser-Lite

Applied to `third_party/mbedtls` (pinned to `mbedtls-3.6.6` LTS).

No patches currently required. Our config (`include/wbl/mbedtls_config.h`)
achieves the footprint and feature set we need using only stock config
macros.

See `../curl-patches/README.md` for conventions.
