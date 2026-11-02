#!/usr/bin/env python3
"""Generate bounded asset metadata and aligned PowerPC assembly; never run it."""
import argparse
import pathlib
import re
import struct


def symbol(path):
    value = re.sub(r"[^A-Za-z0-9_]", "_", pathlib.Path(path).name)
    if not value or value[0].isdigit():
        raise ValueError("asset name is not a C identifier")
    return value


def compiled_png(data):
    """Omit unused compressed metadata, retaining source/pixel chunks verbatim."""
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("invalid embedded PNG signature")
    result = bytearray(data[:8])
    cursor = 8
    while cursor < len(data):
        if len(data) - cursor < 12:
            raise ValueError("truncated embedded PNG chunk")
        length = struct.unpack_from(">I", data, cursor)[0]
        end = cursor + length + 12
        if end > len(data):
            raise ValueError("oversized embedded PNG chunk")
        kind = data[cursor + 4:cursor + 8]
        if kind not in (b"iCCP", b"zTXt", b"iTXt"):
            result.extend(data[cursor:end])
        cursor = end
    return bytes(result)


def headers(output, assets):
    names = [symbol(path) for path in assets]
    if len(set(names)) != len(names):
        raise ValueError("duplicate asset symbol")
    lines = ["#ifndef WIIBROWSER_EMBEDDED_ASSETS_H", "#define WIIBROWSER_EMBEDDED_ASSETS_H", "#include <stddef.h>"]
    for path in assets:
        if pathlib.Path(path).suffix == ".png":
            name = symbol(path)
            lines += [f"extern const unsigned char {name}[];", f"extern const unsigned int {name}_size;"]
    lines += ["inline size_t EmbeddedPngSize(const unsigned char *source) {", "    if (!source) return 0;"]
    for path in assets:
        if pathlib.Path(path).suffix == ".png":
            name = symbol(path)
            lines.append(f"    if (source == {name}) return {name}_size;")
    lines += ["    return 0;", "}", "#endif", ""]
    pathlib.Path(output).write_text("\n".join(lines), encoding="utf-8")


def embed(source, output):
    path = pathlib.Path(source).resolve(strict=True)
    name = symbol(path)
    if path.suffix.lower() == ".png":
        generated = pathlib.Path(str(output) + ".data")
        generated.write_bytes(compiled_png(path.read_bytes()))
        path = generated.resolve(strict=True)
    quoted = str(path).replace("\\", "\\\\").replace('"', '\\"')
    lines = [".section .rodata", ".balign 32", f".global {name}", f".type {name}, @object", f"{name}:",
             f'.incbin "{quoted}"', f".L{name}_end:", f".size {name}, .L{name}_end-{name}",
             ".balign 32", f".global {name}_size", f".type {name}_size, @object", f"{name}_size:",
             f".long .L{name}_end-{name}", f".size {name}_size, 4", ".balign 32", ""]
    pathlib.Path(output).write_text("\n".join(lines), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    header = commands.add_parser("headers")
    header.add_argument("output")
    header.add_argument("assets", nargs="+")
    assembly = commands.add_parser("embed")
    assembly.add_argument("source")
    assembly.add_argument("output")
    args = parser.parse_args()
    if args.command == "headers":
        headers(args.output, args.assets)
    else:
        embed(args.source, args.output)
