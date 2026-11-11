#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cd /src
flags="-m68000 -Os -ffunction-sections -fdata-sections -DHAVE_CONFIG_H -I/src/work/nano/config -I/src/work/nano/regex-include -I/src/src/ports/terminal -I/src/work/ncurses/build/include -idirafter /src/work/nano/cross/nano-9.2/lib"
for source in work/nano/cross/nano-9.2/src/*.c; do m68k-apple-macos-gcc $flags -include /src/src/ports/nano/nano-compat.h -Dmain=nano_main -c "$source" -o work/nano/objects/$(basename "$source" .c).o; done
for source in regex getopt getopt1 strcasestr malloc/dynarray_resize; do m68k-apple-macos-gcc $flags -D_REGEX_AVOID_UCHAR_H -include /src/work/nano/config/config.h -include /src/src/ports/terminal/ncurses-compat.h -c work/nano/cross/nano-9.2/lib/$source.c -o work/nano/objects/$(basename "$source").o; done
m68k-apple-macos-gcc $flags -c src/ports/nano/platform.c -o work/nano/objects/platform.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -I/src/src/ports/terminal -c src/ports/terminal/platform.c -o work/nano/objects/terminal.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -c src/ports/runtime.c -o work/nano/objects/runtime.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/nano/objects/*.o work/ncurses/build/lib/libncurses.a $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/nano/nano-linked.o
m68k-apple-macos-nm -u work/nano/nano-linked.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/nano/nano-linked.o" "$ROOT/work/nano/nano.clx"
