#!/bin/sh
# gen_version.sh — inject version/date into HBC/meta.xml template
# Usage: scripts/gen_version.sh [version] [date] [input] [output]
# Defaults: version from git describe or short HEAD, date UTC YYYYMMDD

set -e

VERSION="${1:-$(git describe --tags --always 2>/dev/null || git rev-parse --short HEAD 2>/dev/null || echo "dev")}"
DATE="${2:-$(date -u +%Y%m%d)}"
INPUT="${3:-HBC/meta.xml.in}"
OUTPUT="${4:-HBC/meta.xml}"

if [ ! -f "$INPUT" ]; then
  INPUT="HBC/meta.xml"
fi

echo "Generating $OUTPUT from $INPUT (version=$VERSION date=$DATE)"
sed -e "s|@VERSION@|${VERSION}|g" \
    -e "s|@RELEASE_DATE@|${DATE}|g" \
    -e "s|<version></version>|<version>${VERSION}</version>|g" \
    -e "s|<release_date></release_date>|<release_date>${DATE}</release_date>|g" \
    "$INPUT" > "$OUTPUT.tmp"
mv "$OUTPUT.tmp" "$OUTPUT"
echo "OK: $OUTPUT"
