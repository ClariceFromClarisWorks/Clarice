#!/bin/sh
set -eu
image=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" "$image" sh -ec '
for name in files inspect; do
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/$name.c -o /src/work/filesystem-perf/after-$name-normal.o
 extra=""; if [ "$name" = inspect ]; then extra=/src/work/build/fnmatch-byte.o; fi
 m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/filesystem-perf/after-$name-normal.o $extra "$(m68k-apple-macos-gcc -print-libgcc-file-name)" -o /src/work/filesystem-perf/after-$name-linked.o
done'
for name in files inspect; do python3 scripts/pack-command.py work/filesystem-perf/after-$name-linked.o work/filesystem-perf/after-$name.clx --runtime 1.10; done
