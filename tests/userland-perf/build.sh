#!/bin/sh
# Disposable CLX diagnostics only; does not rebuild or alter the release.
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/userland-perf
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" "$IMAGE" sh -ec '
cd /src
for name in pressure entry; do m68k-apple-macos-gcc -m68000 -Os -Wno-multichar -c tests/userland-perf/$name.c -o work/userland-perf/$name.o; done'
for name in pressure entry; do python3 scripts/pack-command.py work/userland-perf/$name.o work/userland-perf/$name.clx --runtime 1.11; done
python3 - <<'PY'
from pathlib import Path
for n in (8192,32768):
 Path('work/userland-perf/'+str(n)+'.sh').write_text((': #'+(' '*60)+'\n')*(n//64))
PY
