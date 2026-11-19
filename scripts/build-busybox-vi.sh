#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python3 "$ROOT/scripts/prepare-busybox-vi.py"
mkdir -p "$ROOT/work/busybox/objects"
rm -f "$ROOT/work/busybox/objects/"*.o
docker run --rm --pull never --platform linux/amd64 --network none -e CLARICE_VI_PROFILE -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
flags="-std=gnu99 -m68000 -Os -flto -funsigned-char -ffunction-sections -fdata-sections -DCLARICE -I/src/src/ports/busybox -I/src/src/ports/terminal"
if [ "${CLARICE_VI_PROFILE:-0}" = 1 ]; then flags="$flags -DCLARICE_VI_PROFILE"; fi
for f in work/busybox/vi.c work/busybox/busybox-1.37.0/libbb/read_key.c work/busybox/busybox-1.37.0/libbb/full_write.c work/busybox/busybox-1.37.0/libbb/llist.c work/busybox/busybox-1.37.0/libbb/skip_whitespace.c src/ports/busybox/platform.c src/ports/terminal/platform.c; do
name=$(basename $(dirname $f))-$(basename $f .c)
# Keep the exported platform entry points visible to whole-program LTO.
extra=""; if [ "$f" = src/ports/busybox/platform.c ]; then extra=-fno-lto; fi
m68k-apple-macos-gcc $flags $extra -c $f -o work/busybox/objects/$name.o
done
m68k-apple-macos-gcc $flags -DPORT_BEFORE_CLEANUP=bb_vi_recover -Dclarice_main=bb_runtime_main -c src/ports/runtime.c -o work/busybox/objects/runtime.o
m68k-apple-macos-gcc -m68000 -Os -fwhole-program -flto=2 -flto-partition=one -flinker-output=nolto-rel -nostdlib -r -Wl,-d,--gc-sections,-u,clarice_main work/busybox/objects/*.o -lgcc -o work/busybox/linked.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/busybox/linked.o.gdb" "$ROOT/work/busybox/bbvi.clx" --runtime 1.9
cp "$ROOT/work/busybox/bbvi.clx" "$ROOT/work/busybox/vi.clx"
