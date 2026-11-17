#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"
mkdir -p work/deploy-full
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
for s in tests/release/native-stage-fixture src/update/vendor/microtar/microtar; do
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Werror=implicit-function-declaration -c $s.c -o /src/work/deploy-full/$(basename $s).o
done
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/deploy-full/native-stage-fixture.o work/deploy-full/microtar.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/deploy-full/linked.o
'
python3 scripts/pack-command.py work/deploy-full/linked.o work/deploy-full/release-stage --runtime 1.2
