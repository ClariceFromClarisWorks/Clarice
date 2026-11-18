# BusyBox overlapping-tools comparison — 18 November 2026

Private worktree `Clarice-Busybox-Tools`, branch
`codex/busybox-tools-comparison-20261118`, based on `d9bf254` (the previous
BusyBox vi experiment). User-authorised comparison. After the first matched wc/tr results, the user
approved those two as replacements and requested continuing comparisons. The user
subsequently approved sort, conditional on returning its temporary RAM; all three
64 KiB runs recorded zero remaining invocation allocations. Single-Finder CLX
execution is still inside Clarice, not a separate OS application process.
They remain separately named candidates in this fixture; production replacement
is not yet implemented. No Core change, merge, push or release. Physical SE excluded.

## Scope and source

15 separately loaded experiments: `bbwc`, `bbtr`, `bbcmp`, `bbsort`, `bbsed`,
`bbgrep`, `bbcat`, `bbcut`, `bbhead`, `bbtail`, `bbuniq`, `bbtee`, `bbcksum`,
`bbbasename`, `bbdirname`. These cover the identified stream-processing hotspots
and useful controls. This is not a complete BusyBox distribution or an assertion
that every applet is portable. Native HFS commands, networking, shell and awk
are not replaced by this set. Editor comparison remains in ../busybox/README.md.

Pinned upstream BusyBox **1.37.0**, official archive:
https://busybox.net/downloads/busybox-1.37.0.tar.bz2
SHA256 `3311dff32e746499f4df0d5df04d7eb396382d7e108bb9250e7b519b837043a4`.
Licences and AUTHORS retained in licenses/busybox. `prepare-busybox-tools.py`
stages genuine source; all adaptations are separate from the extracted tree.

Minimal demonstrated boundary, not a general SDK: use existing Clarice tracked
allocation, file/stream I/O, cancellation and diagnostics. Compile upstream
getopt32, line reading, translation/escape helpers, copying, CRC32 and list/vector
helpers directly; full_read is an attributed unchanged excerpt from libbb/read.c.
Use Gregory Pietsch's public-domain newlib getopt source, pinned at
`1b7c72fdcc4bde7520407d2d3364146f04fb8312` (source/hash in
src/vendor/newlib-getopt/SOURCE.json), compiled against existing Clarice stream
and exported-environment bindings, plus the toolchain's pure strspn/strsep/strtok_r objects and
existing sbase/musl strcasestr and regex engine. GNU regex is not substituted.
The same regex implementation and polling/error wrapper as the existing commands
is used, so grep/sed comparisons primarily concern the command implementations.

All candidates compile for **68000, -Os, no LTO**. Separate CLX packages use
runtime1.9 and existing update format. Unused code is garbage-collected.
No resident allocations, Core startup work, loader changes or caches added.
Argument copies prevent upstream argv mutation from damaging shell-owned data.

## Compatibility decisions

- Byte/C locale; `wc -m` counts bytes and word rules differ from sbase Unicode.
  tr is a 256-byte translation table; no multibyte-character replacement.
  Upstream tr lacks [:graph:]/[:print:] classes and [x*N] repetition syntax;
  these are not feature-equivalent replacements for all existing tr workflows.
- wc counters are 32-bit, consistent with the platform's file limits but not
  arbitrary accumulated counts across unlimited files.
- sort stores the entire input in RAM: no external merge/spill. Full key and
  numerical sorting enabled, but -o/-M/-V/-m/-S/-T explicitly rejected. This
  prevents silent success for ignored memory/temp settings and avoids inventing
  missing descriptor/process or locale services. Shell output redirection to a
  **separate file** is supported. No in-place-save equivalence is claimed.
- sed -i excluded and rejected before processing; normal stream-editing retained.
  On input lacking a final newline, the observed BusyBox sed preserves that
  absence while existing sbase sed adds a newline (byte-level evidence in
  sed-final-newline.json). Shared timing fixtures are newline-terminated.
  Command-loop polling preserves cancellation of CPU-only branch loops. Existing
  regex wrapper turns allocation/execution errors into failures, not non-matches.
- grep -r/-R explicitly rejected; existing Clarice find/HFS services remain.
- tail follow mode (-f/-F/-s) excluded. Upstream seek optimisation assumes lines
  shorter than 64 KiB for the backwards scan; this needs a separate correctness
  decision before replacement. Input via redirected stdin uses the streaming path.
- uniq's named input binds an actual FILE rather than closing/replacing descriptor
  zero. Its output operand is rejected; shell redirection works. Upstream NUL
  record limitations remain. tee -i is rejected; cancellation stays enabled.
- dirname uses the existing slash-path adapter, not host Macintosh path parsing.
- All read/write streams refer to data forks. No fabricated Unix permissions,
  ownership, processes, signals or device semantics. Errors and cancellation use
  existing runtime cleanup. Detailed supported options are in each --help.

## Measurement method

Private Snow app bundle `org.clarice.BusyboxToolsSnow`, own cloned closed disks
and PRAM; Plus/68000,4MiB, reported System6.0.8. Core2026111823,
telnetd2026111814 retained from the previous isolated fixture. This is older than
concurrent Telnet rework: findings do not establish acceptance of that newer tree.
Host port23243 -> guest10.0.0.2:2323; private update HTTP8097. Password is read
from an untracked existing fixture, never logged. Use Clarice updater locally;
functional/comparison runs use authenticated Telnet. No physical SE involvement.

Record the installed utility catalog before any change. Existing utilities are
the preserved2026111823 fixture versions (including earlier wc/tr/traversal
optimisations), not freshly rebuilt replacements. Both competitors run on the
same Core, disk, terminal and test files. Output is redirected to separate HFS
files and compared after measuring. wc whitespace is normalised for comparison;
other shared-workload results must compare byte-for-byte.

512-byte,8192-byte,65536-byte deterministic text; separate 65536-byte binary,
UTF-8 and long-line fixtures. Three runs per workload, implementation order
reversed on the middle run. `time` measures complete command loading, execution,
cleanup and redirected output; `status runtime` records image/load/working peak
and remaining allocations. First benchmark sample is **not cold**: version/help
and correctness checks may already have verified the package. Initial candidate
version launches are diagnostic only (a clean host rebuild was active during
those launch checks); they are recorded separately, not used to rank startup. No host compiler should run during the
matched timing batch. Guest ticks (~60Hz) are primary; host durations include
Telnet/UI pacing and shared-host variation. Snow evidence is not SE acceptance.

Tracked working peaks exclude loaded image, loader metadata, stack, libc and
shared network/terminal allocations. In particular upstream cat uses a configured
4KiB stack copy buffer and several applets use a 1KiB image-local scratch area.
Peak alone is not total working memory or safe maximum file size.

## Matched timing results

All **282 timed invocations / 47 workload-size groups** completed successfully;
the harness recorded 390 observations including output verification. Every timed
invocation reported zero remaining owned allocations. Each cell below is
**existing / BusyBox median seconds**, three runs, complete commands in Snow.
These are not physical SE results or cold-start measurements.

| Workload | 512 B | 8 KiB | 64 KiB |
|---|---:|---:|---:|
| wc-bytes | 0.88 / 0.90 | 2.40 / 1.65 | 13.80 / 7.33 |
| tr | 1.20 / 0.86 | 6.61 / 1.25 | 47.06 / 4.18 |
| wc-all | 0.86 / 0.88 | 2.40 / 1.66 | 13.81 / 7.33 |
| cmp | 1.08 / 1.16 | 2.25 / 2.28 | 10.81 / 10.55 |
| sort | 1.10 / 1.11 | 2.63 / 2.81 | 50.48 / 16.66 |
| sed | 1.63 / 1.55 | 6.98 / 7.06 | 47.33 / 48.08 |
| grep-fixed | 1.16 / 1.21 | 2.01 / 2.38 | 8.43 / 11.15 |
| grep-regex | 1.33 / 1.35 | 3.71 / 3.73 | 21.53 / 21.73 |
| cat | 0.88 / 1.00 | 0.95 / 1.08 | 1.95 / 1.86 |
| cut | 1.03 / 1.13 | 2.43 / 3.11 | 12.75 / 17.88 |
| head | 1.00 / 1.08 | 1.05 / 1.23 | 1.05 / 1.25 |
| tail | 0.96 / 1.03 | 1.11 / 1.26 | 1.98 / 3.01 |
| uniq | 1.00 / 1.06 | 1.83 / 2.30 | 8.03 / 11.53 |
| tee | 0.80 / 0.85 | 1.00 / 1.06 | 2.61 / 2.71 |
| cksum | 0.81 / 0.91 | 1.13 / 1.21 | 3.55 / 3.50 |

Path-only commands: basename 0.80 / 0.85 s, dirname 0.76 / 0.80 s.

### Decisions

- **wc and tr: user-approved replacements.** wc gains about 1.9x at 64 KiB,
  while tiny inputs are essentially tied. tr gains 11.3x at 64 KiB and remains
  faster at 8 KiB and 512 B. Preserve the documented byte/Unicode differences.
- **sort: user-approved replacement conditional on releasing temporary RAM.**
  3.0x faster at 64 KiB; slightly slower at 8 KiB and 512 B. Tracked peak is
  157324 B versus 25964 B. All measured runs release owned allocations; the
  loader also disposes the ordinary command image after execution. The command
  still shares Clarice application memory under single Finder; no separate OS
  process or extra system RAM is created. No disk spill in this implementation.
- **Retain existing implementations for the other tested tools.** cmp/cat/cksum
  differences are small; sed/regex grep/tee are near ties; fixed grep, cut, head,
  tail and uniq favor existing code. No material pathname speed gain. BusyBox
  head offers -c as a feature gain, independently of its slower shared test.
- Keep approval separate from integration: candidates remain bb-prefixed in
  the private fixture. No standard package mapping has been changed.

### 64 KiB resource costs

| Workload | Loaded image existing / BB | Tracked working peak existing / BB |
|---|---:|---:|
| wc-bytes | 9800 / 14032 B | 1620 / 5716 B |
| tr | 25300 / 14576 B | 3668 / 6568 B |
| wc-all | 9800 / 14032 B | 1620 / 5716 B |
| cmp | 8508 / 14208 B | 4748 / 5716 B |
| sort | 30360 / 19968 B | 25964 / 157324 B |
| sed | 46260 / 42240 B | 13464 / 13180 B |
| grep-fixed | 32708 / 38304 B | 6824 / 6540 B |
| grep-regex | 32708 / 38304 B | 12996 / 12440 B |
| cat | 8104 / 13424 B | 0 / 5716 B |
| cut | 12240 / 15408 B | 4492 / 8588 B |
| head | 9708 / 13392 B | 4492 / 5716 B |
| tail | 11480 / 16672 B | 9868 / 9128 B |
| uniq | 10388 / 14512 B | 6540 / 6540 B |
| tee | 8392 / 14720 B | 3668 / 5716 B |
| cksum | 9572 / 14720 B | 1620 / 6768 B |

Raw first/repeat timings, loader ticks and observed heap sizes are retained in
measure.json and summary.json. Working peak excludes image, stack and shared
services; do not add it to image size and call that total process memory.

## Memory-pressure result (isolated fixture)

With approximately 356 KB application heap available before loading, BusyBox
sort could not complete the 256 KiB fixture: explicit `out of memory`, status 2,
17.16 guest seconds, tracked peak 300684 B, remaining 0. This is a limit imposed
by available working memory and the all-in-memory algorithm, not the CLX image
ceiling. Ordinary 64 KiB sorting completed and released its allocations.
The existing sort completed the same 256 KiB input in 263.40 seconds, status 0;
sorted-order and full 262144-byte length checks passed. A subsequent BusyBox wc
command also passed with zero remaining allocations. All 43 robustness
observations passed, including empty streams, pipeline output, redirected tail
and locked-file write failure without changing the locked file.

User approval is recorded, but this is a material capacity trade-off: retain
access to the existing disk-spilling sort during integration, rather than
silently removing the ability to sort inputs larger than working RAM. No new
spill implementation or automatic fallback has been added in this comparison.

## Checks and observed compatibility differences

Clean rebuild package hashes match the installed build; all 15 CLX image/import
checks pass. The regression harness completed 190 observations (including resumed
checks and intentionally nonzero exits), not 190 independent test cases. Binary
copy/checksum comparisons, option errors and source preservation, missing files,
read failures, normal regex operations and CPU-only sed cancellation were covered.
See regression.json and the separately retained initial-difference evidence.

- UTF-8 fixture: existing `wc -m` reports 20 characters, BusyBox reports 28 bytes.
  Existing word count is 4 versus BusyBox 2. Approval to replace wc/tr/sort is recorded,
  but these are explicit compatibility changes, not equivalent Unicode support.
- BusyBox tr deletion of NUL bytes matches an independent binary oracle. The
  existing tr invocation produces different output; both exit successfully.
- Existing head rejects `-c`; BusyBox head's 100-byte result matches the binary
  oracle. Shared performance comparisons use `-n`, supported by both.
- sed differs on a missing final newline, as described above. All timed shared
  text fixtures have final newlines.
- Long 64 KiB lines completed for candidate grep, sed and sort, with invocation
  allocations released. They are correctness/pressure observations, not matched
  long-line speed comparisons.

[Telnet findings](TELNET-FINDINGS.md) distinguish this older test fixture from
concurrent Telnet development. No new Telnet defect was identified in these checks.

## Reproduction

```
scripts/build-busybox-tools.sh
python3 tests/busybox-tools/publish.py NEW_VERSION
# Private repository must contain exactly one unchanged Core entry.
# Local guest: clarice update -y --source PRIVATE_URL/index.txt
PYTHON39 tests/busybox-tools/guest.py setup PASSWORD_FILE
PYTHON39 tests/busybox-tools/guest.py smoke PASSWORD_FILE
PYTHON39 tests/busybox-tools/regress.py PASSWORD_FILE
PYTHON39 tests/busybox-tools/guest.py measure PASSWORD_FILE
PYTHON39 tests/busybox-tools/robustness.py PASSWORD_FILE
python3 tests/busybox-tools/summarize.py
```

This work is experimental; decisions require correctness/feature parity as well
as complete-operation speed. Physical hardware, MultiFinder/System7 and broad
production option conformance are separate acceptance work.

## Completion and next step

Private Telnet listener disabled locally, Clarice quit to Finder and this Snow
workspace closed. Private update HTTP server stopped; test disks and package
artifacts retained for reproduction. Other agents' instances and physical SE
were not used. Changes are confined to the separate comparison branch; no merge,
push or publication.

Recommended integration: use the approved wc/tr/sort candidates with explicit
compatibility documentation; retain the current spill-capable sort as an
accessible fallback, and retain the other tested utilities. Adapt standard
packaging/help and run focused replacement regression before calling integration
complete. Physical SE acceptance and current-Core verification remain outstanding;
the latest concurrent Telnet/Core work was deliberately not replaced in this test.
