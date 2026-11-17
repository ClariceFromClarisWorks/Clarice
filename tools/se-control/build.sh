#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cmake -S /src/tools/se-control -B /src/work/se-control/build -DCMAKE_TOOLCHAIN_FILE=/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
cmake --build /src/work/se-control/build
'
