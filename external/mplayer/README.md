# external/mplayer — git submodule (extracted from vendored source/mplayer)

This directory is a **git submodule** placeholder per Sprint 1 Q1.

- Original vendored tree was `source/mplayer/` (~85 MiB, 2925 files, FFmpeg snapshot).
- Extracted to `external/mplayer` as submodule to avoid bloating main repo.
- Disabled by default (`src/core/config.h:4` `#undef MPLAYER`); enable needs `external/mplayer` populated.

## Setup

```sh
git submodule add https://github.com/mplayerhq/mplayer.git external/mplayer
# or use ffmpeg mirror if preferred
git submodule update --init --recursive
# then run external/mplayer/configure with devkitPPC cross flags
# generated files external/mplayer/config.mak/config.h are gitignored (see .gitignore)
```

If submodule empty, build skips MPlayer (`Makefile:49` LIBS excludes `-lmplayerwii`).
CI does not require it; host `make` succeeds without.
