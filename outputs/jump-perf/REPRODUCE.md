# Reproduce development checkpoint 2026111517

This is a source checkpoint, not a release. The existing build tested in Snow
and on the physical SE is identified by its lengths and SHA-256 in [REPORT.md](REPORT.md).
Generated apps, ROMs, OS images, private network settings/tokens and temporary
captures are deliberately not committed. Historical reports may link to those
local artifacts; their absence from Git is intentional.

## Normal application

Use a fresh checkout, Python 3 and Docker. All normal application/command sources
are included. The pinned Retro68 image must already be available, or fetch it
once explicitly:

```sh
docker pull --platform linux/amd64 ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
sh scripts/build.sh
```

Output: `outputs/namespace/Clarice.bin` (MacBinary, both forks) and `Clarice.dsk`.
Build containers have no network access. A fresh `work/build` avoids carrying
diagnostic CMake flags from prior experiments. Build stamp is checked in at
`src/update/version.h`; no date-based version regeneration is required.
The normal build has all optional diagnostic flags absent/off. Do not deploy a
diagnostic build over the accepted SE application just to reproduce a benchmark.

Checkpoint verification: the normal build completed from an isolated archive of
the staged files with an empty work directory. Both Clarice CODE resources match
the physically accepted application byte-for-byte. The complete package is **not
byte-identical**: 14 bundled CLX resources differ after rebuilding all commands;
the accepted package retained older utility builds. The fresh resource fork is
944,266 bytes versus the accepted 939,714. This fresh package has not been deployed
or guest-tested and must not inherit the accepted package's hardware qualification.
Both package hashes and resource differences are in
[checkpoint-validation.json](checkpoint-validation.json). Application build dates
also prevent assuming universal byte-for-byte package reproducibility.

The documented drawing checks were run from that isolated archive: 356 M68000
glyph cases, 532 row comparisons, 510 glyph-builder cases, selection delta and
1,560 scroll guards passed. The scroll-guard fixture requires the work directory
created below; its first invocation without that directory failed before testing.
Existing upstream/license whitespace is preserved rather than reformatted.

## Focused drawing checks

Use Python with Unicorn **2.1.4** for real M68000 instruction tests, plus a host C
compiler with AddressSanitizer/UndefinedBehaviorSanitizer. Run from repository root:

```sh
mkdir -p work/jump-perf work/frame-perf work/snow-performance
docker run --rm --pull never --platform linux/amd64 --network none \
  -v "$PWD:/src" \
  ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c sh -ec '
  m68k-apple-macos-gcc -m68000 -c /src/src/kernels.S -o /src/work/jump-perf/kernels.o
  m68k-apple-macos-ld -Ttext=0x10000 -e asm_cells /src/work/jump-perf/kernels.o -o /src/work/jump-perf/kernels.elf
  m68k-apple-macos-objcopy -O binary /src/work/jump-perf/kernels.elf /src/work/jump-perf/kernels.raw
  m68k-apple-macos-nm /src/work/jump-perf/kernels.elf > /src/work/jump-perf/symbols.txt
  '
cp work/jump-perf/kernels.raw work/jump-perf/symbols.txt work/frame-perf/
python3 tests/terminal/glyph-68k.py
python3 tests/terminal/frame-68k.py
python3 tests/terminal/native-glyphs.py
python3 tests/terminal/selection-host.py
python3 tests/terminal/scroll-guard.py
```

Guest diagnostics are opt-in: after preparing normal command resources, configure
the same CMake build with `-DCLARICE_DRAW_DIAGNOSTICS=ON` using the pinned toolchain.
`scrollbench jumps` alternates both paths in one executable, three repetitions of
24 jumps per workload. `scrollbench glyphpixels` checks dense rows against native
QuickDraw and verifies the scrollbar. Match the accepted 80×24, Monaco 9, light
theme, 128-line history, 4 MB/System 6.0.8 configuration; also run the pixel check
in dark theme. Keep host input/capture out of timed runs. Physical evidence used
stock 68000 mode and quiet SEControl. Guest media must be supplied locally.

## Preserved experimental nano

Nano remains a separate installed CLX command, not bundled in the normal app.
The previously ignored, accepted target configuration and generated GNU headers
are now source inputs in `src/ports/nano/prepared`; licenses are retained.
This does not change nano configuration or the safe-save adapter.

```sh
python3 scripts/prepare-nano.py
sh scripts/build-ncurses.sh
sh scripts/link-nano-probe.sh
```

Preparation fetches checksum-pinned nano 9.2 and ncurses 6.5 sources when absent;
archives can instead be supplied in `work/nano` and `work/ncurses`. Nano preparation
refuses to overwrite an existing source tree. Existing save/performance patches
are applied by the link script. Output is `work/nano/nano.clx`. No new editor guest
acceptance is claimed by reproducing the source preparation.

The preparation plus both existing patches reproduced all 445 C/header inputs
in the accepted local nano tree exactly. No new full nano build was needed for
this checkpoint.
