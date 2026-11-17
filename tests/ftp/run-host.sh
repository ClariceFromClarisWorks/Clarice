#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/ftp
cc -std=c11 -Wno-multichar -fsanitize=address,undefined tests/ftp/host.c -o work/ftp/host
work/ftp/host
