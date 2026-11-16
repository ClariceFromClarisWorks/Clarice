#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/parser-safety
cc -std=c99 -D_POSIX_C_SOURCE=200809L -g -fsanitize=address,undefined -Itests/parser -c tests/parser/fault.c -o work/parser-safety/fault.o
cc -std=c99 -D_POSIX_C_SOURCE=200809L -g -fsanitize=address,undefined -Isrc/vendor/mrsh/include -Itests/parser -include tests/parser/fault.h tests/parser/parse.c work/parser-safety/fault.o src/vendor/mrsh/ast.c src/vendor/mrsh/array.c src/vendor/mrsh/buffer.c src/vendor/mrsh/arithm.c src/vendor/mrsh/parser/parser.c src/vendor/mrsh/parser/program.c src/vendor/mrsh/parser/word.c src/vendor/mrsh/parser/arithm.c -o work/parser-safety/parse
