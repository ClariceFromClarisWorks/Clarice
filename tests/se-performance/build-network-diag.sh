#!/bin/sh
# Separate diagnostic service; does not replace/publish the standard telnetd.
set -eu
cd "$(dirname "$0")/../.."
case "${1:-pending}" in
 pending) mode=0;;
 fixed-peer-port) mode=1;;
 *) echo 'usage: build-network-diag.sh [pending|fixed-peer-port]' >&2; exit 2;;
esac
mkdir -p work/se-performance/http work/se-performance/objects
docker run --rm --pull never --platform linux/amd64 --network none -e SE_FIXED="$mode" -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
extra=""; name=telnetdiag
if test "$SE_FIXED" = 1; then extra=-DSE_NETWORK_FIXED_PEER_PORT; name=telnetfixed; fi
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -DSE_NETWORK_DIAGNOSTICS $extra -Werror=implicit-function-declaration -Wno-multichar -c src/commands/remote/telnetd.c -o work/se-performance/objects/telnetd.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -c src/commands/network/telnet-engine.c -o work/se-performance/objects/engine.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/se-performance/objects/telnetd.o work/se-performance/objects/engine.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/se-performance/objects/linked.o
python3 scripts/pack-command.py work/se-performance/objects/linked.o work/se-performance/http/$name.clx --runtime 1.9
'
