#!/bin/sh
# Run from repository root. Test applications only; no Clarice build changes.
set -eu
image=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" "$image" sh -ec '
cmake -S /src/tests/native-bench -B /src/work/native-bench/build -DCMAKE_TOOLCHAIN_FILE=/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
cmake --build /src/work/native-bench/build
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wno-multichar -c /src/tests/native-bench/driver.c -o /src/work/native-bench/driver.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/native-bench/driver.o -L/Retro68-build/toolchain/m68k-apple-macos/lib -lInterface $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/native-bench/linked.o
python3 /src/scripts/pack-command.py /src/work/native-bench/linked.o /src/work/native-bench/nb.clx
'
python3 - <<'PYIMAGE'
from pathlib import Path
b=Path('work/native-bench/build/NBWorker.bin').read_bytes()
Path('work/native-bench/worker-image.h').write_text('static const unsigned char worker_image[] = {'+','.join(str(x) for x in b)+'};\n')
PYIMAGE
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" "$image" sh -ec '
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wno-multichar -I/src/work/native-bench -c /src/tests/native-bench/install.c -o /src/work/native-bench/install.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/native-bench/install.o -L/Retro68-build/toolchain/m68k-apple-macos/lib -lInterface $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/native-bench/install-linked.o
python3 /src/scripts/pack-command.py /src/work/native-bench/install-linked.o /src/work/native-bench/nbi.clx
'
