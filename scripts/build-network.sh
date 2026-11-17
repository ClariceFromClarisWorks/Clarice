#!/bin/sh
# Isolated networking milestone: never writes the accepted distribution/build.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
mkdir -p "$ROOT/work/network"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
for s in network/native vendor/mactcp/nettcp ipconfig ping wget vendor/picohttpparser/picohttpparser; do
 extra=""; if [ "$s" = vendor/picohttpparser/picohttpparser ]; then extra=-DNDEBUG; fi
 m68k-apple-macos-gcc $extra -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror=implicit-function-declaration -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/$s.c -o /src/work/network/$(basename "$s").o
done
for command in ipconfig ping wget; do
 m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/network/$command.o /src/work/network/native.o /src/work/network/nettcp.o /src/work/network/picohttpparser.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/network/$command-linked.o
done
'
for command in ipconfig ping wget; do
 python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/network/$command-linked.o" "$ROOT/work/network/$command.clx" --runtime 1.1
done
