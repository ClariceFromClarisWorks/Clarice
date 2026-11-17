#!/bin/sh
# Session ownership regression; no networking, guest deployment or hardware access.
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/remote-sessions
cc -std=c99 -g -fsanitize=address,undefined -Wno-multichar \
 -I src/vendor/mrsh/include tests/remote-sessions/session-host.c \
 src/vendor/mrsh/ast.c src/vendor/mrsh/array.c src/vendor/mrsh/buffer.c \
 src/vendor/mrsh/arithm.c src/vendor/mrsh/parser/parser.c \
 src/vendor/mrsh/parser/program.c src/vendor/mrsh/parser/word.c \
 src/vendor/mrsh/parser/arithm.c -o work/remote-sessions/session-host
work/remote-sessions/session-host

cc -g -fsanitize=address,undefined -Wno-unknown-escape-sequence \
 -I tests/remote-sessions/include tests/remote-sessions/service-host.c \
 src/commands/network/telnet-engine.c -o work/remote-sessions/service-host
work/remote-sessions/service-host
