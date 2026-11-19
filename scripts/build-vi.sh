#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ "${1:-}" != "--relink" ]; then python3 "$ROOT/scripts/prepare-vi.py"; fi
mkdir -p "$ROOT/work/vi/objects"
if [ "${1:-}" != "--relink" ]; then rm -f "$ROOT/work/vi/objects/"*.o; fi
rm -f "$ROOT/work/vi/vi.clx"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" -e VI_RELINK="${1:-}" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
flags="-std=gnu99 -m68000 -Os -flto -fcommon -ffunction-sections -fdata-sections -DCLARICE -DVMUNIX -DBIT8 -DISO8859_1 -DUCVISUAL -I/src/src/ports/vi -I/src/src/ports/terminal -I/src/work/vi/cross"
if [ "$VI_RELINK" != "--relink" ]; then
pids=""
for f in ex ex_addr ex_cmds ex_cmds2 ex_cmdsub ex_data ex_extern ex_get ex_io ex_put ex_re ex_set ex_subr ex_tagio ex_temp ex_tty ex_unix ex_v ex_vadj ex_vget ex_vmain ex_voper ex_vops ex_vops2 ex_vops3 ex_vput ex_vwind printf ex_version libterm/termcap libterm/tgoto libterm/tputs; do
m68k-apple-macos-gcc $flags -include /src/src/ports/vi/compat.h -c work/vi/cross/$f.c -o work/vi/objects/$(basename $f).o &
pids="$pids $!"
set -- $pids; if [ "$#" -eq 2 ]; then for pid in $pids; do wait "$pid"; done; pids=""; fi
done
for pid in $pids; do wait "$pid"; done
fi
m68k-apple-macos-gcc $flags -fno-lto -c src/ports/vi/platform.c -o work/vi/objects/platform.o
m68k-apple-macos-gcc $flags -c src/ports/terminal/platform.c -o work/vi/objects/terminal.o
m68k-apple-macos-gcc $flags -DCLARICE_DD_RUNTIME -Dclarice_main=vi_runtime_main -c src/ports/runtime.c -o work/vi/objects/runtime.o
m68k-apple-macos-gcc -m68000 -O2 -fwhole-program -flto=2 -flto-partition=one -flinker-output=nolto-rel -nostdlib -r -Wl,-d,--gc-sections,-u,clarice_main work/vi/objects/*.o -lgcc -o work/vi/vi-linked.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/vi/vi-linked.o.gdb" "$ROOT/work/vi/vi.clx" --runtime 1.9
