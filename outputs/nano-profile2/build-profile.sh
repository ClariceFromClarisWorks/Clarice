set -eu
cd /src
mkdir -p work/nano-profile2/objects
# First run scripts/link-nano-probe.sh for the normal objects and pinned sources.
cp work/nano/objects/*.o work/nano-profile2/objects/
flags='-DPORT_NANO_PERF -m68000 -Os -ffunction-sections -fdata-sections -DHAVE_CONFIG_H -I/src/work/nano/config -I/src/work/nano/regex-include -I/src/src/ports/terminal -I/src/work/ncurses/build/include -idirafter /src/work/nano/cross/nano-9.2/lib'
for name in browser chars color cut files global help history move nano prompt rcfile search text utils winio; do m68k-apple-macos-gcc $flags -include /src/src/ports/nano/nano-compat.h -Dmain=nano_main -c work/nano/cross/nano-9.2/src/$name.c -o work/nano-profile2/objects/$name.o; done
m68k-apple-macos-gcc $flags -c src/ports/nano/platform.c -o work/nano-profile2/objects/platform.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -DPORT_NANO_PERF -c src/ports/runtime.c -o work/nano-profile2/objects/runtime.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -c tests/ports/nano-refresh.c -o work/nano-profile2/objects/refresh.o
m68k-apple-macos-ld -r --gc-sections --wrap=doupdate -u clarice_main work/nano-profile2/objects/*.o work/ncurses/build/lib/libncurses.a $(m68k-apple-macos-gcc -print-libgcc-file-name) -o work/nano-profile2/nano-linked.o
python3 scripts/pack-command.py work/nano-profile2/nano-linked.o work/nano-profile2/nano-format.clx
