# Directory listing and real scrollbar drag — 15 November 2026

All work used Snow, Macintosh Plus/4 MB/System 6.0.8, Monaco 9, 80 columns,
26 history rows. The previously verified 6.0.8 image reports API version 0607.
Existing patched Snow supplies functional modifier keys. No physical SE access,
updates, push or publication. No production renderer or loader validation changes.

## What makes ls slow outside drawing

The native diagnostic utility measured three 100-file listings. Catalog-only
passes took 44/46/45 ticks: path setup 9/10/10, catalog enumeration plus name
encoding 33/33/32; event handling across all passes was four ticks. Detailed
listing took 138/106/106 ticks excluding command dispatch/loading and final
presentation. Its per-entry formatting was 10/7/13 ticks, terminal emission
8/11/7, catalog bucket 40/30/32. The setup bucket also includes `.`/`..` and
associated presentation; yield includes painting. Do not add those inclusive
buckets to separately measured paint totals. Counters use the 60 Hz guest clock,
with instrumentation overhead and tick quantisation.

Thus catalog access is the largest measured per-entry non-paint cost, not date
formatting. The listing already streams indexed HFS catalog records without a
preliminary scan or directory-sized allocation. The current native creation-date
and decimal routines were retained. No persistent directory cache was added.

A final normal-build catalog-only scan of 1,000 entries took 364/338/336 ticks
(6.07/5.63/5.60 s), with first entries at 11/12/12 ticks, zero paint ticks
and 11 event ticks across all three runs. See scan1000.png. This confirms a
substantial enumeration cost even when no listing is drawn; it does not separate
native HFS service time from name conversion within that scan.

The existing normal ls loader took 35 ticks: read 10, receipt work 12, executable
metadata validation 5, imports 1, relocations 1, prepare 6; event time overlaps
these phases. Loading is separate from command selection and listing execution.
See scan100.png, list100.png and ls-runtime.png.

## Retained improvement: eliminate redundant command selection work

Command selection already resolved the executable and read its header, then
repeated the path resolution while opening it and again resolved/opened it to
rediscover the same executable type. Pass the already resolved native path into
header recognition and return the type to dispatch. No resident cache, no stale
cross-invocation state, no skipped CLX structural/runtime/integrity checks. The
loader still opens and validates the executable normally. Completion callers
retain the same selection contract with an optional type output.

Same diagnostic executable, warmed commands, seeded output, native renderer,
three complete invocations per case including final presentation:

| Files | Before ticks | After ticks | Median seconds before → after |
|---|---|---|---|
| 10 | 123, 118, 117 | 107, 108, 108 | 1.97 → 1.80 (8.5% shorter) |
| 100 | 174, 172, 173 | 163, 165, 162 | 2.88 → 2.72 (5.8% shorter) |

Selection/type work fell from 21–22 to 10–12 ticks; loader stayed 33–35 ticks.
Free heap was exactly 422060 after every measured invocation in this diagnostic
build. [Raw counters](dispatch-matched.txt). These are warm same-build comparisons,
not cold startup measurements or cross-emulator comparisons with older reports.

## Actual mouse dragging

Host HID mouse down/drag/up events moved the real scrollbar. Capture compared
terminal pixels across the viewport, excluding the scrollbar and mouse pointer,
through 700 ms after each step. Five of six steps in each case produced measurable
pixel changes; the first step was not observed, even with a one-second hold.
That remains an automation/input-delivery limitation to investigate, not a passed
first-motion latency result. Initial narrow capture missed changes because the
varying filename suffix was outside the crop; those null results are not timings.

| Renderer | First observed text change | Last observed text change | Median last change |
|---|---|---|---|
| Native | 94–144 ms | 250–298 ms | 286 ms |
| Cached assembly | 167–199 ms | 195–254 ms | 214 ms |

These include host event delivery, Snow display and capture latency. Sampling
interval was roughly 45–55 ms, not a precise frame timestamp. Native drawing
appears progressively; first changed pixels are not a completed redraw. Guest
instrumentation separately measures repaint work, with no allocation retained.
See drag-*-full-host.json and drag-*-full-report.png. Pauses between steps allow
settling; these measurements do not establish continuous-drag throughput.

The result is below the historical one-second update interval but still visibly
slow. No speculative drag throttling or heuristic change was retained. The next
renderer investigation should compare small multirow scrolls with patching all
visible rows; the current heuristic may underprice per-row native Toolbox costs.
That is a hypothesis, not a measured fix.

## Validation and build

- ASan/UBSan production command-selection harness: 15 cases pass, including PATH,
  explicit paths, spaces, symlink resolution, scripts, malformed/non-executables,
  missing files and closed descriptors. First PATH match uses one resolve/open.
- Existing HFS volume tests pass: explicit volume refs and fail-closed ambiguous,
  absent, invalid and error paths. Namespace resolution itself is unchanged.
- Normal native/cached pixel reference tests pass: each 24 bursts, 20 views,
  zero mismatches, 153 scrollbar staging/reference checks with no mismatches.
  See native-scroll.txt and cached-scroll.txt. The unchanged renderer's previous
  dark-theme acceptance remains evidence; not rerun here.
- Initial guest fixture named a core multicall alias `dispatch-link`; that command
  returns 127 because its basename is not one of that utility's supported names.
  The corrected symlink test retains basename `ls`. This is not a resolver failure
  and no new arbitrary multicall aliases were added. Corrected normal guest script
  passes PATH, absolute CLX, symlink, extension/shebang script, missing/invalid
  executable and truncated CLX rejection checks. See normal-dispatch-pass.png.
  The first 15-second capture preceded completion; an isolated 40-second run
  completed. The overlapping cancellation batch is not acceptance evidence.
- Final isolated Ctrl+C checks pass with native/cached rendering and paging
  off/screen: all four show ^C, status 130 and a restored prompt. See
  verified-cancel-*-status.png. Host wait intervals are not interrupt-latency
  measurements. Final runtime reports zero tracked allocations remaining,
  425630 bytes free heap (normal-final-runtime.png).
- Normal build 2026111510 compiled. Resource fork 938831 bytes, +99 versus 1509;
  no new production heap/static cache. Diagnostic counters compile out normally.
  Temporary instrumented benchls/benchscan were restored to normal guest commands.
  A normal rebuild of core.clx is byte-identical to its pre-instrumentation image.
  [Normal experimental build](experimental-build/Clarice.bin).
  SHA-256 `6e29aab35b56120d0185692d1c1f78c125c1b05b9a8214669b10bb8efb39e4dd`.

For planning only, SE-at-60%-of-Snow-speed implies the new 100-file listing would
be about 4.53 s; native drag's median final change about 477 ms. Host capture latency
and HFS timing need not scale uniformly. These are not hardware measurements.
No nano, safe-save, serial, updater-error-28 or physical installation changes.
