#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p work/utility-correctness
python3 tests/utility-correctness/file-depth.py
cc -g -O1 -fsanitize=address,undefined -c src/ports/runtime.c -o work/utility-correctness/runtime-host.o
cc -g -O1 -fsanitize=address,undefined -Isrc/ports -c tests/ports/streams.c -o work/utility-correctness/streams-host.o
cc -g -O1 -fsanitize=address,undefined tests/ports/streams-driver.c work/utility-correctness/streams-host.o work/utility-correctness/runtime-host.o -o work/utility-correctness/streams
printf 'x\n' > work/utility-correctness/stream-input
work/utility-correctness/streams work/utility-correctness/stream-input
work/utility-correctness/streams work/utility-correctness/stream-input 16 sequential
