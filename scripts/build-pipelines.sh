#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
python3 "$ROOT/scripts/prepare-pipelines.py"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cd /src
flags="-m68000 -Os -fno-common -ffunction-sections -fdata-sections -Isrc/ports/include -Isrc/vendor/sbase -Isrc/ports -include src/ports/compat.h"
objects=""
libgcc=$(m68k-apple-macos-gcc -print-libgcc-file-name)
for file in src/vendor/sbase/libutf/*.c src/vendor/sbase/libutil/ealloc.c work/build/ports/eprintf.c src/vendor/sbase/libutil/fshut.c src/vendor/sbase/libutil/reallocarray.c src/vendor/sbase/libutil/strtonum.c src/vendor/sbase/libutil/unescape.c src/vendor/sbase/libutil/memmem.c src/vendor/sbase/libutil/linecmp.c src/vendor/sbase/libutil/getlines.c src/vendor/sbase/libutil/concat.c src/vendor/sbase/libutil/writeall.c src/ports/path-names.c; do
 name=$(basename "$file" .c)
 m68k-apple-macos-gcc $flags -c "$file" -o work/build/pipelines/$name.o
 objects="$objects work/build/pipelines/$name.o"
done
for name in wc head cut tr uniq tee tail sort basename dirname; do
 m68k-apple-macos-gcc $flags -Dmain=upstream_main -c work/build/pipelines/$name.c -o work/build/pipelines/$name.o
 m68k-apple-macos-ld -r --gc-sections -u clarice_main work/build/pipelines/$name.o $objects work/build/ports/runtime.o "$libgcc" -o work/build/$name.o
done
'
for name in wc head cut tr uniq tee tail sort basename dirname; do
 python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/$name.o" "$ROOT/work/build/$name.clx"
done
