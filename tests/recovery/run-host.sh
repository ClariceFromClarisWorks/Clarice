#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM
for name in records native; do
 cc -Wno-multichar -fsanitize=address,undefined -Itests/recovery/include "tests/recovery/$name.c" -o "$test_tmp/$name"
 "$test_tmp/$name"
done
