#!/bin/sh
# Isolated networking milestone: never writes the accepted distribution/build.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
mkdir -p "$ROOT/work/telnet"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
for s in network/native network/telnet-engine vendor/mactcp/nettcp telnet; do
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror=implicit-function-declaration -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/$s.c -o /src/work/telnet/$(basename "$s").o
done
for command in telnet; do
 m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/telnet/$command.o /src/work/telnet/native.o /src/work/telnet/telnet-engine.o /src/work/telnet/nettcp.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/telnet/$command-linked.o
done
'
for command in telnet; do
 python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/telnet/$command-linked.o" "$ROOT/work/telnet/$command.clx" --runtime 1.1
done
