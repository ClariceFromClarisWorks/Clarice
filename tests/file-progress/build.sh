#!/bin/sh
set -eu
mkdir -p work/file-progress/before work/file-progress/after outputs/file-progress
# The parent includes accepted ls work and current filesystem safety fixes.
git show 4cc7eb6:src/commands/files.c > work/file-progress/before/files.c
python3 - <<'PY'
from pathlib import Path
p=Path('work/file-progress/before/files.c');p.write_text(p.read_text().replace('"mac-files.h"','"/src/src/commands/mac-files.h"'))
PY
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
flags="-m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation"
m68k-apple-macos-gcc $flags -c work/file-progress/before/files.c -o work/file-progress/before/files.o
m68k-apple-macos-gcc $flags -c src/commands/files.c -o work/file-progress/after/files.o
libgcc=$(m68k-apple-macos-gcc -print-libgcc-file-name)
for v in before after; do m68k-apple-macos-ld -r --gc-sections -u clarice_main work/file-progress/$v/files.o "$libgcc" -o work/file-progress/$v/package.o; done
'
for v in before after; do python3 scripts/pack-command.py work/file-progress/$v/package.o work/file-progress/$v/files --runtime 1.10; done

python3 - <<'PY2'
from pathlib import Path
import re,struct
exports=set(re.findall(r'CLX_IMPORT\("([^"\n]+)"',Path('src/clx-imports.inc').read_text()))
for v in ['before','after']:
 b=Path('work/file-progress/'+v+'/files').read_bytes();h=struct.unpack('>4s7I',b[:32]);start=48+h[2]+12*h[4]
 imports=[b[start+i*64:start+(i+1)*64].split(b'\0')[0].decode() for i in range(h[5])]
 assert set(imports)<=exports,set(imports)-exports
 print(v,': all',len(imports),'imports available in existing runtime')
PY2
