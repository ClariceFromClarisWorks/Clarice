#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python3 "$ROOT/scripts/prepare-awk.py"
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cd /src
flags="-m68000 -Os -fno-common -ffunction-sections -fdata-sections -D__assert_func=port_assert -DYYINITDEPTH=64 -DYYMAXDEPTH=128 -I/src/src/ports -include /src/src/ports/awk-compat.h"
for name in main b parse tran lib run lex math proctab awkgram.tab; do
 m68k-apple-macos-gcc $flags -Dmain=upstream_main -c work/build/awk/$name.c -o work/build/awk/$name.o
done
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -DPORT_AWK -c src/ports/runtime.c -o work/build/awk/runtime.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -c src/ports/awk-runtime.c -o work/build/awk/awk-runtime.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -c src/ports/integer-format.c -o work/build/awk/integer-format.o
libm=$(m68k-apple-macos-gcc -print-file-name=libm.a)
libgcc=$(m68k-apple-macos-gcc -print-libgcc-file-name)
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/build/awk/*.o "$libm" "$libgcc" -o work/build/awk.o
m68k-apple-macos-size work/build/awk.o
m68k-apple-macos-nm -u work/build/awk.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/awk.o" "$ROOT/work/build/awk.clx"
