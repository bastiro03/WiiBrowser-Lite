# libs/wii/ - legacy pre-built static libraries

This directory holds `*.a` files that ship as pre-built binaries in the
tree. That is an explicit tech-debt item: every dependency should be a
git submodule pinned to an upstream revision + a discrete patch stack,
so the build is reproducible on newer toolchains.

## Current state

| File              | Status         | Plan |
|-------------------|----------------|------|
| libexif.a         | pre-built      | TODO submodule -> libexif upstream |
| libiconv.a        | pre-built      | TODO submodule -> GNU libiconv upstream |
| libnetport.a      | pre-built      | TODO inline source (small Wii-specific BSD socket shim) |
| libsevenzip.a     | pre-built      | TODO submodule -> 7-Zip LZMA SDK |
| libunrar.a        | pre-built      | TODO submodule -> upstream unrarlib |
| libzip.a          | pre-built      | TODO submodule -> libzip upstream |

## Already migrated to upstream (either submodule or devkitPro portlib)

Removed from this directory; now built from source on every CI run:

| Former file       | Replacement |
|-------------------|-------------|
| libcurl.a         | `third_party/curl` submodule (curl 8.16) |
| libcyassl.a       | `third_party/mbedtls` submodule (mbedTLS 3.6 LTS) |
| liblua.a          | removed entirely (Lua scripting feature dropped) |
| libpng.a          | devkitPro portlib `ppc-libpng` (pinned upstream) |
| libjpeg.a         | devkitPro portlib `ppc-libjpeg-turbo` |
| libz.a            | devkitPro portlib `ppc-zlib` |
| libfreetype.a     | devkitPro portlib `ppc-freetype` |
| libmxml.a         | devkitPro portlib `ppc-mxml` |
| libfribidi.a      | devkitPro portlib `ppc-libfribidi` |
| libvorbisidec.a   | devkitPro portlib `ppc-libvorbisidec` |

## Link-order rule

`platform/wii/build.mk` sets `LDFLAGS` to search library paths in this
order:

```
-L$(LIBOGC_LIB) -L$(PORTLIBS_LIB) -L$(CURDIR)/libs/wii
```

so that if a library is available both in `libs/wii/` and in portlibs,
the portlib version wins. This makes the migration incremental: when
we submodule a library upstream, we delete the `libs/wii/*.a` file and
the build picks up the newer version automatically.

## Submodule migration rules

When migrating a pre-built `.a` to a submodule:

1. Add the submodule under `third_party/<name>/`:
   ```
   git submodule add https://.../.../<name>.git third_party/<name>
   cd third_party/<name> && git checkout <pinned-upstream-tag>
   ```
2. Add a `third_party/<name>.mk` following the pattern of
   `third_party/{curl,mbedtls,quickjs}.mk`:
   - Uses `AR=gcc-ar / RANLIB=gcc-ranlib` so LTO bitcode is preserved
   - Inherits `CFLAGS` from the platform build.mk so `-flto -Os
     -ffunction-sections -fdata-sections` reaches every TU
   - Applies `third_party/<name>-patches/*.patch` idempotently
3. Create `third_party/<name>-patches/` with a `README.md` describing
   the patch stack. Each patch must be `git format-patch` style so it
   can be submitted upstream via `git am`.
4. Include the `.mk` from the top-level `Makefile`.
5. Delete `libs/wii/lib<name>.a` and verify CI still produces a
   working `.dol` via `scripts/verify-dol.sh`.
