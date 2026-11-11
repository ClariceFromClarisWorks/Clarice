#!/bin/sh
# Experimental integration probe; not installed or added to release resources.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cd /src
flags="-m68000 -Os -ffunction-sections -fdata-sections -I/src/src/ports/terminal -I/src/work/ncurses/build/include -include /src/src/ports/terminal/ncurses-compat.h"
m68k-apple-macos-gcc $flags -c src/ports/terminal/platform.c -o work/ncurses/platform.o
m68k-apple-macos-gcc $flags -c tests/ports/ncurses-demo.c -o work/ncurses/demo.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -I/src/src/ports/include -c src/ports/runtime.c -o work/ncurses/runtime.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/ncurses/demo.o work/ncurses/platform.o work/ncurses/runtime.o work/ncurses/build/lib/libncurses.a $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/ncurses/demo-linked.o
m68k-apple-macos-nm -u work/ncurses/demo-linked.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/ncurses/demo-linked.o" "$ROOT/work/ncurses/cursesdemo.clx"
