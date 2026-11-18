# ls output batching — 18 November 2026

User-authorised follow-up in `Clarice-Ls-Asm`; previous formatter/catalog changes
retained, other trees and installed utilities untouched. No push or release.

## Changes and rationale

Plain listing appends the optional directory slash and newline directly instead
of parsing `snprintf`, scanning its result and issuing two writes per line.
Terminal column rows also include their newline in one write; the row has one
extra byte for a full-width newline. Historical catalog order, slash/escaping,
103-byte explicit-operand display bound, metadata and option semantics remain.

Native-file output uses an optional 4 KiB invocation-local buffer for both plain
and detailed listings. This is ordinary bounded stream buffering, following the
existing chunked I/O approach used by cat; no new stream abstraction, dependency,
HFS parser, catalog cache or resident state. Four KiB is a transfer chunk, not a
limit on directory size or application working memory. The existing allocator
checks native contiguous space and recovery headroom; allocation failure uses
direct output. Terminal, session and virtual-device streams stay immediate.

The wrapper owns cleanup across every traversal exit. It flushes already produced
bytes on EOF, catalog failure and cancellation, then frees the buffer; an output
failure is not retried. Cancellation retains status130; other output failures
return1. Per-entry event checks, first terminal presentation and paging remain.
The final flush can delay discovery of a file-output error, as with ordinary
buffered stdio; the command still reports it before returning.

The preceding catalog investigation supplies the native API research: classic
PBGetCatInfo has no selective-field switch; returned catalog records already
contain the sizes/dates. This patch targets application-side repeated work.
System7 CatSearch and shared resolver changes remain outside this patch.

## Host and compilation evidence

[Sanitizer comparison](host.txt): 576 exact before/after byte/status cases using
the real listing and rendering code: plain/long/full, one-column/multicolumn,
20/80/132 columns, paging, cancellation, catalog faults and allocation fallback.
Additional injected output failures verify failure status and no retry. Allocation
accounting asserts that the optional4096-byte buffer is freed on every exit.

Mock1000-file fixture plus dots: plain writes2004→3; detailed1002→13. These are
stream-call counts, not wall-clock speed or disk-sector counts. Real HFS still
performs its own catalog and disk-buffer work.

Existing4000 independent formatter cases,176 resolver cases, native-catalog loop
and130-operand tests pass. [Format](format.txt), [resolver](resolver.txt),
[catalog](catalog.txt), [operands](operands.txt). Native68000 compilation with
warnings treated as errors and package generation pass: [build](build.txt).

## Resource costs

Compared with the preceding uncommitted catalog candidate, matching runtime1.10:

| Cost | Before | After |
| --- | ---: | ---: |
| CLX package | 12,744B | 13,176B |
| Loaded image | 8,164B | 8,480B |
| Loader relocation/import metadata | 4,532B | 4,648B |
| Optional file-output working buffer | 0 | 4,096B |

Image+316B, loader metadata+116B, package+432B. Loader metadata is temporary and
freed before command execution. Native allocation bookkeeping is additional to
the4096-byte payload. Terminal output allocates no buffer; no new idle storage.
The buffer is heap-allocated, not an extra4KiB stack reservation.
[Package measurements](packages.json), native disassembly retained alongside.

The existing `status runtime` owned counters cover port allocator reporting,
not direct `cli_alloc`; they must not be presented as a measurement of this buffer.
Host ownership checks and explicit native free paths establish cleanup, with guest
free-heap/largest-block observations providing a separate repeated-run check.

## Reproduction

`tests/ls-output/before-listing.inc` preserves the exact previous uncommitted
candidate. Run `python3 tests/ls-output/host.py` and
`sh tests/ls-output/build.sh` from this tree. The native build reuses the pinned
Retro68 image and current decimal/date helpers; no Core rebuild is needed.
`tests/ls-output/benchmark.sh` runs disposable `/tmp/lo-before/ls` and
`/tmp/lo-after/ls` against the preceding100/1000-entry HFS fixtures. It explicitly
flushes the log volume outside the timed intervals before offline extraction.

## Normal-speed Snow results

Snow1.5 Plus/4MiB/System6.0.8, existing Core2026111882, same cloned HFS fixtures
and ROM as the preceding catalog trial. No fast-forward, Core rebuild, installed
utility replacement or physical-machine changes.36 timed commands, all status0;
16 in-guest comparisons pass. Timings include command loading, execution and
final buffer drain, using shell `time`; final prompt presentation and explicit
volume durability flush are outside the timed interval. They are not physical
SE acceptance measurements. The older `benchls` internal traversal timer excludes
the wrapper's final buffered drain; use shell `time` for complete command timing.

Repeated medians (three samples each; full samples in [summary](summary.txt) and
[measurements](measurements.json)):

| Redirected workload | Before | After | Median reduction |
| --- | ---: | ---: | ---: |
| 100 files, plain `ls -a1` | 2.15s | 1.56s | 27.4% |
| 100 files, `ls -la` | 2.05s | 1.86s | 9.3% |
| 1000 files, plain `ls -a1` | 11.16s | 6.46s | 42.1% |
| 1000 files, `ls -la` | 10.95s | 8.83s | 19.4% |

For1000 plain: before10.76–11.56s, after6.46–7.18s; long: before10.48–11.35s,
after7.65–9.40s. Timing variation remains, but the repeat ranges do not overlap.
These compare this change with the preceding catalog candidate, not with the
original pre-formatter baseline. Do not combine percentages across reports.

First runs per workload:100 plain2.30→1.63s;100 long2.11→1.95s;
1000 plain10.76→6.46s;1000 long9.50→8.20s. Only100/plain contains first package
launches; variants share OS caches and do not constitute equivalent cold boots.
One100-file terminal smoke run per variant: plain2.15→2.16s, long2.28→2.18s.
No repeat terminal speed claim; the main demonstrated benefit is file output.

Offline extracted long outputs are byte-identical:49,091B/1,002 lines.
Actual Ctrl+C on redirected candidate returns130 and leaves31,990B/653 complete
lines, an exact prefix of the complete output. This demonstrates the final partial
buffer is flushed on cancellation. [Output checks](output-validation.json),
[cancel status](cancel-status.txt), [cancelled output](cancel.txt).

File-phase post-command heap364948–365000B, largest block363476B after the first
baseline sample (363402B). No progressive4096-byte loss; candidate/baseline free
heaps stay within the same narrow range. This is post-command application memory,
not an all-system peak measurement. Runtime owned counters remain0 but do not
cover the direct native allocation, as explained above.

The script reached OUTPUT-BENCHMARK-DONE and recorded flush0. Following the
cancellation test, the volume was explicitly flushed again, Clarice quit to Finder
and the private Snow workspace exited. Disk handles were closed before extraction.
[Raw log](guest-results.txt). Other test sessions and the physical SE were untouched.

## Decision and next step

Retain the change in this isolated tree. The measured redirected gains justify
4KiB of temporary working storage, with direct-output fallback under memory pressure.
Terminal behavior and the System6 catalog path remain compatible in the tested
cases. Physical SE before/after runs, System7/MultiFinder and complete interactive
submission-to-final-prompt timing remain unverified. The next acceptance step is
matched physical SE redirected workloads; no further backend work is implied.
