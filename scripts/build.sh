#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
mkdir -p "$ROOT/outputs/namespace"
mkdir -p "$ROOT/work/build"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
for name in files inspect misc; do
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/$name.c -o /src/work/build/$name-main.o
 libgcc=$(m68k-apple-macos-gcc -print-libgcc-file-name)
 m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/build/$name-main.o "$libgcc" -o /src/work/build/$name.o
done
m68k-apple-macos-gcc -m68000 -Os -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/serial.c -o /src/work/build/serial.o
m68k-apple-macos-gcc -m68000 -Os -ffreestanding -fno-common -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/core.c -o /src/work/build/core-main.o
m68k-apple-macos-gcc -m68000 -Os -ffreestanding -fno-common -Wall -Wextra -Werror -c /src/src/commands/created.c -o /src/work/build/created.o
m68k-apple-macos-gcc -m68000 -Os -ffreestanding -fno-common -Wall -Wextra -Werror -c /src/src/commands/decimal.c -o /src/work/build/decimal.o
m68k-apple-macos-ld -r /src/work/build/core-main.o /src/work/build/created.o /src/work/build/decimal.o -o /src/work/build/core.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/core.o" "$ROOT/work/build/core.clx"
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/serial.o" "$ROOT/work/build/serial.clx"
for name in files inspect misc; do python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/$name.o" "$ROOT/work/build/$name.clx"; done
"$ROOT/scripts/build-ports.sh"
"$ROOT/scripts/build-awk.sh"
"$ROOT/scripts/build-pipelines.sh"
python3 "$ROOT/scripts/command-resource.py"

docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cmake -S /src/src -B /src/work/build -DCMAKE_TOOLCHAIN_FILE=/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
cmake --build /src/work/build -j2
' > "$ROOT/outputs/namespace/build.log" 2>&1 || { cat "$ROOT/outputs/namespace/build.log"; exit 1; }
python3 "$ROOT/scripts/finish-application.py"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
hmount /src/work/build/Clarice.dsk
hcopy -m /src/work/build/Clarice.bin :Clarice
humount
' >> "$ROOT/outputs/namespace/build.log" 2>&1
cp "$ROOT/work/build/Clarice.bin" "$ROOT/work/build/Clarice.dsk" "$ROOT/outputs/namespace/"
cat "$ROOT/outputs/namespace/build.log"
