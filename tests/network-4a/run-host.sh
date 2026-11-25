#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/network-4a
cc -std=c11 -fsanitize=address,undefined tests/serial/client-host.c -o work/network-4a/serial-host
work/network-4a/serial-host
python3 tests/kermit/native-lifecycle.py
cc -std=c11 -Wno-unknown-escape-sequence -fsanitize=address,undefined -Itests/remote-sessions/include tests/network/tcp-host.c -o work/network-4a/tcp-host
work/network-4a/tcp-host
sh tests/ftp/run-host.sh
sh tests/telnet/run-host.sh
sh tests/http/run-host.sh
sh tests/network/run-host.sh
sh tests/remote-sessions/run-host.sh
