#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
mkdir -p "$ROOT/work/build/ports"
python3 "$ROOT/scripts/prepare-ports.py"
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cd /src
flags="-Dhidden= -DCHARCLASS_NAME_MAX=14 -DRE_DUP_MAX=255 -m68000 -Os -fno-common -ffunction-sections -fdata-sections -I/src/src/ports/include -I/src/src/vendor/sbase -include /src/src/ports/compat.h"
for file in work/build/ports/grep.c src/vendor/sbase/libutil/ealloc.c work/build/ports/eprintf.c src/vendor/sbase/libutil/eregcomp.c src/vendor/sbase/libutil/fshut.c src/vendor/sbase/libutil/strcasestr.c src/vendor/sbase/libutil/reallocarray.c src/vendor/sbase/libutf/rune.c src/vendor/sbase/libutf/utf.c src/vendor/sbase/libutf/isspacerune.c src/vendor/sbase/libutf/isdigitrune.c src/vendor/sbase/libutf/runetype.c src/vendor/sbase/libutil/strlcat.c work/build/ports/regcomp.c work/build/ports/regexec.c src/vendor/musl-regex/tre-mem.c src/vendor/musl-regex/regerror.c; do
 name=$(basename "$file" .c)
 m68k-apple-macos-gcc $flags -I/src/src/vendor/musl-regex -Dmain=upstream_main -c "$file" -o work/build/ports/$name.o
done
m68k-apple-macos-gcc -m68000 -Os -I/src/src/ports/include -c src/ports/runtime.c -o work/build/ports/runtime.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/build/ports/grep.o work/build/ports/ealloc.o work/build/ports/eprintf.o work/build/ports/eregcomp.o work/build/ports/fshut.o work/build/ports/strcasestr.o work/build/ports/regcomp.o work/build/ports/regexec.o work/build/ports/tre-mem.o work/build/ports/regerror.o work/build/ports/runtime.o -o work/build/grep.o
m68k-apple-macos-gcc $flags -Dmain=upstream_main -Dregexec=port_regexec -c work/build/ports/sed.c -o work/build/ports/sed.o
m68k-apple-macos-gcc $flags -c src/ports/regex-wrapper.c -o work/build/ports/regex-wrapper.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/build/ports/sed.o work/build/ports/isspacerune.o work/build/ports/isdigitrune.o work/build/ports/runetype.o work/build/ports/strlcat.o work/build/ports/ealloc.o work/build/ports/eprintf.o work/build/ports/eregcomp.o work/build/ports/fshut.o work/build/ports/reallocarray.o work/build/ports/rune.o work/build/ports/utf.o work/build/ports/regcomp.o work/build/ports/regexec.o work/build/ports/tre-mem.o work/build/ports/regerror.o work/build/ports/regex-wrapper.o work/build/ports/runtime.o -o work/build/sed.o
m68k-apple-macos-nm -u work/build/sed.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/grep.o" "$ROOT/work/build/grep.clx"

python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/sed.o" "$ROOT/work/build/sed.clx"
