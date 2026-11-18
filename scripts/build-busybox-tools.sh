#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
[ -d "$ROOT/work/busybox/busybox-1.37.0" ] || python3 "$ROOT/scripts/prepare-busybox-vi.py"
python3 "$ROOT/scripts/prepare-busybox-tools.py"
mkdir -p "$ROOT/work/busybox-tools/objects"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
w=work/busybox-tools
u=work/busybox/busybox-1.37.0
flags="-std=gnu99 -m68000 -Os -funsigned-char -ffunction-sections -fdata-sections -I/src/src/ports/busybox-tools -I/src/work/busybox-tools -I/src/src/ports/include"
compile() {
 srcfile="$1"; objfile="$2"; shift 2
 key=$( (echo "$flags $*"; cat "$srcfile" src/ports/busybox-tools/libbb.h src/ports/busybox-tools/common_bufsiz.h work/busybox-tools/config.h work/busybox-tools/help.h src/ports/compat.h) | sha256sum | cut -d" " -f1)
 if [ -f "$objfile" ] && [ -f "$objfile.key" ] && [ "$(cat "$objfile.key")" = "$key" ]; then return; fi
 m68k-apple-macos-gcc $flags "$@" -c "$srcfile" -o "$objfile"
 echo "$key" > "$objfile.key"
}
for f in src/ports/busybox-tools/platform.c $u/libbb/getopt32.c $u/libbb/llist.c $u/libbb/skip_whitespace.c $u/libbb/compare_string_array.c $u/libbb/process_escape_sequence.c $w/full_read.c $u/libbb/full_write.c $u/libbb/get_line_from_file.c $u/libbb/xregcomp.c $u/libbb/xrealloc_vector.c $u/libbb/bb_cat.c $u/libbb/copyfd.c $u/libbb/crc32.c $u/libbb/single_argv.c $u/libbb/last_char_is.c $u/libbb/get_last_path_component.c src/ports/path-names.c; do
compile $f $w/objects/$(basename $f .c).o
done
mkdir -p work/build/ports
python3 scripts/prepare-ports.py
for f in work/build/ports/regcomp.c work/build/ports/regexec.c src/vendor/musl-regex/tre-mem.c src/vendor/musl-regex/regerror.c src/ports/regex-wrapper.c src/vendor/sbase/libutil/strcasestr.c; do
compile $f $w/objects/$(basename $f .c).o -Dhidden= -DCHARCLASS_NAME_MAX=14 -DRE_DUP_MAX=255 -I/src/src/vendor/musl-regex -include /src/src/ports/compat.h
done
compile src/ports/runtime.c $w/objects/runtime.o
rm -f $w/objects/libc_a-getopt.o
compile src/vendor/newlib-getopt/getopt.c $w/objects/getopt.o -I/src/src/ports/terminal -include /src/src/ports/terminal/ncurses-compat.h
compile src/ports/terminal/platform.c $w/objects/env-platform.o -I/src/src/ports/terminal
(cd $w/objects; m68k-apple-macos-ar x $(m68k-apple-macos-gcc -print-file-name=libc.a) libc_a-strspn.o libc_a-strsep.o libc_a-strtok_r.o)
for name in wc tr cmp sort sed grep cat cut head tail uniq tee cksum basename dirname; do
extra=""; case "$name" in sed|grep) extra="-Dregexec=port_regexec";; esac
compile $w/$name.c $w/$name.o $extra
compile src/ports/busybox-tools/entry.c $w/entry-$name.o -DAPPLET=$name
m68k-apple-macos-gcc -m68000 -nostdlib -r -Wl,-d,--gc-sections,-u,clarice_main $w/$name.o $w/entry-$name.o $w/objects/*.o -lgcc -o $w/bb$name.o
done
'
for name in wc tr cmp sort sed grep cat cut head tail uniq tee cksum basename dirname; do
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/busybox-tools/bb$name.o.gdb" "$ROOT/work/busybox-tools/bb$name.clx" --runtime 1.9
done
