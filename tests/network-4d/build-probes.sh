#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
stage=${1:?baseline or candidate}
case "$stage" in baseline|candidate) ;; *) exit 2;; esac
mkdir -p "work/network-4d/$stage"
docker run --rm --pull never --platform linux/amd64 --network none -e STAGE="$stage" -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
for name in wget ftp telnet; do
 case "$name" in wget) mode=PROFILE_WGET; extra=work/network/picohttpparser.o;; ftp) mode=PROFILE_FTP; extra="";; telnet) mode=PROFILE_TELNET; extra=work/telnet/telnet-engine.o;; esac
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -D$mode -Werror=implicit-function-declaration -Wno-multichar -c tests/network-4d/profile-guest.c -o work/network-4d/$STAGE/$name.o
 m68k-apple-macos-ld -r --gc-sections -u clarice_main work/network-4d/$STAGE/$name.o work/network/native.o $extra $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/network-4d/$STAGE/$name-linked.o
done
'
for name in wget ftp telnet; do python3 scripts/pack-command.py "work/network-4d/$stage/$name-linked.o" "work/network-4d/$stage/$name.clx" --runtime 1.1; done
