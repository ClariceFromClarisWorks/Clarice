#!/bin/sh
# Inside the pinned Retro68 container, /src is this isolated worktree.
set -eu
cd /src
mkdir -p work/ls-asm
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -c src/commands/decimal.c -o work/ls-asm/baseline.o
m68k-apple-macos-gcc -m68000 -c tests/ls-asm/decimal-field.S -o work/ls-asm/candidate.o
m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -c tests/ls-asm/specialized.c -o work/ls-asm/specialized.o
for name in baseline candidate specialized; do
 m68k-apple-macos-ld --gc-sections -e $(if [ "$name" = baseline ]; then echo format_decimal_field; else echo ${name}_decimal_field; fi) -Ttext=0x10000 work/ls-asm/$name.o -o work/ls-asm/$name.elf
 m68k-apple-macos-objcopy -O binary work/ls-asm/$name.elf work/ls-asm/$name.raw
 m68k-apple-macos-objdump -dr work/ls-asm/$name.elf > work/ls-asm/$name.disassembly
 m68k-apple-macos-nm -n work/ls-asm/$name.elf > work/ls-asm/$name.symbols
 done
