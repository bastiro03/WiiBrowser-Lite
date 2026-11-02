#!/usr/bin/env python3
"""Remove only this build system's default products; never delete broad paths."""
import pathlib
import shutil

root = pathlib.Path(__file__).resolve().parents[1]
build = root / "build"
if build.is_symlink():
    raise SystemExit("Refusing to clean a symlinked build directory")
if build.is_dir():
    shutil.rmtree(build)
for name in ("boot.elf", "boot.dol"):
    path = root / name
    if path.is_file() and not path.is_symlink():
        path.unlink()
