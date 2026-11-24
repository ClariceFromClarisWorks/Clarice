#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
python3 tests/review-a/regress.py
t=$(mktemp -d)
trap 'rm -rf "$t"' EXIT HUP INT TERM
cc -Wno-multichar -Wno-return-type -fsanitize=address,undefined -Itests/recovery/include tests/review-a/first-helper.c -o "$t/first-helper"
"$t/first-helper"
