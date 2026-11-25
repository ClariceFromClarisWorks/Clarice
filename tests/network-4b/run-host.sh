#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/network-4b
cc -std=c11 -Wno-unknown-escape-sequence -fsanitize=address,undefined -Itests/remote-sessions/include tests/network/tcp-host.c -o work/network-4b/tcp-host
work/network-4b/tcp-host
python3 tests/network-4b/pause-host.py
sh tests/ftp/run-host.sh
sh tests/http/run-host.sh
sh tests/telnet/run-host.sh
sh tests/remote-sessions/run-host.sh
