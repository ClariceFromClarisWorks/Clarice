#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/tests/release/packaging.py"
for name in collection manifest journal; do
 cc -std=c99 -Wall -Wextra -Wno-unused-function -fsanitize=address,undefined "$ROOT/tests/release/$name.c" -o "$TMP/$name"
 "$TMP/$name"
done
