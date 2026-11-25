#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/network-4c
python3 tests/network-4c/input-host.py
sh tests/remote-sessions/run-host.sh
