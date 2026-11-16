#!/bin/sh
set -eu
# Run the normal targeted mktemp build first. Do not install these as utilities.
docker run --rm --pull never --platform linux/amd64 --network none -v "$PWD:/src" ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
cd /src
flags="-m68000 -Os -ffunction-sections -fdata-sections"
m68k-apple-macos-gcc $flags -c tests/reuse-audit/mktemp-fault.c -o work/http-upstream/fault.o
objects="";for f in work/build/pipelines/*.o; do case "$f" in */runtime.o|*/mktemp-runtime.o|*/wc.o|*/head.o|*/cut.o|*/tr.o|*/uniq.o|*/tee.o|*/tail.o|*/sort.o|*/basename.o|*/dirname.o|*/cmp.o|*/cksum.o|*/sleep.o|*/printenv.o|*/dd.o) ;; *) objects="$objects $f";; esac; done
for mode in write cancel; do
 flag=-Dcli_write=fixture_write; if [ "$mode" = cancel ]; then flag=-Dapp_yield=fixture_yield; fi
 m68k-apple-macos-gcc $flags $flag -DCLARICE_DD_RUNTIME -DCLARICE_MKTEMP_RUNTIME -c src/ports/runtime.c -o work/http-upstream/fault-runtime.o
 m68k-apple-macos-ld -r --gc-sections -u clarice_main $objects work/http-upstream/fault-runtime.o work/http-upstream/fault.o $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/http-upstream/mktemp-$mode-fault.o
done
'
for mode in write cancel; do python3 scripts/pack-command.py work/http-upstream/mktemp-$mode-fault.o work/http-upstream/mktemp-$mode-fault.clx; done
