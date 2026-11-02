# Development status — not release-cleared

Source changes implement the approved first-release containment strategy:
standalone HBC; no upload commands, extraction, updater, forwarder, Lua/iframe/
script rewriting, GIF/BMP decoding or playback. Dormant source is not certified.

Implemented safeguards include checked destination capacities, exclusive
partials, original-file retention, read-only recovery, bounded history/config,
explicit POST bodies, strict TLS, controller dummy state, GUI serialization,
main-owned shutdown, bounded PNG/JPEG adapters and GPU capture fencing.

## Verification still required

- **Host verification:** `make test` passes seven source/resource checks, portable
  ASan/UBSan bounds/URL/DOM/recovery tests, and generated CSS parser/lexer compile
  and sanitizer smoke tests. Native g++ 15.2, flex 2.6.4 and bison 3.8.2 were
  installed with explicit permission. These tests are not a full application build.
- **Wii compile:** a clean HTTP-only development build now produces `boot.dol`
  and `boot.elf` with devkitPPC r50/GCC 16.1.0, libogc 3.1.0, libfat 2.1.0,
  FreeType 2.14.1 and curl 8.23.0. The PowerPC ELF header and DOL load sections/
  entry point were checked. Compiler warnings remain. Neither executable was
  run in Dolphin or on Wii hardware.
- **TLS gate:** the supplied Docker image intentionally builds curl without
  TLS. HTTPS is unsupported, not downgraded. The available third-party Wii
  entropy patch uses predictable clock/device data and was not adopted. A
  reviewed strong entropy source and a clean TLS-enabled build remain required.
- **Reproducibility:** the base SDK image digest and curl source SHA-256 are
  pinned in `scripts/Dockerfile.wii` and `scripts/wii-deps.sh`. The clean-build
  command/log, SDK package inventory, library hashes and output checksums are
  recorded under `build/`; retain these generated records with test artifacts.
- **Hardware gate:** test zero/one/four Wii Remotes, PAD-only/Classic input and
  hotplug; PAL50/PAL60/NTSC, 4:3/16:9, composite/component; SD-only/USB-only/dual
  media and alternate HBC directories; blocked/offline networking and HOME/reset/
  power while loading, downloading or displaying modals.
- **Data integrity gate:** disposable backed-up media only. Inject failed opens,
  short reads/writes, full/removed media, reset/power during saves, interrupted
  generation replacement and replacement with another volume. No dirty-cache
  write to replacement media may be assumed safe without verification.
- **Soak:** repeated forms/bookmarks/navigation, concurrent downloads, unusual
  glyphs, screenshots and multi-hour sessions; record peak/retained MEM1/MEM2.
  Also verify unknown-image layout/reflow; fixed placeholder spacing remains a
  development limitation rather than a completed renderer feature.
- **Distribution gate:** resolve GPLv2-only/GPLv3-or-later combination and art/audio
  provenance; include exact corresponding source and required dependency notices.
  Replacing fonts alone does not clear the application license combination.

`make package` produces a development ZIP only. Nothing here automatically runs
a Wii binary, flashes hardware, installs packages, publishes artifacts or creates
a GitHub release. Public release approval remains a separate decision after the
above evidence exists.

## Roadmap disposition

- R1–R6: safety containment implemented; legacy backends are gated, not certified.
- R7: build inputs/generation, native checks and a pinned HTTP-only Wii build
  are verified; TLS-enabled compilation and runtime validation remain open.
- R8–R24: source hardening implemented across input, text, state, requests,
  ownership, storage and video; platform-specific behavior remains hardware-gated.
  Image placeholder reflow remains incomplete.
- R25: standalone development package/metadata and truthful limitations supplied.
- R26: redistributable font replacement/provenance supplied; application/dependency
  license compatibility and art/audio clearance remain external release blockers.
