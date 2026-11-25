#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM
for name in locations native maintenance; do
 cc -DuserCanceledErr=-128 -Wno-multichar -Wno-return-type -fsanitize=address,undefined -Itests/recovery/include "tests/installer-b/$name.c" -o "$test_tmp/$name"
 "$test_tmp/$name"
done
sh tests/installer-a/run-host.sh
sh tests/recovery/run-host.sh
