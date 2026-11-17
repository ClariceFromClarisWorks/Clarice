#!/bin/sh
# Run inside the existing pinned Retro68 container at /src.
# Reuse accepted nano/ncurses objects; compile only adapter and diagnostics.
set -eu
cd /src
mkdir -p work/heap-audit
CC=m68k-apple-macos-gcc
FLAGS='-m68000 -Os -ffunction-sections -fdata-sections'
$CC $FLAGS -c tests/heap-audit/snapshot.c -o work/heap-audit/snapshot.o
$CC $FLAGS -I/src/src/ports/terminal -c src/ports/terminal/platform.c -o work/nano/objects/terminal.o
$CC $FLAGS -I/src/src/ports/terminal -c tests/terminal/environment-guest.c -o work/heap-audit/environment-guest.o
$CC $FLAGS -Dclarice_main=runtime_main -Dcli_alloc=env_test_alloc -c src/ports/runtime.c -o work/heap-audit/fault-runtime.o
$CC $FLAGS -c tests/heap-audit/environment-fault.c -o work/heap-audit/environment-fault.o
LD=m68k-apple-macos-ld
$LD -r --gc-sections -u clarice_main work/heap-audit/environment-guest.o work/nano/objects/terminal.o work/nano/objects/runtime.o -o work/heap-audit/environment-linked.o
$LD -r --gc-sections -u clarice_main work/heap-audit/environment-fault.o work/heap-audit/fault-runtime.o work/heap-audit/environment-guest.o work/nano/objects/terminal.o -o work/heap-audit/fault.o
$LD -r --gc-sections -u clarice_main work/nano/objects/*.o work/ncurses/build/lib/libncurses.a $($CC -print-libgcc-file-name) -o work/nano/nano-linked.o
# Pack these objects with scripts/pack-command.py on the host:
# snapshot.o -> snapshot.clx; environment-linked.o -> environment.clx;
# fault.o -> env-fault.clx; work/nano/nano-linked.o -> nano.clx.
