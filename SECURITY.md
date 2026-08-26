# Security Policy

## Supported Versions

| Version | Supported |
|---|---|
| `main` (Sprint 5, `2b04324b`) | ✅ |
| `57d2f4c` and earlier | ⚠️ — use at own risk, no portlibs |

## Reporting

Use **GitHub Security Advisories** or email `jgracia9988@gmail.com` (as in `security_report.yml`). Do not post exploit details in public issues.

Include: Wii system version, `boot.dol` build (CI `#` or `git rev-parse --short HEAD`), steps to reproduce, and whether `external/mplayer` was enabled.

## Scope

- `src/network` (`httplib`, `curl`, `url_helper`) — URL parsing, TLS (wolfSSL/cyassl via portlibs)
- `src/html_parser` (`litehtml`, `html_helpers`) — HTML/CSS parsing (fuzz target planned)
- `src/archive` (ZIP/7Z/RAR) — archive extraction

Third-party vendored code (`external/*`) retains upstream security posture; see `docs/THIRD_PARTY.md`.

We aim to acknowledge within 72h and patch in next `dist` release.
