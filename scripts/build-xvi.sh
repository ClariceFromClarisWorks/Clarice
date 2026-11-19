#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ "${1:-}" != "--relink" ]; then python3 "$ROOT/scripts/prepare-xvi.py"; fi
rm -f "$ROOT/work/xvi/xvi.clx"
mkdir -p "$ROOT/work/xvi/objects"
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" -e XVI_RELINK="${1:-}" "$IMAGE" sh -ec '
cd /src
flags="-std=gnu99 -m68000 -Os -ffunction-sections -fdata-sections -DCLARICE -DPOSIX -I/src/src/ports/xvi -I/src/src/ports/terminal -I/src/work/xvi/cross"
files="alloc altstack ascii buffers cmdline cmdmode cmdtab cursor dispmode edit ex_cmds1 ex_cmds2 events fileio find flexbuf map mark misccmds mouse movement normal param pipe preserve ptrfunc regexp screen search signal startup status tag targets undo update version vi_cmds vi_ops virtscr windows yankput defscr"
if [ "$XVI_RELINK" != "--relink" ]; then
for f in $files; do m68k-apple-macos-gcc $flags -include /src/src/ports/xvi/compat.h -c work/xvi/cross/$f.c -o work/xvi/objects/$f.o; done
fi
m68k-apple-macos-gcc $flags -c src/ports/xvi/platform.c -o work/xvi/objects/platform.o
m68k-apple-macos-gcc $flags -c src/ports/terminal/platform.c -o work/xvi/objects/terminal.o
m68k-apple-macos-gcc $flags -Dclarice_main=xvi_runtime_main -c src/ports/runtime.c -o work/xvi/objects/runtime.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/xvi/objects/*.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/xvi/xvi-linked.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/xvi/xvi-linked.o" "$ROOT/work/xvi/xvi.clx" --runtime 1.9
