#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
m68k-apple-macos-gcc -std=gnu99 -m68000 -Os -funsigned-char -ffunction-sections -fdata-sections -I/src/src/ports -c tests/busybox-additions/forksum.c -o work/busybox-additions/forksum.o
m68k-apple-macos-gcc -m68000 -nostdlib -r -Wl,-d,--gc-sections,-u,clarice_main work/busybox-additions/forksum.o work/busybox-additions/objects/runtime.o -lgcc -o work/busybox-additions/forksum-linked.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/busybox-additions/forksum-linked.o.gdb" "$ROOT/work/busybox-additions/forksum.clx" --runtime 1.10
