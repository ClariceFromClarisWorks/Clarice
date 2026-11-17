# Nano startup bottlenecks — 14 November 2026

Focused continuation on Snow Plus / 68000 / 4 MB / System 6.0.8, same disk,
font, fixtures and speed settings. Genuine nano 9.2 tiny and ncurses 6.5 retained.
No physical acceptance, publication or push. Three-second target remains unmet.

## Root cause and selected improvement

The extra file cost is largely native allocation, not bulk file I/O. Before this
change, the 8 KiB run made 540 native portable allocations versus 295 for empty.
NewPtr accounted for 240–255 ticks versus 77 on repeated empty startup; FreeMem
checks took only 6–9 ticks and all 17 data reads together took five ticks. The
byte-reading/line-construction loop alone took 245–255 ticks. Thus a rewrite of
nano's reader or a larger disk buffer would not address the measured dominant
avoidable cost. Existing per-stage markers were retained, adding only loop and
native-operation measurements.

Cold cost is also mainly native allocation. The new loader split attributes a
first-launch preparation sample to image NewPtr (89 ticks) and relocation/import
workspace NewPtr (31), versus zero for those calls on repeat. Newlib FILE-pool
preparation itself takes only two ticks, not the whole preparation interval.
Further temporary allocations in ncurses/nano account for additional cold cost.
This identifies the native calls, not a verified ROM-level explanation for why
their first allocation is slower. No claim of disk-cache or preloading benefit.

Implemented one shared-runtime change: small allocations use invocation-owned
2048-byte pages with 64/128/256-byte slots including their bookkeeping. Larger
requests keep the native path. Pages preserve alignment, recycle freed slots,
return immediately when empty, and release all live blocks at exit/cancellation.
They are not also retained in the existing spare-block cache. If a page cannot
fit, the exact-sized native allocation is still attempted. The existing native
32 KiB emergency margin remains enforced. No resident arena, preloading, upstream
algorithm replacement, checksum bypass, filesystem or rendering change.

## Controlled timings

| Dispatch to painted input wait | Before | Final |
|---|---:|---:|
| Cold empty, first nano after boot | 15.18 s | 13.67 s |
| Repeat empty | 12.17 s | 11.15 s |
| First 8 KiB after empty runs | 17.92 s | 14.23 s |
| Repeat 8 KiB | 17.68 s | 14.07 s |
| Cold 8 KiB, separate boot | 20.92 s (prior report) | 16.77 s |

Repeat 8 KiB improves 217 ticks / 3.62 s (20.5%). The prior report's slightly
lower 11.98 / 17.50 s baselines used fewer diagnostic hooks and a different heap
layout; the first four rows use this task's controlled before/final records.
The final separate cold 8 KiB launch repeats at 14.02 s. Its cold stage values
are loader 555 ticks (63 read, 331 validation, 17 imports, 22 relocations, 122
preparation), runtime 0, ncurses 41, file 151 (91 parsing), first paint 94.
The cold-8-KiB before value is historical, not a newly repeated matched-profile run.

| Stage, ticks | Before cold empty | Final cold empty | Before repeat empty | Final repeat empty | Before repeat 8 KiB | Final repeat 8 KiB |
|---|---:|---:|---:|---:|---:|---:|
| Loader total | 546 | 555 | 430 | 432 | 430 | 433 |
| Image/header reads | 62 | 63 | 62 | 63 | 61 | 64 |
| Validation | 330 | 331 | 329 | 329 | 329 | 330 |
| Imports | 18 | 17 | 17 | 17 | 17 | 17 |
| Relocations | 21 | 22 | 22 | 22 | 22 | 22 |
| Preparation | 115 | 122 | 0 | 1 | 1 | 0 |
| Runtime entry to upstream | 2 | 0 | 1 | 1 | 1 | 0 |
| Native screen / ncurses initscr | 72 | 41 | 28 | 27 | 28 | 26 |
| File open / construct | 90 | 85 | 92 | 87 | 308 | 141 |
| Parsing loop (included above) | 27 | 0 | 0 | 0 | 245 | 54 |
| Main-loop entry to first paint/input wait | 8 | 8 | 8 | 8 | 95 | 95 |
| Total | 911 | 820 | 730 | 669 | 1061 | 844 |

Unlisted time is shell dispatch and nano's setup between markers. Loader events
are included, not additive (repeat 8 KiB: 17 ticks before / 25 after). New runtime
code grows the image by 392 diagnostic bytes, with a small loader cost.

The measured cold-empty penalty falls from 181 to 151 ticks (3.02 to 2.52 s).
Final cold preparation is 122 ticks versus one on repeat; temporary NewPtr time
is 50 versus 18. These two native-allocation differences account for essentially
all of that penalty within sampling variation. The earlier ~3.3 s observation is
consistent with this allocation-sensitive first-launch cost, not evidence for a
permanent preload cache.

The extra document cost falls from 331 to 175 ticks (5.52 to 2.92 s). Remaining
contributors include 54 ticks extra file setup/parsing, 87 extra ticks to paint
the populated screen, and nano work between markers. Rendering and cursor code
were not changed.

## Memory tradeoff

Portable peak is 78,548 bytes empty (previously 68,755) and 105,172 bytes for 8 KiB
(previously 83,748): +9,793 / +21,424 bytes. This includes whole allocated pages,
slot rounding and bookkeeping, rather than hiding unused slot capacity. The normal application payload is byte-identical to the preceding build
(only MacBinary dates/header checksum differ). No new
idle allocations: these pages and their list live in the external command's
invocation. The first tiny allocation in a size class may cost a 2 KiB page;
large requests remain exact-sized. Direct spare blocks retain their prior 8 KiB
limit; pooled blocks bypass that cache so they cannot pin otherwise empty pages.

Every final launch/cancel run has remaining=0 and the same free heap 397,102,
largest block 392,616. These diagnostic values differ from before by 106 bytes
of app instrumentation/code layout, not a per-run growing allocation. Native
screen/stdio/HFS allocations remain outside the portable peak counter.

## Validation boundaries

Host ASan/UBSan: actual page allocator, 50,000 mixed operations with content and
alignment checks, native/owned accounting, exhaustion, exact-size fallback and
page release. Actual shared runtime: 20 repeated allocation/reallocation workloads
with live blocks deliberately left for cleanup, 64 KiB constrained budget,
4 KiB clean allocation failure, and cancellation. Existing demand-allocation and
binary update-stream/seek/descriptor tests pass. Three upstream grep smoke cases
also pass with the updated runtime. These are focused tests, not all utilities.

The implementation is shared source for ports; this task relinks nano. Existing
bundled command images have not been broadly rebuilt or silently replaced.

Snow final-policy evidence: all six measured editor runs accepted a typed `z`,
then cancelled with zero remaining allocations and stable heap. Forty Down events,
Option-W search, typing a known prefix and Option-O saves were exercised. Both
normal editor exits returned 0. Saved 8 KiB contents match the expected prefix and
every original byte; the resource-bearing fixture matches expected data, original
resource bytes, type/creator, Finder flags and creation date. No transaction
siblings remain. Loader cancellation returns 130 with unchanged heap. See
validation.json and screenshots. Nonzero Finder placement is not newly verified.

The 8 KiB safe save took 123 ticks (2.05 s), small resource-bearing save 87 ticks
(1.45 s); these intervals include the existing safe-replacement service. Mixed
input run: 64 paint batches, max 62 ticks including saving; single test character
paints in 4–6 ticks. No cursor or drawing changes, new save semantics, atomicity
claim or physical-media guarantee.

## Files and reproducibility

- src/ports/alloc-pages.inc and runtime.c: allocator and existing ownership integration.
- src/port-files.inc, module.inc, shell.c: opt-in native timing counters only.
- scripts/patch-nano-perf.py: existing markers 9/10 now bracket read_file's loop.
- tests/ports/alloc-pages.c and allocation-workload.c: focused regression fixtures.

All native counters are conditional on CLARICE_NANO_PERF; normal app behavior and
idle allocations are unchanged. Raw records, screenshots, build logs and host
results are in this directory; the disposable Snow workspace is work/nano-startup.
Before records use the added profiling hooks but no allocator change. `after*`
records are an intermediate pool trial; use `final*` for the retained policy.

Timing units are guest ticks (60/s). Total starts at shell dispatch, as in the
previous report, excluding initial command-echo redraw; ready means the first
painted input wait, confirmed by typed text. Portable peak includes complete pages
and bookkeeping, not just requested bytes; it is not whole-process peak memory.

## Normal build acceptance

`build/Clarice.bin` and `build/nano.clx` (also Clarice-nano-experimental.zip) are
normal 68000 builds, all profiling disabled. Final Snow smoke test saved a known
prefix into the 8 KiB fixture, verified exact bytes, reopened it and exited twice:
status 0, owned remaining 0, free heap 398,284 and largest block 393,798 both times.
Owned peak 105,172; loaded image 198,288 bytes (+392 versus preceding normal nano).
Normal loader 545 ticks cold / 429 repeat. Detailed end-to-end stage times above
are from the diagnostic build, not inferred from these invocation lifetimes.

The preceding normal-build test recorded 398,290 free bytes: this run is six bytes
lower but stable across both exits. No growing retention observed; this is a
bounded cleanup test, not whole-process allocation tracing or long-term endurance.
The application payload is unchanged, and the allocator adds no resident page pool.
Licence, installation instructions and hashes accompany the build. Safe-replacement
semantics, data/resource forks and metadata handling remain unchanged.

## Remaining bottlenecks and review boundary

Validation remains 329–331 ticks (~5.5 s); image reading ~1.05 s, import/relocation
work ~0.65 s, and populated first paint ~1.58 s. Ncurses initscr is now ~0.43 s
on repeat. Even eliminating the remaining small-allocation cost cannot meet a
three-second target while the existing checksum alone exceeds it.

No further architecture change is implemented. Options needing separate review:
(1) a versioned command format with a measured faster integrity algorithm, while
continuing to validate legacy FNV images; or (2) a native application/worker
loading path as already discussed in the roadmap. Neither should be represented
as a proven three-second solution: file setup and first rendering also remain.
A narrow first-render profile can proceed independently of those format/backend
decisions. Do not skip validation, permanently preload nano, or change cursor
rendering to mask Snow artifacts.
