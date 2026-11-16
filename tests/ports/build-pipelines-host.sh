#!/bin/sh
set -eu
mkdir -p work/pipelines-host/helpers work/build/ports
python3 scripts/prepare-ports.py
python3 scripts/prepare-pipelines.py
flags='-g -O1 -fsanitize=address,undefined -Isrc/vendor/sbase -Isrc/ports -include src/ports/compat.h'
helpers='src/vendor/sbase/libutil/ealloc.c work/build/ports/eprintf.c src/vendor/sbase/libutil/fshut.c src/vendor/sbase/libutil/reallocarray.c src/vendor/sbase/libutil/strtonum.c src/vendor/sbase/libutil/unescape.c src/vendor/sbase/libutil/memmem.c src/vendor/sbase/libutil/linecmp.c src/vendor/sbase/libutil/getlines.c src/vendor/sbase/libutil/concat.c src/vendor/sbase/libutil/writeall.c src/ports/path-names.c src/ports/awk-runtime.c src/ports/temp-name.c src/vendor/sbase/libutil/strlcpy.c src/vendor/sbase/libutil/strlcat.c'
for file in src/vendor/sbase/libutf/*.c $helpers; do
 name=$(basename "$file" .c)
 cc $flags -c "$file" -o work/pipelines-host/helpers/$name.o
done
cc -g -O1 -fsanitize=address,undefined -DCLARICE_DD_RUNTIME -c src/ports/runtime.c -o work/pipelines-host/helpers/runtime.o
reference_helpers=$(printf '%s' "$helpers" | sed 's@work/build/ports/eprintf.c@src/vendor/sbase/libutil/eprintf.c@g; s@src/ports/awk-runtime.c@@g; s@src/ports/temp-name.c@@g')
for name in ${CLARICE_TEST_COMMANDS:-wc head cut tr uniq tee tail sort basename dirname cmp cksum sleep printenv mktemp dd}; do
 cc $flags -Dmain=upstream_main -c work/build/pipelines/$name.c -o work/pipelines-host/$name.o
 cc -g -O1 -fsanitize=address,undefined tests/ports/host.c work/pipelines-host/$name.o work/pipelines-host/helpers/*.o -o work/pipelines-host/$name
 cc -O2 -Isrc/vendor/sbase -Isrc/ports src/vendor/sbase/$name.c src/vendor/sbase/libutf/*.c $reference_helpers -o work/pipelines-host/reference-$name
done
