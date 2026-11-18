#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python3 "$ROOT/scripts/prepare-busybox-source.py"
python3 "$ROOT/scripts/prepare-busybox-tools.py"
python3 "$ROOT/scripts/prepare-busybox-additions.py"
mkdir -p "$ROOT/work/busybox-additions/objects"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
w=work/busybox-additions
rm -f $w/objects/work_busybox_busybox-1.37.0_archival_libarchive_data_extract_to_stdout.c.o
u=work/busybox/busybox-1.37.0
flags="-std=gnu99 -m68000 -Os -funsigned-char -ffunction-sections -fdata-sections -I/src/src/ports/busybox-additions -I/src/src/ports/busybox-tools -I/src/work/busybox-additions -I/src/src/ports/include"
compile() {
 srcfile="$1"; objfile="$2"; shift 2
 key=$( (echo "$flags $*"; cat "$srcfile" work/busybox-additions/bb_archive.h work/busybox-additions/dump.h src/ports/safe-save.h src/ports/alloc-pages.inc src/ports/busybox-additions/libbb.h src/ports/busybox-tools/libbb.h src/ports/busybox-tools/common_bufsiz.h work/busybox-additions/config.h work/busybox-additions/help.h src/ports/compat.h) | sha256sum | cut -d" " -f1)
 if [ -f "$objfile" ] && [ -f "$objfile.key" ] && [ "$(cat "$objfile.key")" = "$key" ]; then return; fi
 m68k-apple-macos-gcc $flags "$@" -c "$srcfile" -o "$objfile"
 echo "$key" > "$objfile.key"
}
for f in src/ports/busybox-additions/platform.c $u/libbb/isqrt.c $u/libbb/uuencode.c $u/libbb/process_escape_sequence.c $w/dump.c $w/get_header_tar.c $u/archival/libarchive/init_handle.c $u/archival/libarchive/data_align.c $u/archival/libarchive/data_skip.c $u/archival/libarchive/seek_by_read.c $u/archival/libarchive/header_list.c $u/archival/libarchive/header_skip.c $u/archival/libarchive/filter_accept_all.c $w/data_extract_to_stdout.c src/ports/busybox-additions/tar-platform.c $w/decompress_gunzip.c src/ports/busybox-additions/compression.c src/ports/busybox-tools/platform.c $u/libbb/getopt32.c $u/libbb/llist.c $u/libbb/skip_whitespace.c $u/libbb/compare_string_array.c $u/libbb/process_escape_sequence.c work/busybox-tools/full_read.c $u/libbb/full_write.c $u/libbb/get_line_from_file.c $u/libbb/xrealloc_vector.c $u/libbb/bb_cat.c $u/libbb/copyfd.c $u/libbb/crc32.c $u/libbb/single_argv.c $u/libbb/last_char_is.c $u/libbb/get_last_path_component.c src/ports/path-names.c; do
compile $f $w/objects/$(echo $f | tr / _).o
done
compile src/ports/runtime.c $w/objects/runtime.o
rm -f $w/objects/libc_a-getopt.o
compile $w/getopt.c $w/objects/getopt.o -I/src/src/ports/terminal -include /src/src/ports/terminal/ncurses-compat.h
compile src/ports/terminal/platform.c $w/objects/env-platform.o -I/src/src/ports/terminal
(cd $w/objects; m68k-apple-macos-ar x $(m68k-apple-macos-gcc -print-file-name=libc.a) libc_a-strspn.o libc_a-strsep.o libc_a-strtok_r.o)
for name in diff base64 xxd patch gzip gunzip tar; do
extra=""; case "$name" in sed|grep) extra="-Dregexec=port_regexec";; esac
compile $w/$name.c $w/$name.o $extra
compile src/ports/busybox-additions/entry.c $w/entry-$name.o -DAPPLET=$name
m68k-apple-macos-gcc -m68000 -nostdlib -r -Wl,-d,--gc-sections,-u,clarice_main $w/$name.o $w/entry-$name.o $w/objects/*.o -lgcc -o $w/bb$name.o
done
'
for name in diff base64 xxd patch gzip gunzip tar; do
python3 "$ROOT/scripts/pack-command.py" "$ROOT/work/busybox-additions/bb$name.o.gdb" "$ROOT/work/busybox-additions/bb$name.clx" --runtime 1.10
done
