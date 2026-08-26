# Tools

- `scripts/embeddata.sh` — embeds `assets/*` via `powerpc-eabi-gcc -x assembler-with-cpp` (replaces `bin2o`, adds `_size` symbol).
- Future: `gen-version.sh` for HBC meta templating (currently in CI via `sed`).

Run `make distclean` to remove generated `src/html_parser/css_lex.*` etc.
