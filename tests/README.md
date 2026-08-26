# Tests

Host-side unit tests (doctest / native `g++`, not devkitPPC).

```
make test        # runs tests/Makefile if present
```

Planned suites:
- `url` — `adjustUrl`/`getHost` (src/ui/menu.cpp)
- `archive` — Zip/7z/Rar strategy
- `html_parser` — litehtml + htmlcxx parsing
- `stringop` — utils/stringop

Add new tests under `tests/unit/*.cpp` with doctest single-header.
