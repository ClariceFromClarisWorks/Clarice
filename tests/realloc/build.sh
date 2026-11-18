#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
mkdir -p "$ROOT/work/realloc"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
m68k-apple-macos-gcc -std=gnu99 -m68000 -Os -funsigned-char -ffunction-sections -fdata-sections -DPORT_ALLOC_TRACE -Isrc/ports/include -c src/ports/runtime.c -o work/realloc/runtime-trace.o
objects=""
for f in work/busybox-additions/objects/*.o; do case "$f" in */runtime.o) ;; *) objects="$objects $f";; esac; done
m68k-apple-macos-gcc -m68000 -nostdlib -r -Wl,-d,--gc-sections,-u,clarice_main work/busybox-additions/diff.o work/busybox-additions/entry-diff.o work/realloc/runtime-trace.o $objects -lgcc -o work/realloc/difftrace.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/realloc/difftrace.o.gdb" "$ROOT/work/realloc/difftrace.clx" --runtime 1.10
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
flags="-std=gnu99 -m68000 -Os -funsigned-char -ffunction-sections -fdata-sections"
m68k-apple-macos-gcc $flags -DPORT_ALLOC_TRACE -DPORT_NATIVE_RESIZE -Isrc/ports/include -c src/ports/runtime.c -o work/realloc/runtime-grow.o
m68k-apple-macos-gcc $flags -DPORT_ALLOC_TRACE -c src/ports/native-resize.c -o work/realloc/native-resize.o
objects=""
for f in work/busybox-additions/objects/*.o; do case "$f" in */runtime.o) ;; *) objects="$objects $f";; esac; done
m68k-apple-macos-gcc -m68000 -nostdlib -r -Wl,-d,--gc-sections,-u,clarice_main work/busybox-additions/diff.o work/busybox-additions/entry-diff.o work/realloc/runtime-grow.o work/realloc/native-resize.o $objects -lgcc -o work/realloc/diffgrow.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/realloc/diffgrow.o.gdb" "$ROOT/work/realloc/diffgrow.clx" --runtime 1.10
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
m68k-apple-macos-gcc -std=gnu99 -m68000 -Os -funsigned-char -ffunction-sections -fdata-sections -Isrc/ports -c tests/realloc/guest.c -o work/realloc/guest.o
m68k-apple-macos-gcc -m68000 -nostdlib -r -Wl,-d,--gc-sections,-u,clarice_main work/realloc/guest.o work/realloc/runtime-grow.o work/realloc/native-resize.o -lgcc -o work/realloc/alloctest.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/realloc/alloctest.o.gdb" "$ROOT/work/realloc/alloctest.clx" --runtime 1.10
