#!/bin/bash
# Verify LTO bitcode is present in our third-party static archives
# and that the final-link LTO pass is actually doing cross-module work.
#
# Run after `make` in tests/.

set -e
cd "$(dirname "$0")"
ROOT=$(pwd)/..
AR=${AR:-gcc-ar}

echo "================================================================"
echo "LTO pipeline verification"
echo "================================================================"
echo ""

ARCHIVES=(
    "$ROOT/third_party/mbedtls/library/libmbedcrypto.a"
    "$ROOT/third_party/mbedtls/library/libmbedtls.a"
    "$ROOT/third_party/mbedtls/library/libmbedx509.a"
    "$ROOT/third_party/quickjs/libquickjs.a"
    "$ROOT/third_party/curl/lib/.libs/libcurl.a"
)

printf "%-28s %-10s %-12s %s\n" "archive" "size" "sample-obj" "lto-sections"
printf "%-28s %-10s %-12s %s\n" "-------" "----" "----------" "------------"

for lib in "${ARCHIVES[@]}"; do
    [[ -f "$lib" ]] || continue
    name=$(basename "$lib")
    bytes=$(stat -c %s "$lib")
    tmp=$(mktemp -d)
    (cd "$tmp" && $AR x "$lib")
    first=$(ls "$tmp"/*.o 2>/dev/null | head -1)
    if [[ -f "$first" ]]; then
        objname=$(basename "$first")
        count=$(readelf -SW "$first" 2>/dev/null | grep -c '\.gnu\.lto_' || echo 0)
        printf "%-28s %-10s %-12s %s\n" "$name" "$(numfmt --to=iec $bytes)" "$objname" "$count"
    fi
    rm -rf "$tmp"
done

echo ""
echo "================================================================"
echo "Cross-module inlining evidence"
echo "================================================================"
echo ""

# If the linker did LTO across the archives, some mbedTLS small helpers
# (like mbedtls_platform_zeroize) will be inlined into curl.a call sites
# and won't appear as separate symbols in the final binary.
if [[ -x curl_smoke ]]; then
    echo "curl_smoke binary symbols containing 'zeroize':"
    nm curl_smoke 2>/dev/null | grep -i zeroize | head -5 || echo "  (none - inlined)"
    echo ""
    echo "tls_smoke binary size breakdown:"
    size tls_smoke
    echo ""
    echo "curl_smoke binary size breakdown:"
    size curl_smoke
fi

echo ""
echo "================================================================"
echo "Build flag sanity"
echo "================================================================"
# A quick sanity check: dump one object's readelf and confirm -flto flag
# was set when compiling. GCC embeds the command line into .GCC.command.line
# of each LTO object.
tmp=$(mktemp -d)
(cd "$tmp" && $AR x "$ROOT/third_party/mbedtls/library/libmbedtls.a")
obj=$(ls "$tmp"/ssl_tls.o 2>/dev/null | head -1)
if [[ -f "$obj" ]]; then
    echo "mbedTLS ssl_tls.o was compiled with:"
    readelf -p .GCC.command.line "$obj" 2>/dev/null | grep -oE 'f(lto|function-sections|data-sections|unsigned-char)' | sort -u | sed 's/^/  -/'
fi
rm -rf "$tmp"
