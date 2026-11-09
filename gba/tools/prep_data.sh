#!/usr/bin/env bash
# Prepare DAT files for the GBA build.
#
# Each upstream sprite DAT is funneled through `preproc_dat` (built on
# demand from preproc_dat.c) before being written out as gba/data/<stem>.bin.
# preproc_dat detects image_data_type resources inside the DAT and rewrites
# them with the compression and bitpack already unrolled, so the runtime can
# blit them straight from ROM without per-blit decode scratch.
#
# bin2s (devkitPro tool) names symbols after the *file*: a file named
# kid.bin becomes `kid_bin` / `kid_bin_end` / `kid_bin_size`.
#
# Usage:
#   ./tools/prep_data.sh
# Run from inside the gba/ directory.

set -euo pipefail

cd "$(dirname "$0")/.."

DST="data"
TOOL_SRC="tools/preproc_dat.c"
TOOL_BIN="tools/preproc_dat"
mkdir -p "$DST"

# Build the host tool if missing or out of date.
if [ ! -x "$TOOL_BIN" ] || [ "$TOOL_SRC" -nt "$TOOL_BIN" ]; then
    echo "Building host tool: $TOOL_BIN"
    cc -O2 -Wall -Wno-pointer-sign -o "$TOOL_BIN" "$TOOL_SRC"
fi

# Resolve a source DAT (case-insensitive .DAT / .dat). Checks gba/data/
# first, then ../data/.  Echoes the path or returns failure.
find_src() {
    local stem="$1"
    for d in "$DST" "../data"; do
        for ext in "DAT" "dat"; do
            if [ -f "$d/$stem.$ext" ]; then
                echo "$d/$stem.$ext"
                return 0
            fi
        done
        if [ -f "$d/$stem" ] && [ "$d" != "$DST" ]; then
            echo "$d/$stem"
            return 0
        fi
    done
    return 1
}

# Process one DAT through preproc_dat into the target .bin.
process_dat() {
    local stem="$1"        # upstream stem, e.g. KID
    local outname="$2"     # lower-case symbol-safe stem, e.g. kid
    local src
    if ! src="$(find_src "$stem")"; then
        echo "  warning: no DAT for $stem" >&2
        return 0
    fi
    "$TOOL_BIN" "$src" "$DST/$outname.bin"
}

# Sound DATs / LEVELS contain no image_data_type records and don't need
# preprocessing; copy them across as-is.
copy_dat() {
    local stem="$1"
    local outname="$2"
    local src
    if ! src="$(find_src "$stem")"; then
        echo "  warning: no DAT for $stem" >&2
        return 0
    fi
    cp "$src" "$DST/$outname.bin"
    echo "  $src -> $DST/$outname.bin (passthrough)"
}

echo "Preprocessing sprite DATs (decompress + 8bpp expand) ..."
process_dat "KID"      "kid"
process_dat "PRINCE"   "prince"
process_dat "TITLE"    "title"
process_dat "GUARD"    "guard"
process_dat "GUARD1"   "guard1"
process_dat "GUARD2"   "guard2"
process_dat "VPALACE"  "vpalace"
process_dat "VDUNGEON" "vdungeon"
process_dat "VIZIER"   "vizier"
process_dat "FAT"      "fat"
process_dat "SKEL"     "skel"
process_dat "SHADOW"   "shadow"
process_dat "PV"       "pv"

echo "Pre-converting digi sound samples (unsigned -> signed) ..."
process_dat "DIGISND1" "digisnd1"
process_dat "DIGISND2" "digisnd2"
process_dat "DIGISND3" "digisnd3"

echo "Copying remaining DATs unchanged ..."
copy_dat "LEVELS" "levels"
for s in IBM_SND1 IBM_SND2 MIDISND1 MIDISND2; do
    out="$(echo "$s" | tr '[:upper:]' '[:lower:]')"
    copy_dat "$s" "$out"
done

# Font file (rarely present in DAT form). When absent the runtime falls
# back to hc_font_data baked into the ROM.
for cand in "$DST/font" "$DST/font.dat" "../data/font" "../data/font.dat"; do
    if [ -f "$cand" ]; then
        "$TOOL_BIN" "$cand" "$DST/font.bin" 2>/dev/null || cp "$cand" "$DST/font.bin"
        break
    fi
done

# Pre-decompress the built-in font so the GBA runtime never runs the per-blit
# decoder on glyphs. predecomp_font expands every glyph to depth=8/cmeth=0;
# bin2c bakes the result into src/hc_font_decoded.c (used by load_font()).
echo "Pre-decompressing built-in font (predecomp_font + bin2c) ..."
FONT_TOOL="tools/predecomp_font"
if [ ! -x "$FONT_TOOL" ] || [ "tools/predecomp_font.c" -nt "$FONT_TOOL" ]; then
    cc -O2 -w -o "$FONT_TOOL" tools/predecomp_font.c
fi
if [ ! -x tools/bin2c ] || [ "bin2c.c" -nt tools/bin2c ]; then
    cc -O2 -w -o tools/bin2c bin2c.c
fi
# Extract the hc_font_data[] byte array from the upstream source.
perl -0777 -ne 'if(/hc_font_data\[\]\s*=\s*\{(.*?)\}\s*;/s){my $b=$1; print chr(hex($1)) while $b=~/0x([0-9A-Fa-f]+)/g}' \
    ../src/seg009.c > /tmp/hcfont_raw.bin
if [ -s /tmp/hcfont_raw.bin ]; then
    "$FONT_TOOL" /tmp/hcfont_raw.bin /tmp/hcfont_dec.bin
    tools/bin2c /tmp/hcfont_dec.bin src/hc_font_decoded.c hc_font_decoded
    echo "  -> src/hc_font_decoded.c"
else
    echo "  warning: could not extract hc_font_data from ../src/seg009.c" >&2
fi

echo "Done."
du -sh "$DST" 2>/dev/null || true
