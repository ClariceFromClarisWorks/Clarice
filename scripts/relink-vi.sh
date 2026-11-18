#!/bin/sh
# Incremental development helper: rebuild explicitly named upstream translation units.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" -e VI_FILES="$*" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
for f in $VI_FILES; do m68k-apple-macos-gcc -std=gnu99 -m68000 -Os -flto -fcommon -ffunction-sections -fdata-sections -DCLARICE -DVMUNIX -DBIT8 -DISO8859_1 -DUCVISUAL -I/src/src/ports/vi -I/src/src/ports/terminal -I/src/work/vi/cross -include /src/src/ports/vi/compat.h -c work/vi/cross/$f.c -o work/vi/objects/$(basename $f).o; done
'
"$ROOT/scripts/build-vi.sh" --relink
