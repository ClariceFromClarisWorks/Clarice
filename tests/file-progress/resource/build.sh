#!/bin/sh
set -eu
python3 tests/file-progress/resource/fixture.py
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
Rez work/file-progress/resource/fixture.r --data work/file-progress/resource/data -t TEST -c FPts -o work/file-progress/resource/valid.bin
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation -c tests/file-progress/resource/probe.c -o work/file-progress/resource/probe.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/file-progress/resource/probe.o "$(m68k-apple-macos-gcc -print-libgcc-file-name)" -o work/file-progress/resource/probe-package.o
'
python3 scripts/pack-command.py work/file-progress/resource/probe-package.o work/file-progress/resource/rescheck --runtime 1.10
