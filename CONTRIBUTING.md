# Contributing

## Workflow

- Branch from `main`, open PR against `main`.
- Run `clang-format` (`.clang-format:1`) and ensure `.editorconfig` compliance.
- Build via `make` (or Docker) and `make forwarder` if touching forwarder.
- Add tests in `tests/unit` for pure logic (URL, Archive, stringop).

## Code Style

- C++17 (`Makefile:41` `gnu++17`), `MSDATA` Wii flags preserved.
- No `TODO` without issue link; no commented-out code.
- Prefer `snprintf`, `std::string`, RAII over raw `new`/`malloc`.

## Commit

- Conventional commits (`feat:`, `fix:`, `chore:`).
- Preserve history with `git mv` when moving files.

## License

MIT (see `LICENSE:1`). Vendored deps retain their licenses.
