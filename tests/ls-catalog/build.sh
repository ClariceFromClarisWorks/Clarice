#!/bin/sh
# Focused packages only; no Core/application rebuild or installation.
set -eu
mkdir -p work/ls-catalog/build
python3 - <<'PY'
from pathlib import Path
import subprocess
out=Path('work/ls-catalog/build')
def original(p):return subprocess.check_output(['git','show','6a0546a:'+p],text=True)
for variant in ('base','fmt','next'):
 d=out/variant;d.mkdir(exist_ok=True)
 core=original('src/commands/core.c') if variant!='next' else Path('src/commands/core.c').read_text()
 listing=original('src/listing.inc') if variant!='next' else Path('src/listing.inc').read_text()
 decimal=original('src/commands/decimal.c') if variant=='base' else Path('src/commands/decimal.c').read_text()
 core=core.replace('../sys6.h','/src/src/sys6.h').replace('../listing.inc','listing.inc')
 (d/'core.c').write_text(core);(d/'listing.inc').write_text(listing);(d/'decimal.c').write_text(decimal)
PY
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
flags="-m68000 -Os -ffreestanding -fno-common -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation"
m68k-apple-macos-gcc $flags -c src/commands/created.c -o work/ls-catalog/build/created.o
m68k-apple-macos-gcc $flags -c tests/ls-catalog/flush.c -o work/ls-catalog/flush.o
for variant in base fmt next; do
 d=work/ls-catalog/build/$variant
 m68k-apple-macos-gcc $flags -c $d/core.c -o $d/core.o
 m68k-apple-macos-gcc $flags -c $d/decimal.c -o $d/decimal.o
 m68k-apple-macos-ld -r $d/core.o $d/decimal.o work/ls-catalog/build/created.o -o $d/package.o
done
'
for variant in base fmt next; do
 python3 scripts/pack-command.py work/ls-catalog/build/$variant/package.o work/ls-catalog/build/$variant/ls --runtime 1.10
done

python3 scripts/pack-command.py work/ls-catalog/flush.o work/ls-catalog/flush --runtime 1.10
