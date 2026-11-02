#!/usr/bin/env bash
# Explicit container setup, never invoked automatically by make.
set -euo pipefail

export DEVKITPRO=/opt/devkitpro
export DEVKITPPC="$DEVKITPRO/devkitPPC"
export PATH="$DEVKITPPC/bin:$DEVKITPRO/tools/bin:$PATH"
prefix="$DEVKITPRO/portlibs/wii"
sources=/tmp/opencode/deps
wait_patch="$(dirname "$(readlink -f "$0")")/wii-curl-wait.patch"
mkdir -p "$sources"
cd "$sources"

curl -fsSL https://curl.se/download/curl-8.23.0.tar.xz -o curl-8.23.0.tar.xz
sha256sum --check <<'HASHES'
8dadd7d2536a314f982ad05a651b713f426bf7d02cc2c76cffb9796f2218e6c5  curl-8.23.0.tar.xz
HASHES
tar -xJf curl-8.23.0.tar.xz

flags="-O2 -mrvl -mcpu=750 -meabi -mhard-float -ffunction-sections -fdata-sections"
cd "$sources/curl-8.23.0"
patch -p1 < "$wait_patch"
export PKG_CONFIG_LIBDIR="$prefix/lib/pkgconfig:$DEVKITPRO/portlibs/ppc/lib/pkgconfig"
# HTTP-only DEVELOPMENT build, explicitly approved for compilation testing.
# No reviewed strong Wii entropy source is available. Do not silently link a
# TLS backend with clock/device-ID pseudo-entropy or bypass verification.
# libogc supplies sockets directly; no legacy libwiisocket shim is used.
CFLAGS="$flags" CPPFLAGS="-DGEKKO -I$prefix/include -I$DEVKITPRO/portlibs/ppc/include -I$DEVKITPRO/libogc/include" \
LDFLAGS="-mrvl -L$prefix/lib -L$DEVKITPRO/portlibs/ppc/lib -L$DEVKITPRO/libogc/lib/wii" LIBS="-logc -lm" \
./configure --prefix="$prefix" --host=powerpc-eabi \
    --disable-shared --enable-static --disable-ipv6 --disable-unix-sockets \
    --disable-manual --disable-threaded-resolver --disable-socketpair \
    --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp \
    --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap \
    --disable-smtp --disable-gopher --disable-mqtt --disable-websockets --disable-ipfs \
    --without-libpsl --without-libssh2 --without-nghttp2 \
    --without-nghttp3 --without-ngtcp2 --without-brotli --without-zstd \
    --without-ssl --with-zlib="$DEVKITPRO/portlibs/ppc"
make -j2 -C lib
make -C lib install
make -C include install
make install-binSCRIPTS install-pkgconfigDATA
mkdir -p "$prefix/licenses/curl"
cp COPYING "$prefix/licenses/curl/"
