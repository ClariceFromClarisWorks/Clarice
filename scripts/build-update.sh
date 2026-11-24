#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
mkdir -p "$ROOT/work/update-command"
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
cmake -S /src/src/update -B /src/work/update-helper -DCMAKE_TOOLCHAIN_FILE=/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
cmake --build /src/work/update-helper -j2
'
python3 - "$ROOT" <<'PY'
from pathlib import Path
import sys,hashlib,struct,binascii
root=Path(sys.argv[1]);b=(root/'work/update-helper/ClariceUpdater.bin').read_bytes()
d,r=struct.unpack_from('>II',b,83);offset=128+((d+127)//128)*128;resource=b[offset:offset+r]
(root/'work/update-command/update-helper-stamp.h').write_text('static const unsigned long offline_helper_data='+str(d)+'UL,offline_helper_resource='+str(r)+'UL,offline_helper_data_crc='+str(binascii.crc32(b[128:128+d]))+'UL,offline_helper_resource_crc='+str(binascii.crc32(resource[:16]+resource[128:]))+'UL;\nstatic const char offline_helper_leaf[]=\"Updater-'+hashlib.sha256(b).hexdigest()[:12]+'\";\n')
p=root/'work/update-command/update-helper-image.h'
p.write_text('static const unsigned char update_helper_image[]={\n'+''.join(','.join(str(x) for x in b[i:i+24])+',\n' for i in range(0,len(b),24))+'};\nstatic const char update_helper_leaf[]=\"Updater-'+hashlib.sha256(b).hexdigest()[:12]+'\";\n')
PY
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
FLAGS="-m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror=implicit-function-declaration -Wno-multichar -Wno-misleading-indentation -Wno-unused-function -Wno-missing-field-initializers -I/src/work/update-command"
m68k-apple-macos-gcc $FLAGS -c /src/src/commands/clarice.c -o /src/work/update-command/clarice.o
m68k-apple-macos-gcc $FLAGS -Dclarice_main=update_wget_main -c /src/src/commands/wget.c -o /src/work/update-command/wget.o
m68k-apple-macos-gcc $FLAGS -c /src/src/commands/vendor/mactcp/nettcp.c -o /src/work/update-command/nettcp.o
m68k-apple-macos-gcc $FLAGS -c /src/src/commands/network/native.c -o /src/work/update-command/native.o
m68k-apple-macos-gcc $FLAGS -DNDEBUG -c /src/src/commands/vendor/picohttpparser/picohttpparser.c -o /src/work/update-command/picohttpparser.o
m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/update-command/clarice.o /src/work/update-command/wget.o /src/work/update-command/nettcp.o /src/work/update-command/native.o /src/work/update-command/picohttpparser.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o /src/work/update-command/command.o
'
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/update-command/command.o" "$ROOT/work/update-command/clarice.clx" --runtime 1.11
cp "$ROOT/work/update-command/clarice.clx" "$ROOT/work/build/clarice.clx"

python3 "$ROOT/scripts/self-update-resource.py"
