#!/bin/sh
set -eu
mkdir -p work/ports-host work/build/ports
python3 scripts/prepare-ports.py
flags='-g -O1 -fsanitize=address,undefined -Dhidden= -DCHARCLASS_NAME_MAX=14 -DRE_DUP_MAX=255 -Isrc/ports/include -Isrc/vendor/sbase -Isrc/vendor/musl-regex -include src/ports/compat.h'
for file in work/build/ports/grep.c src/vendor/sbase/libutil/ealloc.c work/build/ports/eprintf.c src/vendor/sbase/libutil/eregcomp.c src/vendor/sbase/libutil/fshut.c src/vendor/sbase/libutil/strcasestr.c work/build/ports/regcomp.c work/build/ports/regexec.c src/vendor/musl-regex/tre-mem.c src/vendor/musl-regex/regerror.c; do
 name=$(basename "$file" .c)
 cc $flags -Dmain=upstream_main -c "$file" -o work/ports-host/$name.o
done
cc -g -O1 -fsanitize=address,undefined -c src/ports/runtime.c -o work/ports-host/runtime.o
cc -g -O1 -fsanitize=address,undefined tests/ports/host.c work/ports-host/*.o -o work/ports-host/grep
cc -O2 -Isrc/vendor/sbase src/vendor/sbase/grep.c src/vendor/sbase/libutil/ealloc.c src/vendor/sbase/libutil/eprintf.c src/vendor/sbase/libutil/eregcomp.c src/vendor/sbase/libutil/fshut.c src/vendor/sbase/libutil/strcasestr.c -o work/ports-host/reference-grep
