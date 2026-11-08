#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p work outputs/update1
cc -std=c99 -g -fsanitize=address,undefined -Wno-multichar -I src/vendor/mrsh/include tests/shell/host.c src/vendor/mrsh/ast.c src/vendor/mrsh/array.c src/vendor/mrsh/buffer.c src/vendor/mrsh/arithm.c src/vendor/mrsh/parser/parser.c src/vendor/mrsh/parser/program.c src/vendor/mrsh/parser/word.c src/vendor/mrsh/parser/arithm.c -o work/sh-host
python3 tests/shell/compare.py
