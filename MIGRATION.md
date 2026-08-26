# Migration Guide — `source/` → `src`/`assets`/`external`

This guide helps contributors who worked on the pre-restructure tree (`source/` flat + `libs/` vendored, `gnu++0x`) to move to the Sprint 1-5 layout.

## Summary of Changes

| Before (≤ `57d2f4c`) | After (Sprint 5) | Notes |
|---|---|---|
| `source/*.cpp/*.h` flat (15+ files) | `src/core`, `src/ui`, `src/media`, `src/filesystem`, `src/archive`, `src/network`, `src/html_parser`, `src/text`, `src/utils` | `git log --follow` preserves history via `git mv` renames |
| `source/images`, `source/fonts`, `source/sounds`, `source/lang`, `source/include/master.css` | `assets/{images/appbar,fonts,sounds,lang,master.css}` | Embedded via `scripts/embeddata.sh` (was `bin2o`) |
| `source/libwiigui`, `source/litehtml`, `source/html`, `source/css` | `external/libwiigui`, `external/litehtml` | No longer in `SOURCES` as `source/html` |
| `source/mplayer` (77 MiB, 5708 files) | `external/mplayer` git submodule placeholder + `.gitmodules` | `config.h` has `#undef MPLAYER` by default; enable with `external/mplayer` populated + `CFLAGS -DMPLAYER` |
| `libs/include` + `libs/wii/*.a` (16) | Deleted — `dkp-pacman ppc-*` portlibs | `Makefile` `LIBDIRS:=$(PORTLIBS)`, `Dockerfile` installs `ppc-zlib` etc. |
| `HBC/` / `hbc/` clash (case-insensitive) | Canonical `HBC/` + `HBC/meta.xml.in` template | `make package` / CI uses `HBC/meta.xml.in` → `dist/.../meta.xml` via `scripts/gen_version.sh` |
| `template.pnproj/.pnps` | Deleted (`.gitignore` `*.pnproj`) | Use VS Code / `clangd` |
| `Makefile` `gnu++0x` flat `notdir` | `gnu++17` path-preserving `build/src/...` mirrors, `BUILD_OFILES`, `mkdir -p $(@D)`, `make docs/format/lint` | `DEVKITPRO=/tmp make -n all` dry-run verified |
| `common.h` transitive | `common.h` decoupled (`#include <string>` + forward decls), `src/core/context.h` `AppContext` | Include `gui.h`/`httplib.h` explicitly where needed |

## For Contributors

```sh
# Old workflow
make
# New workflow (same)
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=$DEVKITPRO/devkitPPC
make            # builds boot.elf/dol via build/src/... mirrors
make package    # → dist/apps/wiibrowser-lite/ + WiiBrowser-Lite-HBC.zip
make forwarder  # optional, folded into main (was separate)
make test       # host unit tests (needs g++, now 2 suites)
make docs       # doxygen
make lint       # clang-format --dry-run
```

## For Forks / Patches

- **History:** `git log --follow -- src/ui/menu.cpp` (was `source/menu.cpp`) still works due to rename detection.
- **Patches:** Rebase old patches with `git format-patch` and apply via `git am`; if `source/xxx` not found, use `git checkout -- src/xxx` mapping table in `ARCHITECTURE.md`.
- **Submodule:** After cloning, `git submodule update --init --recursive` (or ignore if not using MPLAYER).

## Rollback

To build old tree: `git checkout 57d2f4c -- source libs Makefile` (backport) — but prefer new layout.

## Checklist

- [ ] `git submodule update --init` if using MPLAYER
- [ ] `dkp-pacman -S ppc-*` per `BUILD.md`
- [ ] `git config core.hooksPath .githooks` for pre-commit
