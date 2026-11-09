#!/bin/sh
set -eu
mkdir -p work/awk-host
python3 scripts/prepare-awk.py
flags='-g -O1 -fsanitize=address,undefined -DYYINITDEPTH=64 -DYYMAXDEPTH=128 -Isrc/ports -include src/ports/awk-compat.h'
for name in main b parse tran lib run lex math proctab awkgram.tab; do
 cc $flags -Dmain=upstream_main -c work/build/awk/$name.c -o work/awk-host/$name.o
done
cc -g -O1 -fsanitize=address,undefined -DPORT_AWK -c src/ports/runtime.c -o work/awk-host/runtime.o
cc -g -O1 -fsanitize=address,undefined -c src/ports/awk-runtime.c -o work/awk-host/awk-runtime.o
cc -g -O1 -fsanitize=address,undefined tests/ports/host.c src/ports/integer-format.c work/awk-host/*.o -lm -o work/awk-host/awk
cc -O2 -Isrc/vendor/awk src/vendor/awk/main.c src/vendor/awk/b.c src/vendor/awk/parse.c src/vendor/awk/tran.c src/vendor/awk/lib.c src/vendor/awk/run.c src/vendor/awk/lex.c src/vendor/awk/math.c src/vendor/awk/proctab.c src/vendor/awk/awkgram.tab.c -lm -o work/awk-host/reference-awk
