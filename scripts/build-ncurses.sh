#!/bin/sh
# Real upstream dependency; experimental command is kept out of release media.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
python3 "$ROOT/scripts/prepare-ncurses.py"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cd /src/work/ncurses/build
CC=m68k-apple-macos-gcc AR=m68k-apple-macos-ar RANLIB=m68k-apple-macos-ranlib CFLAGS="-m68000 -Os" ../ncurses-6.5/configure --host=m68k-apple-macos --without-shared --without-debug --without-ada --without-cxx --without-cxx-binding --without-manpages --without-tests --without-progs --disable-widec --disable-database --with-fallbacks=clarice --disable-largefile
'
python3 "$ROOT/scripts/configure-ncurses.py"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cd /src/work/ncurses/build
make -C include
make -C ncurses -j2 CPP="m68k-apple-macos-gcc -E -I/src/src/ports/terminal" CFLAGS="-m68000 -Os -ffunction-sections -fdata-sections -I/src/src/ports/terminal -include /src/src/ports/terminal/ncurses-compat.h"
'
"$ROOT/scripts/link-ncurses-demo.sh"
