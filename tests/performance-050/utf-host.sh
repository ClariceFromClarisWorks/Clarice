#!/bin/sh
set -eu
python3 scripts/prepare-utf.py
mkdir -p work/performance-050/utf-host
flags='-g -O1 -fsanitize=address,undefined -Isrc/vendor/sbase'
for f in fgetrune fputrune isspacerune; do
 cc $flags -Dfgetrune=fast_fgetrune -Defgetrune=fast_efgetrune -Dfputrune=fast_fputrune -Defputrune=fast_efputrune -Disspacerune=fast_isspacerune -c work/build/utf/$f.c -o work/performance-050/utf-host/$f.o
done
cc $flags tests/performance-050/utf.c src/vendor/sbase/libutf/*.c work/performance-050/utf-host/*.o -o work/performance-050/utf-test
work/performance-050/utf-test
