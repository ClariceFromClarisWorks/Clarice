#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p work/kermit
${CC:-cc} -O2 -DNODEBUG -DNO_LP -DNO_AT -DNO_SSW -DNO_SCAN -DFN_MAX=64 -DKERMIT_HOST -Isrc/ports/ekermit/upstream src/commands/kermit.c src/ports/ekermit/kermit.c -o work/kermit/kermit
printf '%s\n' 'Built work/kermit/kermit (local one-shot serial peer; --help for usage)'
