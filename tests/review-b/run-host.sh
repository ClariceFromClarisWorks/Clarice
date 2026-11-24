#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
python3 tests/review-b/repository.py
python3 tests/review-b/geometry.py
python3 tests/geometry/host.py
sh tests/recovery/run-host.sh
