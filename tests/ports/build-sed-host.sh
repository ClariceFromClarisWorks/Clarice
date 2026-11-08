#!/bin/sh
set -eu
mkdir -p work/sed-host work/build/ports
python3 scripts/prepare-ports.py
flags='-g -O1 -fsanitize=address,undefined -Dhidden= -DCHARCLASS_NAME_MAX=14 -DRE_DUP_MAX=255 -Isrc/ports/include -Isrc/vendor/sbase -Isrc/vendor/musl-regex -include src/ports/compat.h'
helpers='src/vendor/sbase/libutil/ealloc.c work/build/ports/eprintf.c src/vendor/sbase/libutil/eregcomp.c src/vendor/sbase/libutil/fshut.c src/vendor/sbase/libutil/reallocarray.c src/vendor/sbase/libutil/strlcat.c src/vendor/sbase/libutf/rune.c src/vendor/sbase/libutf/utf.c src/vendor/sbase/libutf/isspacerune.c src/vendor/sbase/libutf/isdigitrune.c src/vendor/sbase/libutf/runetype.c'
for file in $helpers work/build/ports/regcomp.c work/build/ports/regexec.c src/vendor/musl-regex/tre-mem.c src/vendor/musl-regex/regerror.c src/ports/regex-wrapper.c; do
 name=$(basename "$file" .c)
 cc $flags -c "$file" -o work/sed-host/$name.o
done
cc $flags -Dmain=upstream_main -Dregexec=port_regexec -c work/build/ports/sed.c -o work/sed-host/sed.o
cc -g -O1 -fsanitize=address,undefined -c src/ports/runtime.c -o work/sed-host/runtime.o
cc -g -O1 -fsanitize=address,undefined tests/ports/host.c work/sed-host/*.o -o work/sed-host/sed
reference_helpers=$(printf "%s" "$helpers" | sed "s@work/build/ports/eprintf.c@src/vendor/sbase/libutil/eprintf.c@g")
cc -O2 -Isrc/vendor/sbase src/vendor/sbase/sed.c $reference_helpers -o work/sed-host/reference-sed
