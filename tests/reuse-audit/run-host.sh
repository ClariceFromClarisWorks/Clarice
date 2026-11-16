#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/reuse-audit
cc -std=c99 -fsanitize=address,undefined -Dfnmatch=upstream_fnmatch -Isrc/vendor/musl-fnmatch -Isrc/ports/fnmatch-include -c src/vendor/musl-fnmatch/fnmatch.c -o work/reuse-audit/reference.o
cc -std=c99 -fsanitize=address,undefined -Isrc/vendor/musl-fnmatch -Isrc/ports/fnmatch-include src/ports/fnmatch-byte.c tests/reuse-audit/fnmatch.c work/reuse-audit/reference.o -o work/reuse-audit/fnmatch-test
work/reuse-audit/fnmatch-test
python3 tests/performance-050/traversal.py
