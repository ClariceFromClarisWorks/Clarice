#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
mkdir -p "$ROOT/outputs/toolchain"
# Use the existing SkiFree image, offline; never replace it with a floating tag.
docker image inspect "$IMAGE" > "$ROOT/outputs/toolchain/image.json"
docker run --rm --pull never --platform linux/amd64 --network none \
  -v "$ROOT:/src" "$IMAGE" /bin/sh -ec '
    cmake -S /src/probes/toolchain -B /src/work/build-probe \
      -DCMAKE_TOOLCHAIN_FILE=/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
    cmake --build /src/work/build-probe -j2
    /Retro68-build/toolchain/bin/m68k-apple-macos-objdump -dr /src/work/build-probe/CMakeFiles/Sys6Probe.dir/abi_probe.S.obj > /src/outputs/toolchain/abi-disassembly.txt
  ' > "$ROOT/outputs/toolchain/build.log" 2>&1 || {
    cat "$ROOT/outputs/toolchain/build.log"
    exit 1
  }
cp "$ROOT/work/build-probe/Sys6Probe.bin" "$ROOT/outputs/toolchain/"
cp "$ROOT/work/build-probe/Sys6Probe.dsk" "$ROOT/outputs/toolchain/"
shasum -a 256 "$ROOT/outputs/toolchain/Sys6Probe.bin" "$ROOT/outputs/toolchain/Sys6Probe.dsk" > "$ROOT/outputs/toolchain/SHA256SUMS"
cat "$ROOT/outputs/toolchain/build.log"
