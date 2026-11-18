#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
mkdir -p "$ROOT/outputs/namespace"
python3 "$ROOT/scripts/release.py" begin
# A full build cannot inherit a caller's partial utility selection.
unset CLARICE_BUILD_COMMANDS
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
for name in files inspect misc free; do
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/$name.c -o /src/work/build/$name-main.o
 extra=""
 if [ "$name" = inspect ]; then
  m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -I/src/src/vendor/musl-fnmatch -I/src/src/ports/fnmatch-include -c /src/src/ports/fnmatch-byte.c -o /src/work/build/fnmatch-byte.o
  extra=/src/work/build/fnmatch-byte.o
 fi
 libgcc=$(m68k-apple-macos-gcc -print-libgcc-file-name)
 m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/build/$name-main.o $extra "$libgcc" -o /src/work/build/$name.o
done
m68k-apple-macos-gcc -m68000 -Os -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/serial.c -o /src/work/build/serial.o
m68k-apple-macos-gcc -m68000 -Os -ffreestanding -fno-common -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation -c /src/src/commands/core.c -o /src/work/build/core-main.o
m68k-apple-macos-gcc -m68000 -Os -ffreestanding -fno-common -Wall -Wextra -Werror -c /src/src/commands/created.c -o /src/work/build/created.o
m68k-apple-macos-gcc -m68000 -Os -ffreestanding -fno-common -Wall -Wextra -Werror -c /src/src/commands/decimal.c -o /src/work/build/decimal.o
m68k-apple-macos-ld -r /src/work/build/core-main.o /src/work/build/created.o /src/work/build/decimal.o -o /src/work/build/core.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/core.o" "$ROOT/work/build/core.clx" --runtime 1.10
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/serial.o" "$ROOT/work/build/serial.clx" --runtime 1.0
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/files.o" "$ROOT/work/build/files.clx" --runtime 1.10
for name in inspect misc; do python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/$name.o" "$ROOT/work/build/$name.clx"; done
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/build/free.o" "$ROOT/work/build/free.clx" --runtime 1.4
"$ROOT/scripts/build-ports.sh"
"$ROOT/scripts/build-awk.sh"
"$ROOT/scripts/build-pipelines.sh"
"$ROOT/scripts/build-busybox-tools.sh"
for name in wc tr sort; do cp "$ROOT/work/busybox-tools/bb$name.clx" "$ROOT/work/build/bb$name.clx"; done
"$ROOT/scripts/build-kermit.sh"
sh "$ROOT/scripts/build-network.sh"
sh "$ROOT/scripts/build-ftp.sh"
sh "$ROOT/scripts/build-telnet.sh"
for name in ipconfig ping wget; do cp "$ROOT/work/network/$name.clx" "$ROOT/work/build/$name.clx"; done
cp "$ROOT/work/ftp/ftp.clx" "$ROOT/work/telnet/telnet.clx" "$ROOT/work/build/"
sh "$ROOT/scripts/build-update.sh"

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

python3 "$ROOT/scripts/release.py" seal
python3 "$ROOT/scripts/package-utilities.py"
