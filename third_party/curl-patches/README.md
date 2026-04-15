# curl patch stack for WiiBrowser-Lite

These patches apply to `third_party/curl` (pinned to `curl-8_16_0`).
They are maintained in `git format-patch` format so each one is
individually submittable upstream as a pull request.

## How patches are applied

The top-level `Makefile` (via `third_party/curl.mk`) applies every
`*.patch` in this directory before running `autoreconf && configure
&& make` on the curl submodule. The application is idempotent:
`git apply --check` is used to skip patches that are already present.

To submit one of these as an upstream PR, copy it to a fresh clone
of `curl/curl` at the pinned tag and use `git am`:

```bash
cd curl
git checkout curl-8_16_0
git am /path/to/WiiBrowser-Lite/third_party/curl-patches/0001-*.patch
git push <your-fork> HEAD:my-topic-branch
```

## Patches

### 0001-vtls-mbedtls-gate-tls13-session-ticket-signal-on-proto-tls13.patch

**Title:** vtls/mbedtls: gate TLS 1.3 session-ticket signal on MBEDTLS_SSL_PROTO_TLS1_3

**Fix type:** build

**Summary:** `mbedtls_ssl_conf_tls13_enable_signal_new_session_tickets()`
is a TLS-1.3-only API in mbedTLS, but `lib/vtls/mbedtls.c` guards its
call only with `#ifdef MBEDTLS_SSL_SESSION_TICKETS`. An mbedTLS build
that keeps session tickets (for TLS 1.2 resumption) but omits TLS 1.3
(e.g. to avoid the PSA Crypto dependency) fails to link. Add the
missing `&& MBEDTLS_SSL_PROTO_TLS1_3` guard.

**Upstream status:** not submitted yet.

---

## Rules for adding new patches

1. Generate with `git format-patch -1 HEAD^..HEAD -o /tmp/`.
2. Name the patch `NNNN-<verb>-<short-description>.patch` where NNNN
   is the next sequential number.
3. Write a commit message that would stand on its own in a curl PR:
   one-line subject, blank line, paragraph explaining the problem
   and fix, Signed-off-by trailer.
4. Keep each patch to one logical change. If the upstream fix has
   multiple parts, use multiple patches.
5. Verify with `git apply --check` against a fresh checkout of the
   pinned tag.
