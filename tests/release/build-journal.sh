#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"
mkdir -p work/release-journal
cat > work/release-journal/native-probe.c <<'PROBE'
#include "../../src/update/release-native.h"
int journal_read_probe(ReleaseTransaction *t){return release_read_journal(t);}
PROBE
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Werror=implicit-function-declaration -DCLARICE_JOURNAL_GUEST -c tests/release/journal.c -o work/release-journal/journal.o
m68k-apple-macos-gcc -m68000 -Os -Werror=implicit-function-declaration -c work/release-journal/native-probe.c -o work/release-journal/native-probe.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/release-journal/journal.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/release-journal/linked.o
'
python3 scripts/pack-command.py work/release-journal/linked.o work/release-journal/journal-test --runtime 1.2
