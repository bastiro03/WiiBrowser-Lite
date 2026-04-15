#!/bin/bash
# Validate a Wii .dol binary.
#
# The .dol format (used by Wii/GameCube homebrew) is documented at
# https://wiibrew.org/wiki/DOL. A valid .dol has:
#   - A 256-byte header with text/data section offsets, sizes, BSS
#     info, and entry point (all big-endian u32's)
#   - Up to 7 text sections and 11 data sections
#   - Entry point pointing somewhere inside one of the sections
#
# We also verify the companion .elf is a big-endian PowerPC executable
# with a reasonable text size (non-empty) and that objdump can walk
# its sections.
#
# Usage: scripts/verify-dol.sh <path-to-dol> [<path-to-elf>]
# Returns non-zero on any sanity-check failure so CI fails loudly.

set -euo pipefail

DOL="${1:-wiibrowserlite.dol}"
ELF="${2:-${DOL%.dol}.elf}"

red()   { printf "\033[31m%s\033[0m\n" "$*"; }
green() { printf "\033[32m%s\033[0m\n" "$*"; }
blue()  { printf "\033[34m%s\033[0m\n" "$*"; }

# Which objdump do we have? Prefer the devkitPPC cross tool, fall
# back to a system powerpc objdump if present.
OBJDUMP=""
for candidate in \
    "${DEVKITPPC:-}/bin/powerpc-eabi-objdump" \
    "/opt/devkitpro/devkitPPC/bin/powerpc-eabi-objdump" \
    "$(command -v powerpc-eabi-objdump || true)" \
    "$(command -v powerpc-linux-gnu-objdump || true)" \
    "$(command -v objdump || true)"; do
    if [[ -x "$candidate" ]]; then
        OBJDUMP="$candidate"
        break
    fi
done
if [[ -z "$OBJDUMP" ]]; then
    red "no objdump available"
    exit 2
fi
blue "using objdump: $OBJDUMP"

# --- 1. File existence ---------------------------------------------------
if [[ ! -f "$DOL" ]]; then
    red "missing: $DOL"
    exit 1
fi
if [[ ! -f "$ELF" ]]; then
    red "missing: $ELF"
    exit 1
fi

DOL_SIZE=$(stat -c %s "$DOL")
ELF_SIZE=$(stat -c %s "$ELF")
green "[OK] .dol exists: $DOL ($DOL_SIZE bytes)"
green "[OK] .elf exists: $ELF ($ELF_SIZE bytes)"

# --- 2. .dol header sanity ------------------------------------------------
# Read big-endian u32 at offset $1 from $DOL and echo as decimal.
beu32() {
    local off=$1
    xxd -s "$off" -l 4 -g 1 "$DOL" | awk '{printf "%d\n", strtonum("0x"$2$3$4$5)}'
}

# text section 0 offset lives at file offset 0x00
# data section 0 offset lives at file offset 0x1C
# entry point lives at file offset 0xE0
# total memory image size lives at (implicit - we compute from headers)
TEXT0_FOFF=$(beu32 0x00)
DATA0_FOFF=$(beu32 0x1C)
TEXT0_LADDR=$(beu32 0x48)
DATA0_LADDR=$(beu32 0x64)
TEXT0_SIZE=$(beu32 0x90)
DATA0_SIZE=$(beu32 0xAC)
BSS_ADDR=$(beu32 0xD8)
BSS_SIZE=$(beu32 0xDC)
ENTRY=$(beu32 0xE0)

printf "  .dol header:\n"
printf "    text[0] file_off=0x%X  load=0x%X  size=0x%X\n" "$TEXT0_FOFF" "$TEXT0_LADDR" "$TEXT0_SIZE"
printf "    data[0] file_off=0x%X  load=0x%X  size=0x%X\n" "$DATA0_FOFF" "$DATA0_LADDR" "$DATA0_SIZE"
printf "    bss     load=0x%X  size=0x%X\n" "$BSS_ADDR" "$BSS_SIZE"
printf "    entry   0x%X\n" "$ENTRY"

# Entry point must be within Wii main RAM (MEM1: 0x80000000-0x817FFFFF
# or MEM2: 0x90000000-0x93FFFFFF in HW mapping).
if (( ENTRY < 0x80000000 || (ENTRY >= 0x81800000 && ENTRY < 0x90000000) || ENTRY >= 0x94000000 )); then
    red "[FAIL] entry point 0x$(printf '%X' $ENTRY) outside valid Wii RAM range"
    exit 1
fi
green "[OK] entry point in valid Wii RAM range"

# text[0] must be non-empty for a non-trivial program.
if (( TEXT0_SIZE < 4096 )); then
    red "[FAIL] text[0] size $TEXT0_SIZE bytes is suspiciously small"
    exit 1
fi
green "[OK] text[0] size sane ($TEXT0_SIZE bytes)"

# text[0] file offset must be >= 0x100 (header size).
if (( TEXT0_FOFF < 0x100 )); then
    red "[FAIL] text[0] file offset 0x$(printf '%X' $TEXT0_FOFF) overlaps DOL header"
    exit 1
fi
green "[OK] text[0] file offset past header"

# text[0] must fit inside the file.
if (( TEXT0_FOFF + TEXT0_SIZE > DOL_SIZE )); then
    red "[FAIL] text[0] extends past end of file"
    exit 1
fi
green "[OK] text[0] fits in file"

# --- 3. ELF file-format check via objdump --------------------------------
ELF_HDR=$("$OBJDUMP" -f "$ELF")
echo "$ELF_HDR" | sed 's/^/    /'

if ! echo "$ELF_HDR" | grep -q "elf32-powerpc"; then
    red "[FAIL] .elf is not PowerPC 32-bit - got:"
    echo "$ELF_HDR" | grep "file format"
    exit 1
fi
green "[OK] .elf format is elf32-powerpc"

# --- 4. ELF section verification -----------------------------------------
# We want to see .text, .rodata, .data, .bss at minimum, all non-empty
# (except .bss which is runtime-only).
REQUIRED_SECTIONS=(.text .rodata .data)
for section in "${REQUIRED_SECTIONS[@]}"; do
    size=$("$OBJDUMP" -h "$ELF" | awk -v s="$section" '$2 == s {print strtonum("0x"$3); exit}')
    if [[ -z "$size" || "$size" -eq 0 ]]; then
        red "[FAIL] ELF section $section is missing or empty"
        exit 1
    fi
    printf "    %-12s %d bytes\n" "$section" "$size"
done
green "[OK] required ELF sections present and non-empty"

# --- 5. Symbol sanity ----------------------------------------------------
# The entry symbol should be addressed inside main RAM. Find it via -t.
MAIN_SYM=$("$OBJDUMP" -t "$ELF" | awk '/ main$/ {print $1; exit}' || true)
if [[ -n "$MAIN_SYM" ]]; then
    blue "    main() at 0x${MAIN_SYM}"
fi

# --- 6. Ratio check ------------------------------------------------------
# A working Wii browser .dol should be at least 500KB (we link libcurl,
# mbedtls, libogc, freetype etc). If it's <100KB something is very wrong.
if (( DOL_SIZE < 100 * 1024 )); then
    red "[FAIL] .dol is implausibly small ($DOL_SIZE bytes)"
    exit 1
fi
green "[OK] .dol size plausible: $(numfmt --to=iec $DOL_SIZE)"

blue ""
blue "=== all checks passed ==="
