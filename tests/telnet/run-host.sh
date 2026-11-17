#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/telnet
cc -std=c11 -fsanitize=address,undefined tests/telnet/host.c src/commands/network/telnet-engine.c -o work/telnet/host
work/telnet/host
cc -std=c11 -fsanitize=address,undefined tests/telnet/vt100.c -o work/telnet/vt100-host
work/telnet/vt100-host
cc -std=c11 -fsanitize=address,undefined -Isrc/commands/vendor/libtelnet \
 src/commands/network/telnet-engine.c tests/telnet/upstream-alloc.c \
 src/commands/vendor/libtelnet/test/telnet-test.c -o work/telnet/upstream-test
for input in src/commands/vendor/libtelnet/test/*.input; do
 work/telnet/upstream-test "$input" "${input%.input}.txt"
 printf "PASS upstream %s\n" "$(basename "$input" .input)"
done
