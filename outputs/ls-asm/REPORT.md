# ls assembly investigation — 18 November 2026

Subsequent authorised [catalog/formatter implementation and measurements](../ls-catalog/REPORT.md) supersede this report's prototype-only status. The findings below describe the initial investigation.

Recommendation: keep native HFS enumeration and the existing C command. Trial a
small decimal-loop specialisation before considering a whole-command assembly
rewrite. The experiment demonstrates fewer executed instructions, **not a measured
Macintosh speedup**. No production code or installed application was changed.

Isolated tree: `Clarice-Ls-Asm`, branch `codex/ls-asm-investigation-20261118`, based
on `6a0546a`. Existing Main updater work and the separate CLX memory/policy tree
were preserved. This investigation does not change command packaging, builtin
classification or execution policy.

## What is already optimised

`src/commands/core.c` includes the Clarice-native `src/listing.inc`; this is not an
upstream Unix ls port. It streams native indexed `PBGetCatInfoSync` records in
catalog order. There is no sorting pass or directory-sized array to eliminate.
The row buffer is bounded, creation-date reuse is invocation-local, and the decimal
and date helpers already use 68000 `DIVU` inline assembly. Terminal copy/scrolling
also has existing assembly kernels. Rewriting the C wrapper cannot accelerate the
OS's catalog implementation or disk access.

The baseline remains stock SE/8 MHz 68000/4 MB/System 6.0.8, preserving Plus,
later 68k and single Finder. Native APIs and shared C boundaries remain useful
regardless of whether an independently packaged utility runs in-process or in a
separate native application.

## Existing complete-operation evidence

These are **historical measurements**, not runs of this prototype:

- [HFS investigation](../hfs-cost/REPORT.md): Snow 1,000-entry production scans
  379/323/322 ticks; direct synchronous enumeration 297/302/300; asynchronous
  enumeration 300/301/300. Changing native enumeration mode did not deliver a win.
- [Earlier listing profile](../list-drag/REPORT.md): 100-file detailed listing
  138/106/106 ticks, excluding loading/dispatch and final presentation. Formatting
  accounted for 10/7/13 ticks respectively (6.6–12.3%); decimal fields are only
  part of that bucket. Even deleting all formatting would save only that portion
  in those runs. This is not a forecast for current code or other workloads.
- [Paired full-command measurement](../output-perf/REPORT.md): previous combined
  formatting/output work improved real `ls -la` through final prompt from 156 to
  153 ticks for 100 files (1.9%), and 611 to 577 for 1,000 (5.6%). Those gains
  already exist; they cannot be credited to this experiment. The report's physical
  results cover cumulative earlier changes, not isolated assembly formatting.

These profiles make a whole-command rewrite a poor first investment. Name-only
`ls` and human-readable `ls -lh` do not use the decimal-field helper tested here.

## Bounded prototypes and measured costs

The target writes exactly ten right-aligned bytes for an unsigned 32-bit fork
length. Normal `ls -l` invokes it for data and resource fork lengths of files.

1. **Hand assembly:** single-word division for values up to 65,535; decimal-power
   subtraction for larger values. Original 68000 code, no third-party source
   copied. The large-value path tests whether eliminating expensive divides helps,
   but requires many more branches/subtractions.
2. **C specialisation with existing inline DIVU:** retain two-step long division
   while needed, then switch to a short loop once the remaining quotient fits a
   word. This removes repeated high-word setup without a lookup table or a new
   algorithm for large values.

| Isolated linked routine | Baseline | Hand assembly | C + inline DIVU |
| --- | ---: | ---: | ---: |
| Binary bytes | 108 | 176 | 130 |
| Difference | — | +68 | +22 |
| Heap allocation / resident cache | none | none | none |
| Executed instructions, value 0 | 61 | 54 | 56 |
| Executed instructions, value 8,192 | 97 | 63 | 68 |
| Executed instructions, value 65,536 | 114 | 259 | 84 |
| Executed instructions, value 1,048,576 | 143 | 280 | 104 |
| Executed instructions, value 4,294,967,295 | 194 | 394 | 152 |

All routines save one register (four stack bytes), apart from the same caller's
arguments/return address. Assembly has a 40-byte constant table within its 176
bytes. Baseline and C versions include compiler function-name trailers: their
instruction regions are 84 and 102 bytes; assembly's is 136. Thus the isolated
+22-byte C delta includes four extra metadata bytes from its experimental name;
these are not final CLX payload-size measurements. No loader, import, application
partition or idle allocation change was made.

At 8,192, instruction counts fall 35.1% for assembly and 29.9% for the C change,
but **all three still execute four DIVU instructions**. At the maximum value,
baseline/C execute 15 DIVU; assembly executes none but more than doubles the total
instruction count. Instruction costs differ, division timing is operand-dependent,
and Unicorn is not cycle-accurate: neither comparison establishes elapsed speed.
The C candidate captures most of the small-value instruction saving with less
code and avoids the table/subtraction tradeoff. No claimed full-command gain.

## Validation and reproduction

Pinned Retro68 compilation with `-m68000 -Os` succeeded. Unicorn 2.1.4 configured
as M68000 executed **75,567 cases per routine** (226,701 total): every value
0–65,536, decimal boundaries, signed/unsigned endpoints, and 10,000 seeded random
32-bit values. Each result matched Python's independent decimal formatting;
tests also checked odd output addresses, adjacent guard bytes, preserved ABI
registers and balanced stack. This is functional CPU emulation, not Snow,
Macintosh OS execution or physical hardware validation. Tracing explicitly
invalidates pre-existing translation blocks and rejects zero instruction counts.

[Raw results](cpu-results.txt), [structured results](cpu-results.json),
[build log](build.log). Sources and harness: `tests/ls-asm/`.

From this worktree:

```sh
docker run --rm --pull never --platform linux/amd64 --network none \
  -v "$PWD:/src" \
  ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c \
  sh /src/tests/ls-asm/build.sh
python3 tests/ls-asm/compare.py
```

Python requires Unicorn 2.1.4. This run reused
`../Clarice/work/date-test-env/bin/python`; no dependency installation was needed.
The build generates only isolated routines under `work/ls-asm`; it does not build
or install Clarice. Production listing regressions were not rerun because the
production implementation was unchanged.

## Research and decision

Apple's [File Manager performance guidance (FL31)](https://leopard-adc.pepas.com/technotes/fl/fl_31.html)
and [Searching a Volume](https://leopard-adc.pepas.com/documentation/mac/Files/Files-94.html)
provide the native enumeration starting point. Existing repository experiments
already compare the relevant synchronous/asynchronous alternatives; CatSearch
cannot be assumed on the System 6 baseline. The
[Motorola programmer's reference](https://cache.nxp.com/docs/en/reference-manual/M68000PM.pdf)
defines the word-divide instruction used here; later-CPU long division is not an
option for the 68000 baseline. The experiment uses established word division and
decimal subtraction, introducing no loader, dependency or runtime abstraction.

Do not promote the full assembly prototype on instruction counts alone. The next
focused step is an alternating baseline/C-specialised **complete `ls -la` A/B**
on identical 100/1,000-file fixtures with varied fork sizes: first/repeat launches,
screen and redirected output, command submission through final prompt, identical
output, peak/free-memory recovery, failure/cancellation and actual CLX size.
Use Snow for reproducible functional evidence and the physical 8 MHz SE for final
performance acceptance. Retain only a repeatable worthwhile full-operation gain.

A subsequent name-only investigation could measure replacing its bounded
`snprintf("%s%s", ...)` with direct name/suffix handling in C. That is a separate
unmeasured opportunity, not a reason to rewrite ls or another utility in assembly.
