#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM
cc -DuserCanceledErr=-128 -Wno-multichar -Wno-return-type -fsanitize=address,undefined -Itests/recovery/include tests/installer-c/native.c -o "$test_tmp/native"
"$test_tmp/native"
sh tests/installer-a/run-host.sh
