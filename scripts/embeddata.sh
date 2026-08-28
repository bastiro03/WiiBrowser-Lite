#!/bin/sh
# embeddata.sh <input-file> <output-object>
#
# Embeds a binary asset into an ELF object defining three symbols:
#   <name>      : start of the data
#   <name>_end  : end of the data
#   <name>_size : absolute symbol = size of the data (end - start)
#
# Replaces the devkitPro 'bin2o'/'bin2s' step, which omits the _size symbol
# that this project references (extern const u32 <name>_size).
set -e

IN="$1"
OUT="$2"

BASE=$(basename "$IN" | tr '.' '_')
TMPDIR=$(dirname "$OUT")
TMP="$TMPDIR/embed_$$.s"

mkdir -p "$TMPDIR"

cat > "$TMP" <<EOF
    .section .rodata
    .align 2
    .global ${BASE}
${BASE}:
    .incbin "${IN}"
    .global ${BASE}_end
${BASE}_end:
    .align 2
    .global ${BASE}_size
${BASE}_size:
    .long ${BASE}_end - ${BASE}
EOF

"${CC:-powerpc-eabi-gcc}" -x assembler-with-cpp -c "$TMP" -o "$OUT"
rm -f "$TMP"
