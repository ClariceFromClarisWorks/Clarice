#!/bin/sh
# After the normal complete build: disposable probes only, never publication.
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/network-4a/server
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" -w /src ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
m68k-apple-macos-gcc -m68000 -Os -c tests/network-4a/heap-guest.c -o work/network-4a/heap.o
m68k-apple-macos-gcc -m68000 -Os -c tests/serial/cancel-guest.c -o work/network-4a/serial-cancel.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -c tests/network-4a/ftp-error-guest.c -o work/network-4a/ftp-error.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/network-4a/ftp-error.o work/ftp/native.o work/ftp/nettcp.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/network-4a/ftp-error-linked.o
m68k-apple-macos-gcc -m68000 -Os -DNODEBUG -DNO_LP -DNO_AT -DNO_SSW -DNO_SCAN -DFN_MAX=64 -Isrc/ports/ekermit/upstream -c tests/network-4a/kermit-native-guest.c -o work/network-4a/kermit-native.o
m68k-apple-macos-ld -r work/network-4a/kermit-native.o work/build/kermit-engine.o -o work/network-4a/kermit-native-linked.o
'
python3 scripts/pack-command.py work/network-4a/heap.o work/network-4a/server/netheap2.clx
python3 scripts/pack-command.py work/network-4a/serial-cancel.o work/network-4a/server/serial-cancel.clx
python3 scripts/pack-command.py work/network-4a/ftp-error-linked.o work/network-4a/server/ftp-error.clx --runtime 1.1
python3 scripts/pack-command.py work/network-4a/kermit-native-linked.o work/network-4a/server/kermit-native.clx
