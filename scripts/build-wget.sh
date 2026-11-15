#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
mkdir -p "$ROOT/work/wget"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
for source in wget vendor/mactcp/nettcp; do
 name=$(basename "$source")
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror=implicit-function-declaration -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/$source.c -o /src/work/wget/$name.o
done
m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/wget/wget.o /src/work/wget/nettcp.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/wget/command.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/wget/command.o" "$ROOT/work/wget/wget.clx" --runtime 1.1

mkdir -p "$ROOT/work/build"
cp "$ROOT/work/wget/wget.clx" "$ROOT/work/build/wget.clx"
