#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
mkdir -p "$ROOT/work/build"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
flags="-m68000 -Os -DNODEBUG -DNO_LP -DNO_AT -DNO_SSW -DNO_SCAN -DFN_MAX=64 -I/src/src/ports/ekermit/upstream"
m68k-apple-macos-gcc $flags -c /src/src/ports/ekermit/kermit.c -o /src/work/build/kermit-engine.o
m68k-apple-macos-gcc $flags -Wall -Wextra -Wno-multichar -Wno-unused-function -c /src/src/commands/kermit.c -o /src/work/build/kermit-main.o
libgcc=$(m68k-apple-macos-gcc -print-libgcc-file-name)
m68k-apple-macos-ld -r /src/work/build/kermit-main.o /src/work/build/kermit-engine.o "$libgcc" -o /src/work/build/kermit.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/kermit.o" "$ROOT/work/build/kermit.clx" --runtime 1.9
