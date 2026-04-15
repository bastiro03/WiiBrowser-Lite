# WiiBrowser-Lite Cross-Platform Architecture

WiiBrowser-Lite is being evolved into a unified browser tech stack
that targets Nintendo Wii, DSi, DS, and Apple Mac Plus from a single
codebase, with feature flags controlling what gets compiled in.

## Platform capability matrix

| Capability       | Wii  | DSi  | DS   | Mac Plus |
|------------------|------|------|------|----------|
| RAM              | 88MB | 16MB | 4MB  | 1-4MB    |
| CPU              | PPC 729MHz | ARM 133MHz | ARM 67MHz | 68000 8MHz |
| HTTPS (mbedTLS)  | yes  | yes  | no   | no       |
| JavaScript (QuickJS) | yes | yes  | no   | no       |
| JPEG decoding    | yes  | yes  | no   | no       |
| Archives (zip/rar/7z) | yes | no | no | no       |
| CSS level        | 3    | 3    | 2    | 1        |

Capabilities are compile-time flags in `include/wbl/platform.h`. A no-JS
platform still includes `wbl/js_engine.h`; calls become no-ops via a
stub implementation. This lets shared code be written without `#ifdef`
spaghetti.

## Source layout

```
include/wbl/        Cross-platform headers (platform.h, js_engine.h, mbedtls_config.h)
src/wbl/            Cross-platform implementation (js_engine.c, ...)
src/                Wii-specific application code (gradually being abstracted)
platform/wii/       Wii build config (devkitPPC, libogc, GX rendering)
platform/dsi/       DSi build config (stub - devkitARM + libnds DSi mode)
platform/nds/       DS build config  (stub)
platform/macplus/   Mac Plus build config (stub - Retro68 toolchain)
third_party/quickjs MicroQuickJS minimal JS engine (~200KB)
third_party/mbedtls mbedTLS 3.6 LTS (TLS 1.2 + 1.3, ECDHE/AES-GCM only)
third_party/curl    libcurl 8.16 (HTTP/HTTPS only, no other protocols)
```

## Building

```bash
# Wii (default)
make WBL_PLATFORM=wii

# Future targets (currently stub - emit error with TODO)
make WBL_PLATFORM=nds
make WBL_PLATFORM=dsi
make WBL_PLATFORM=macplus

# Clean third-party builds too
make distclean WBL_PLATFORM=wii
```

## Memory budgets

`include/wbl/platform.h` declares per-platform memory budgets:

| Budget                | Wii    | DSi    | DS     | Mac Plus |
|-----------------------|--------|--------|--------|----------|
| `WBL_MEM_BUDGET_DOM_KB`     | 16384 | 4096 | 1024 | 256 |
| `WBL_MEM_BUDGET_NETBUF_KB`  | 512   | 128  | 32   | 16  |
| `WBL_MEM_BUDGET_JS_HEAP_KB` | 4096  | 1024 | 0    | 0   |

The DOM tree, network buffers, and JS heap each clamp to these to
prevent runaway pages from OOM-killing the browser.

## TLS configuration

`include/wbl/mbedtls_config.h` defines a minimal mbedTLS profile
suitable for browsing modern HTTPS sites in 2026. We support only:

- TLS 1.3 (preferred), TLS 1.2 (fallback)
- Cipher suites: ECDHE-ECDSA + ECDHE-RSA with AES-128-GCM or ChaCha20-Poly1305
- Curves: secp256r1, secp384r1, X25519
- 4KB record buffers (default 16KB) - saves 24KB RAM per connection

This is enough to negotiate with `wikipedia.org`, `github.com`, and most
other modern sites without dragging in legacy crypto.

## JavaScript configuration

QuickJS-ng is built with `-Os -flto` and our "MicroQuickJS" macros
disable BigInt/BigFloat/Atomics/Storage. The result is ~200KB of code
plus configurable heap (1MB on DSi, 4MB on Wii).

For pages like the Wikipedia "Nintendo DS" article, the inline `<script>`
content is typically MediaWiki ResourceLoader bootstrap - we run it but
ignore network-fetched scripts (MathJax, etc.) since we don't ship a
full HTTP-fetching JS environment.
