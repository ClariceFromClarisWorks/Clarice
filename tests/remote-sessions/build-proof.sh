#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/remote-sessions
cc -fsanitize=address,undefined tests/remote-sessions/protocol.c src/commands/network/telnet-engine.c -o work/remote-sessions/protocol-host
work/remote-sessions/protocol-host
image=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" "$image" sh -ec '
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -DREMOTE_GUEST -c /src/tests/remote-sessions/protocol.c -o /src/work/remote-sessions/protocol.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -DNDEBUG -c /src/src/commands/network/telnet-engine.c -o /src/work/remote-sessions/engine.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/remote-sessions/protocol.o /src/work/remote-sessions/engine.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/remote-sessions/command.o
'
python3 scripts/pack-command.py work/remote-sessions/command.o work/remote-sessions/protocol.clx
