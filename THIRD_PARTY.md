# Third-party provenance and unresolved distribution review

This is an inventory of known evidence, **not** a declaration that all rights or
license combinations have been cleared. Existing source notices remain controlling.

| Component | Evidence/status |
|---|---|
| WiiBrowser HTTP/parsing glue and imported tree code | GPLv2-only notices are present; do not silently relicense them |
| Adapted GUI long text/frame image | GPLv3-or-later notices are present; compatibility with GPLv2-only code remains unresolved |
| FreeTypeGX | LGPL notice in `src/ui/FreeTypeGX.*`; preserve applicable notices/source |
| libwiigui, WiiXplorer/WiiMC-derived integration | Preserve per-file authors/notices; exact upstream revisions and modifications need a final manifest |
| libogc/portlibs | Actual linked package versions/licenses must be captured from the release SDK; no obsolete binary fallback is used |
| Art/PCM effects | Historical project assets; standalone provenance/redistribution record still needs review |
| Opaque forwarder booter | Not rebuilt or audited; forwarder is quarantined and excluded from packaging |

## Fonts

Unmodified DejaVu Sans 2.37 regular, bold and oblique files replace Calibri.
Only their filenames are aliased to the application's existing asset symbols.
Font notices are in `assets/fonts/LICENSE-DejaVu.txt` and included in the bundle.

Source: <https://github.com/dejavu-fonts/dejavu-fonts/releases/tag/version_2_37>
(`dejavu-fonts-ttf-2.37.zip`). Archive SHA-256:
`7576310b219e04159d35ff61dd4a4ec4cdba4f35c00e002a136f00e96a908b0a`.

| Local alias | Upstream file | SHA-256 |
|---|---|---|
| `font_regular.ttf` | `DejaVuSans.ttf` | `7da195a74c55bef988d0d48f9508bd5d849425c1770dba5d7bfc6ce9ed848954` |
| `font_bold.ttf` | `DejaVuSans-Bold.ttf` | `e6476c1b80502924294eed40894c5b18e06c181444ca953e5334262df9c27724` |
| `font_italic.ttf` | `DejaVuSans-Oblique.ttf` | `4af75fa16ee6d3ad43e1ecec41862c24954af26a55c6bb1ebb27bd486a50f5f4` |

## Trust store

`HBC/cacert.pem` was imported as data from <https://curl.se/ca/cacert.pem> and
checked against its HTTPS-served `.sha256`. It contains 118 Mozilla CA roots,
with source date 2026-10-20. Its original header is retained. The complete file's
checksum is recorded in `HBC/cacert.sha256`; the SHA printed inside the bundle
header identifies the upstream source data and is not the PEM file checksum.
Review Mozilla/curl trust-store provenance/notices and update policy before a
public release. No downloaded code or font/certificate data was executed.
