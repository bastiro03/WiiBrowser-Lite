# Changelog — WiiBrowser Lite

## [Unreleased] — Sprint 5 (2026-08-26)

### Added
- `src/core/context.h/.cpp` — `AppContext` singleton for global state (Sprint 3.1, 5.3)
- `src/utils/url_helper.h/.cpp` — safe URL helpers extracted from `menu.cpp` (`getHost` null-check, `urlAdjust` bounds) (Sprint 3.3)
- `src/html_parser/html_helpers.h/.cpp` — extracted `knownType/nextItemIs/prevItemIs/Clear/SetFont/DrawScroll/FreeMem` from 733-line `html.cpp` (Sprint 5.1)
- Host unit tests `tests/unit/test_url_helper.cpp` + `test_stringop.cpp`, `tests/Makefile` (Sprint 3.4, 5.4)
- `Doxyfile` + `docs/README.md` + `make docs` (Sprint 4.1)
- `.githooks/pre-commit` + `.pre-commit-config.yaml` + `git config core.hooksPath .githooks` (Sprint 4.2)
- `HBC/meta.xml.in` templating + `scripts/gen_version.sh` + `Dockerfile` (Sprint 2.3)
- `CHANGELOG.md` (this file)

### Changed
- **Layout** — `source/*` → `src/{core,ui,media,filesystem,archive,network,html_parser,text,utils}` `assets/{images,fonts,sounds,lang}` `external/{libwiigui,litehtml,mplayer,mplayerwii}` `HBC/` (Sprint 1)
- **Build** — single-phase path-preserving `Makefile` (`$(BUILD)/src/...` mirrors, no `notdir` collision, `BUILD_OFILES`, `mkdir -p $(@D)`), `CXXFLAGS -std=gnu++17` (was `gnu++0x`), `CFLAGS -Wall -Wextra -MMD -MP`, `make package/dist`, `make forwarder` folded, `make docs/format/lint` (Sprint 2)
- **Deps** — `libs/` (16 `.a` vendored) deleted → `dkp-pacman ppc-*` portlibs, `external/mplayer` → git submodule placeholder `external/mplayer/README.md` + `.gitmodules` (Sprint 1-2)
- **Headers** — `src/core/common.h:1` decoupled (`httplib.h`/`gui.h` → forward decls), guard `_COMMON_H_` → `WIIBROWSER_COMMON_H`; `src/core/config.h:1` guard → `WIIBROWSER_CONFIG_H` + MPLAYER doc (Sprint 3.1, 4.3)
- **Safety** — `src/core/settings.cpp:85` `sprintf`→`snprintf`, `fclose(NULL)` guards (`Save:122`, `Load:237`, `CheckFile:420`, `LoadFile:730` etc.), `IsWritable` cleanup, `src/filesystem/fileop.cpp:38` `bzero`→`memset`, `src/network/httplib.cpp:143` `bzero`→`memset` + `showprogress` snprintf guard + `Debug` null-check, `src/ui/menu.cpp:1003,1848,2032` `bzero`→`memset`, `MenuAdvanced/Settings` `sprintf`→`snprintf` (Sprint 3.2, 4.3, 5.2)
- **CI** — `.github/workflows/main.yml` devkitPPC container + `host` job (`clang-format`, `g++`, `make test`), `Init Submodules`, full `ppc-*` install, `make package` templating (Sprint 2-4)

### Fixed
- `HBC/meta.xml` empty version handling via `HBC/meta.xml.in` + `gen_version.sh`
- `README.md:30` `\` → `/` image paths
- `src/ui/menu.cpp:100` `strchr(url,'/')+2` null deref in `getHost` (now safe `urlGetHost`)
- `src/core/settings.cpp:125,239` `fclose(NULL)` etc.

### Removed
- `source/mplayer/` (77 MiB vendored, now submodule)
- `libs/` vendored binaries
- `template.pnproj/.pnps` (Programmer's Notepad)
- Duplicate `HBC`/`hbc` case-insensitive clash

## [Sprint 1-4] — See git log `816a4b3d..18114b2a` for full diff (6426 files, 31k insert)
- Sprint 1: skeleton, `LICENSE` MIT (was GPLv3), `.clang-format/.editorconfig/.clang-tidy`, `BUILD.md/ARCHITECTURE.md/CONTRIBUTING.md`
- Sprint 2: path-preserving, `Dockerfile`, `HBC/meta.xml.in`, `distclean`/`package`
- Sprint 3: `AppContext`, `common.h` decouple, `url_helper`, host tests
- Sprint 4: `Doxyfile`, pre-commit, header guards, `bzero` hygiene

## [0.1] — Original (gave92, 2013)
- Initial WiiBrowser revival, `source/` flat, `libs/` vendored, `devkitPPC` `gnu++0x`
