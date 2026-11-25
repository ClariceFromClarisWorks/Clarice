#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/userland-perf
cc -std=c99 -D_POSIX_C_SOURCE=200809L -g -fsanitize=address,undefined -Isrc/vendor/mrsh/include tests/userland-perf/cursor.c src/vendor/mrsh/ast.c src/vendor/mrsh/array.c src/vendor/mrsh/buffer.c src/vendor/mrsh/arithm.c src/vendor/mrsh/parser/*.c -o work/userland-perf/cursor
work/userland-perf/cursor
