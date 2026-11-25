#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/network-4b/server
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" -w /src ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -c tests/network-4b/tcp-guest.c -o work/network-4b/tcp-probe.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main work/network-4b/tcp-probe.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/network-4b/tcp-linked.o
'
python3 scripts/pack-command.py work/network-4b/tcp-linked.o work/network-4b/server/tcp-probe.clx --runtime 1.1
