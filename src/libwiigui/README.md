# libwiigui (WBL fork)

This directory contains WBL's vendored copy of **libwiigui**, a GUI library for Wii homebrew originally by Tantric (WiiMC, SNES9x GX, FCE Ultra GX, etc.).

## Current state (as of 2026-04)

- **NOT a submodule** — this is bundled source checked directly into the main tree
- **NOT tracking upstream** — libwiigui has no canonical upstream; every Wii homebrew project maintains its own fork
- **Last full snapshot**: commit `303b68f` ("The Return of WBL (the second)", May 2025)

The closest thing to a canonical upstream is [`dborth/libwiigui`](https://github.com/dborth/libwiigui), but it hasn't meaningfully moved since ~2010 and has the same bugs/drift we've fixed locally (e.g. `pngu.h` signature mismatch).

## WBL-specific modifications

1. **`pngu.h`** (commit `fcff2fe`, Apr 2026)  
   Replaced with a forward-include shim to `src/utils/pngu.h` to fix LTO undefined-reference errors. The old header declared `DecodePNG(src, w, h, maxW, maxH)` with 5 args; the canonical implementation in `src/utils/pngu.c` has 4 args `(src, w, h, dst)`.

2. **WBL-specific GUI extensions**  
   Several files (e.g. `gui_favorite`, `gui_longtext`, `gui_download`, `gui_toolbar`) appear to be WBL-specific and don't exist in other libwiigui forks.

## TODO: Compare with USB Loader GX's libwiigui fork

[**USB Loader GX**](https://github.com/wiiudev/usbloadergx) is actively maintained and has successfully ported to modern devkitPPC toolchains (including updates in 2025). Their libwiigui fork likely contains:

- Fixes for modern devkitPPC + libogc APIs
- LTO compatibility patches
- Compiler warning fixes for gcc 14.x+
- Memory management improvements

**Action item**: Diff our `src/libwiigui/` against USB Loader GX's `source/libwiigui/` ([link](https://github.com/wiiudev/usbloadergx/tree/master/source/libwiigui)) to identify:

1. Whether we can switch to using their repo as a submodule + WBL-specific patches
2. Fixes/improvements we're missing (especially devkitPPC toolchain updates)
3. Whether our WBL-specific extensions (`gui_favorite`, `gui_longtext`, etc.) can cleanly layer on top as patches

If their fork is mature enough, converting to a submodule would give us:
- Upstream fixes for free (they track devkitPPC updates)
- Clear separation between "upstream bundled libwiigui" and "WBL GUI extensions"
- Patch stack consistency with how we handle curl/mbedtls/quickjs

## Patch stack (if we convert to submodule)

Currently **N/A** because libwiigui is bundled source. If we move it to `third_party/libwiigui/` as a submodule, create:

```
third_party/libwiigui-patches/
├── 0001-pngu-forward-to-canonical-header.patch
└── 0002-wbl-gui-extensions.patch (gui_favorite, gui_toolbar, etc.)
```

## Build integration

Only specific files from this directory are compiled — see `Makefile` variable `WBL_SRC`. The original libwiigui `pngu.c` is **excluded** because WBL uses the implementation in `src/utils/pngu.c` instead.
