# Architecture — WiiBrowser Lite

## Overview

Wii homebrew browser (PPC750 “GEKKO”, libogc/GX). Entry at `src/core/main.cpp:215` `main()` → `InitVideo` → `SetupPads` → `fatInit` → `InitGUIThreads` → `InitFreeType` → `Settings.Load()` → `MainMenu(MENU_SPLASH)`.

## Directory Map (post-restructure)

- `src/core/` — `main`, `settings`, `handle` (form handling), `input`, `liste` (history), `config` (`#undef MPLAYER`, `WIIFLOW` flags), `sigslot.h`
- `src/ui/` — `menu.cpp` (God object to be split), `FreeTypeGX`, `filebrowser`, libwiigui wrappers
- `src/html_parser/` — `html.cpp` (`DisplayHTML` 445L), `htmlcxx.cpp` (`getTag`), `ParserDom/Sax`, `entities`, `css_lex.l`/`css_syntax.y` (generated), `Uri`, `litehtml` glue (`src/include/litehtml.h` → `external/litehtml`)
- `src/network/` — `httplib`, `networkop`, `transfer`, `update`, `urlcode`, `mime`
- `src/media/` — `video.cpp`, `audio.cpp`, `oggplayer.c` (disabled via `config.h:4` when `MPLAYER` off)
- `src/filesystem/` — `filelist.h`, `fileop` (SD/USB browsing)
- `src/archive/` — `Archive` strategy → `ZipFile/7ZipFile/RarFile`
- `src/text/` — `TextEditor`, `TextPointer`, `gettext`
- `src/utils/` — `pngu`, `mem2_manager`, `giflib`, `easybmp`, `stringop`, `timer`, `jmemsrc`
- `external/libwiigui/` — Tantric gui (GX) — `gui.h:1` 1393L monolith
- `external/litehtml/` — lightweight HTML renderer (fork)
- `external/mplayer/` — **git submodule** (ex-vendored `source/mplayer/`, ~85 MiB). Disabled by default (`DOCWRITE`, `IFRAME` flags). See `external/mplayer/README.md`.
- `external/mplayerwii/` — Wii-specific mplayer glue
- `assets/` — `images/`, `appbar/`, `fonts/*.ttf`, `sounds/*.pcm`, `lang/*.lang`, `master.css` — embedded via `scripts/embeddata.sh` (replaces `bin2o`) producing `_size` symbol
- `HBC/` — Homebrew Channel metadata (`meta.xml` templated with `<version>`/`<release_date>` in CI)
- `forwarder/` — HBC forwarder DOL (`forwarder/data/app_booter.dol` bin2s)
- `tools/` `scripts/` — `embeddata.sh`, version helpers
- `tests/unit` — host doctest harness (planned)

## Build

`Makefile:20` defines `SOURCES` covering `src/*`, `external/*`, `assets/*`; `VPATH:70` + `OFILES:97` flatten (known debt — path-preserving planned). `scripts/embeddata.sh` generates `.ttf.o` etc. `MPLAYER` at `external/mplayer` with `config.mak`/`config.h` gitignored.

## Known Debt (to address incrementally)

- Globals: `Settings` (`src/core/main.cpp:171`), `history` (`src/core/common.h:113`), `curl_handle` (`src/ui/menu.cpp:45`), `vmode` (`src/media/video.cpp:31`) — introduce `AppContext`.
- Long functions: `DisplayHTML` (`src/html_parser/html.cpp:203`), `HandleForm` (`src/core/handle.cpp:21`), `MenuSettings` (`src/ui/menu.cpp:1361`).
- Unsafe C: `sprintf`→`snprintf`, `fclose(NULL)` (`src/core/settings.cpp:125`), `strchr null` (`src/ui/menu.cpp:100`).
- Mixed naming: `ListaDiTesto` Italian vs English.
- `notdir` collision risk in Makefile (same filename in two `SOURCES` overwrites).

## Threading

MPlayer threads (`src/core/main.cpp:46` `mthread`/`cthread`, `cachestack`) guarded by `controlledbygui`; disabled when `MPLAYER` undef. GUI threads via `InitGUIThreads()`.

## Diagram

```
main → InitVideo/Input/Audio/FAT/GUI → Settings.Load → MainMenu → MenuHome
                                              │
                         httplib/networkop → Download → DisplayHTML → litehtml → GX
                                              │
                         filebrowser → fileop → Archive (ZIP/RAR/7Z)
```
