#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM
cc -Wno-unused-function -fsanitize=address,undefined tests/repository/records.c -o "$test_tmp/records"
"$test_tmp/records"
python3 tests/repository/catalog.py
python3 tests/repository/publish.py
python3 tests/repository/mirror.py
python3 tests/repository/http-server.py
python3 tests/repository/helper-recovery.py
cc -Wno-multichar -fsanitize=address,undefined tests/commands/update-host.c -o "$test_tmp/install"
"$test_tmp/install"
for name in transaction plan extract confirmation; do
 cc -Wno-multichar -fsanitize=address,undefined "tests/update/$name.c" -o "$test_tmp/$name"
 "$test_tmp/$name"
done
python3 tests/commands/imports.py
