# WiiBrowser Lite — development build

An experimental WiiBrowser revival for **original Wii / Homebrew Channel**.
**Not ready for public release:** secure Wii TLS integration, hardware testing
and the mixed-source license review remain required. See [RELEASE.md](RELEASE.md).

## Current scope

- Legacy htmlcxx/custom libwiigui HTML/CSS rendering, links and URL-encoded forms.
- HTTP; verified HTTPS requires a suitable TLS-enabled SDK. The supplied Docker
  development build is HTTP-only. PNG/JPEG images within bounded dimensions.
- Bookmarks, bounded history, on-screen/USB keyboard and SD/USB downloads.
- No JavaScript, iframe injection, local Lua, file uploads, extraction, self-update,
  forwarder, GIF/BMP decoding or video playback. Legacy implementations in git
  are **not** evidence that these features are enabled or safe.
- Modern complex sites, authentication flows and large pages are not supported
  reliably. This is not a suitable browser for sensitive accounts.
- Development limits: 2 MiB responses, 1-megapixel source images, bounded page/
  form/URL structures and aggregate texture/glyph budgets. Compressed PNG metadata
  is rejected. Unknown-size images reserve fixed layout space; asynchronous image
  reflow remains incomplete. Rejected/over-budget content may show placeholders.

## Build and checks

No packages are installed by the build. Host checks require Python 3, GNU Make
and a native C++17 compiler; parser smoke tests also require flex/bison:

```sh
make check                 # Resource/source invariants, not a compilation test
make test                  # Also compile/run portable ASan/UBSan tests
```

Wii builds additionally require an existing devkitPro/devkitPPC SDK, GNU Make
4.3+, flex, bison, target pkg-config metadata, libogc/libfat, libcurl,
FreeType, libpng 1.6 simplified read/write, libjpeg-turbo, mxml 3.x, fribidi
and Tremor/ogg. HTTPS additionally requires a reviewed TLS/entropy backend.
Do not mix the removed prebuilt 2012 libraries with modern headers.

```sh
export DEVKITPRO=/opt/devkitpro
export PATH="$DEVKITPRO/devkitPPC/bin:$DEVKITPRO/tools/bin:$PATH"
make -j2
make package               # Development ZIP only; never publishes a release
```

`CURL_LIBS` may be supplied explicitly from the **target** `libcurl.pc` when host
pkg-config cannot resolve its TLS/compression dependencies. The build does not
guess a TLS ABI. Generated assets/parser files live under `build/`. CI runs host
checks only; it does not claim a Wii build or execute a Wii binary.
Unused compressed metadata is omitted from compiled PNG copies; source art and
the image-data chunks are kept unchanged.

### Docker development build

With Docker Desktop running (and WSL integration enabled when using WSL):

```sh
docker build -f scripts/Dockerfile.wii -t wiibrowser-lite-build .
docker run --rm -v "$PWD:/work" wiibrowser-lite-build
```

This explicitly installs build dependencies **inside the image only**. The SDK
image digest and curl source hash are pinned in `scripts/`. Output is
`boot.dol` and the debug-symbol ELF `boot.elf` in the repository root.
Clean compilation was verified with devkitPPC r50 (GCC 16.1.0), libogc 3.1.0
and curl 8.23.0. This does not establish runtime compatibility.

**This is an HTTP-only development build.** No reviewed strong Wii entropy
source is available, so this container builds curl without a TLS backend.
HTTPS returns an unsupported-protocol error; it is not downgraded to HTTP.
Clock/device-ID pseudo-entropy and certificate-verification bypasses are not
used. A secure TLS/entropy integration and hardware tests are still required;
do not use this development executable for sensitive data.
The sample configuration's HTTPS homepage will not load in this variant; enter
an explicit `http://` URL (for example `http://example.com/`) for testing.

## Development installation

Back up existing app data first. Extract the development ZIP at the root of a
FAT SD/USB device:

```text
apps/wiibrowser/
  boot.dol
  meta.xml
  icon.png
  wiibrowser.cfg
  cacert.pem
  licenses/
```

Keep `cacert.pem` beside `boot.dol`; verification is never disabled when it is
missing. Set the Wii clock correctly. HBC `argv[0]` selects the app directory;
`wiibrowser-lite` is a fallback discovery path, not a second mandatory install.
The optional first HBC argument may be an HTTP(S) URL, `homepage`, or
`bookmark_0` through `bookmark_8`; arbitrary argument files are not executed or
interpreted as settings commands.

### Controls

Wii Remote: point/A to select, D-pad to scroll, B+IR to drag-scroll, Plus/Minus
to open/close the page toolbar, HOME to exit. Button 1 saves a screenshot;
button 2 restores the view. Console RESET returns to Wii Menu; POWER requests
standby. Classic/GC input, including disconnects and no-IR operation, still
requires the hardware matrix in RELEASE.md. USB keyboard is active only in its
modal editor; input is bounded to its actual capacity.

## Storage, recovery and uninstall

- Downloads first create an exclusive `name.part`; success checks HTTP/I/O and
  then commits the destination. Failed/cancelled downloads retain `.part` data.
  Automatic range resume is disabled; move/remove reviewed partials yourself.
- Replacing a download requires confirmation and leaves `name.download.bak`.
  A pre-existing backup is never overwritten; choose another destination.
- Settings/bookmarks/history use `.new`/`.bak` generations. An interrupted or
  invalid load is preserved; recovered state is read-only rather than silently
  resetting files. Close the app, back up **all** generations on a computer,
  inspect the files and restore the desired valid generation to the live name.
  Remove stale `.new` only after preserving it and confirming a valid live copy.
- Saved pages/screenshots retain an existing destination as `.bak`; collisions
  fail instead of destroying an earlier backup. FAT is not transactional: no
  arbitrary power-cut guarantee is claimed.
- **Do not remove or exchange writable media while the app runs.** Detection and
  shutdown paths need real-media verification; back up test media.
- To uninstall, copy any desired downloads/bookmarks elsewhere, then remove the
  app directory on a computer. No NAND/title/IOS installation is required.

Report bugs at <https://github.com/bastiro03/WiiBrowser-Lite/issues>. Include the
source revision, exact SDK/dependency versions, loader, controller, video mode,
media type and reproduction steps. The old third-party HTTP reporting endpoint
is disabled; no report or console identifier is sent to it.

## Credits and licensing

Original WiiBrowser: [gave92](https://github.com/gave92). File/UI foundations:
Dimok's WiiXplorer and dborth/Tantric's libwiigui/WiiMC. Legacy parsing: htmlcxx
contributors and Kasper Peeters' tree library. LiteHTML/MPlayer material remains
in the repository but is not the active browser renderer/player.

Calibri has been replaced with unmodified DejaVu Sans 2.37 fonts and their
notices. See [THIRD_PARTY.md](THIRD_PARTY.md). The repository's root GPLv3 file
does not resolve GPLv2-only/GPLv3-or-later component conflicts; redistribution
must wait for that review. This is not legal advice.
