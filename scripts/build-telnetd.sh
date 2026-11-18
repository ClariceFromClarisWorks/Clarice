#!/bin/sh
# Optional service: kept outside the standard collection until acceptance.
set -eu
cd "$(dirname "$0")/.."
mkdir -p work/remote-sessions
# A failed build must not leave a publishable image from an earlier invocation.
rm -f work/remote-sessions/telnetd.clx work/remote-sessions/seriald.clx
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
for source in remote/telnetd remote/seriald network/telnet-engine; do
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror=implicit-function-declaration -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/$source.c -o /src/work/remote-sessions/$(basename $source).o
done
m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/remote-sessions/telnetd.o /src/work/remote-sessions/telnet-engine.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/remote-sessions/telnetd-linked.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/remote-sessions/seriald.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/remote-sessions/seriald-linked.o
'
python3 scripts/pack-command.py work/remote-sessions/telnetd-linked.o work/remote-sessions/telnetd.clx --runtime 1.9
python3 scripts/pack-command.py work/remote-sessions/seriald-linked.o work/remote-sessions/seriald.clx --runtime 1.9
