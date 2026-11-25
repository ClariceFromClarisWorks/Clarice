#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/network-4d
cc -std=c11 -Wno-multichar -fsanitize=address,undefined -DTEST_HTTP tests/network-4d/poll-host.c src/commands/vendor/picohttpparser/picohttpparser.c -o work/network-4d/http-host
work/network-4d/http-host
cc -std=c11 -Wno-multichar -fsanitize=address,undefined -DTEST_FTP tests/network-4d/poll-host.c -o work/network-4d/ftp-host
work/network-4d/ftp-host
cc -std=c11 -fsanitize=address,undefined -DTELNET_HOST_TEST tests/network-4d/poll-host.c src/commands/network/telnet-engine.c -o work/network-4d/telnet-host
work/network-4d/telnet-host
