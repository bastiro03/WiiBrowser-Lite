# WiiBrowser Lite

[![Build Status](https://github.com/matthargett/WiiBrowser-Lite/actions/workflows/build.yml/badge.svg)](https://github.com/matthargett/WiiBrowser-Lite/actions/workflows/build.yml)

Wiibrowser-lite is an alternative to the Wii Internet Channel. It is also a new version of Wiibrowser by [gave92](https://github.com/gave92). I want to provide a more modern base that makes things easier for other developers. Contributions through pull requests and similar methods are welcome, even though I am myself no longer actively working on this project.

## Quick Start

Build the latest version with one command:

```bash
./docker-build.sh
```

Or download pre-built binaries from the [releases page](https://github.com/matthargett/WiiBrowser-Lite/releases) or [GitHub Actions](https://github.com/matthargett/WiiBrowser-Lite/actions).
## Screenshots
<p><img src="screenshots\advanced.png" width="100"> <img src="screenshots\bookmarks.png" width="100"> <img src="screenshots\download.png" width="100"> <img src="screenshots\file_browser.png" width="100"> <img src="screenshots\gmail.png" width="100"> <img src="screenshots\google.png" width="100"> <img src="screenshots\homescreen.png" width="100"> <img src="screenshots\settings.png" width="100"></p>

## Features

### Currently implemented
* View your own web pages
* **Modern HTTPS with certificate verification** (Wikipedia, GitHub, etc.)
* **Link Time Optimization (LTO)** for smaller, faster binaries
* Links and web form support
* Bookmarks
* Import/Export of bookmarks
* Basic rendering of HTML 4.01 and CSS2
* Download support to an SD/SDHC card
* Upload support
* Address bar with on-screen keyboard
* Keyboard auto-completion
* Forward and back navigation
* PNG/JPEG/GIF/BMP image support
* ZIP/RAR/7Z extraction
* Moving and resizing images
* Auto-refresh

### Planned
* Better HTML/CSS rendering
* JavaScript support
* Full video support
* Multiple tabs

## Building

See [BUILD.md](BUILD.md) for detailed build instructions.

**Quick build with Docker:**
```bash
./docker-build.sh
```

**Requirements:**
- Docker (recommended) - [Get Docker](https://docs.docker.com/get-docker/)
- Or devkitPPC r47+ with GCC 15.1.0

## Credits
- [Dimok](https://github.com/dimok789): developed [WiiXplorer](https://sourceforge.net/projects/wiixplorer/), which WiiBrowser was both inspired by and stole code snippets from for its file management backend
- [dborth](https://github.com/dborth): developed [libwiigui](https://github.com/dborth/libwiigui), the graphics backend used by WiiBrowser, and the original [WiiMC](https://github.com/dborth/wiimc) which WiiBrowser stole code snippets from for its media support
- [SuperrSonic](https://github.com/SuperrSonic): develops [WiiMC-SSLC](https://github.com/SuperrSonic/WiiMC-SSLC), parts of which WiiBrowser Lite uses for its media backend
- [The LiteHTML contributors](https://github.com/litehtml): develop the lightweight rendering engine that WiiBrowser uses

## Links
- GBATemp thread: https://gbatemp.net/threads/wii-browser.342634/
- Original webpage (archived): https://web.archive.org/web/20131002190414/http://wiibrowser.altervista.org/mainsite/index.html
- Original source code (archived): https://code.google.com/archive/p/wiibrowser/
