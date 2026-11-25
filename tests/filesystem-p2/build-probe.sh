#!/bin/sh
set -eu
mkdir -p work/filesystem-p2
image=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" "$image" sh -ec '
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Wno-multichar -Wno-misleading-indentation -c /src/tests/filesystem-p2/probe.c -o /src/work/filesystem-p2/probe-main.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/filesystem-p2/probe-main.o "$(m68k-apple-macos-gcc -print-libgcc-file-name)" -o /src/work/filesystem-p2/probe.o
'
python3 scripts/pack-command.py work/filesystem-p2/probe.o work/filesystem-p2/fsprobe.clx --runtime 1.10
