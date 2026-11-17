#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/network
cc -std=c11 -Wno-unknown-escape-sequence -Wno-multichar -fsanitize=address,undefined -Itests/network/include tests/network/host.c -o work/network/host
cc -std=c11 -fsanitize=address,undefined -Itests/network/include tests/network/commands.c -o work/network/commands-host
cc -std=c11 -Wno-multichar -fsanitize=address,undefined tests/http/host.c src/commands/vendor/picohttpparser/picohttpparser.c -o work/network/http-host
work/network/host
work/network/commands-host
work/network/http-host
