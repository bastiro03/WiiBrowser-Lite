# Build Guide — WiiBrowser Lite

## Prerequisites

- **devkitPro** with `devkitPPC` + `libogc` + `ppc-portlibs`
  - Install via https://devkitpro.org/wiki/Getting_Started
  - `sudo dkp-pacman -S devkitPPC libogc ppc-zlib ppc-libpng ppc-libjpeg ppc-freetype ppc-libcurl ppc-mxml ppc-lua` (add `ppc-ntfs` `ppc-ext2fs` `ppc-libfat` for forwarder)
- Env:
  ```sh
  export DEVKITPRO=/opt/devkitpro
  export DEVKITPPC=$DEVKITPRO/devkitPPC
  export PATH=$DEVKITPRO/tools/bin:$PATH
  ```

## Quick Start

```sh
make            # builds boot.elf / boot.dol in project root (via build/)
make forwarder  # builds forwarder/boot.dol (optional channel forwarder)
make clean      # remove build/
make distclean  # also removes generated css_lex/syntax
```

Generated parser/lexer (`src/html_parser/css_lex.c/.h`, `css_syntax.c/.h`) are gitignored and regenerated if you have `flex`/`bison`; otherwise committed stubs are used.

## Docker (reproducible)

```sh
docker run --rm -v $PWD:/src -w /src devkitpro/devkitppc:latest bash -lc "make -j$(nproc)"
```

CI uses same container (see `.github/workflows/main.yml`).

## Packaging HBC bundle

```sh
mkdir -p dist/apps/wiibrowser-lite
cp boot.dol dist/apps/wiibrowser-lite/boot.dol
cp HBC/icon.png dist/apps/wiibrowser-lite/icon.png
# version/release_date injected in CI via sed from HBC/meta.xml
cp HBC/meta.xml dist/apps/wiibrowser-lite/meta.xml
(cd dist && zip -r ../WiiBrowser-Lite-HBC.zip apps)
```

## Forwarder

`forwarder/` is folded into main build as optional target. `make forwarder` or CI job `Build Forwarder Channel` produces `forwarder/boot.dol`. Do not delete `forwarder/data/app_booter.dol` (embedded via bin2s).

## Troubleshooting

- `Please set DEVKITPPC` → export as above.
- `unknown type name 'u32'` → missing `libogc` include; verify `$DEVKITPPC/wii_rules` exists.
- `cannot find -lcyassl` → portlibs migrated from vendored `libs/` to `dkp-pacman`; ensure `ppc-wolfssl` or `ppc-libcyassl` installed; workflow runs `dkp-pacman -Sy`.
- `flex: command not found` → install flex/bison locally; or rely on committed stubs.
