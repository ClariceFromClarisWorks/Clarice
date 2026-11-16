#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/http-upstream
cc -std=c11 -Wno-multichar -fsanitize=address,undefined tests/http/host.c src/commands/vendor/picohttpparser/picohttpparser.c -o work/http-upstream/host
work/http-upstream/host
cc -std=c11 -fsanitize=address,undefined tests/http/progress.c -o work/http-upstream/progress
work/http-upstream/progress
