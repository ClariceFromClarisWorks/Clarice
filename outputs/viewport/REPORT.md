# Terminal viewport and rendering investigation

17 November 2026. Development branch `codex/terminal-viewport`, based on RC
`2df1684`. **Work in progress: no final visual acceptance or release promotion.**
The release candidate remains unchanged. The user permits up to 100 KiB of
additional rendering memory where measured benefits justify it.

## Correctness fixes

- **Viewport and prompt:** partial output now remains visible before the editable
  prompt. Height-only resize preserves the history anchor: **76→81 before,
  76→76 after**. Width changes preserve positions inside long/multiline input;
  allocation failure retains the previous column model.
- **Scrollbar:** a clipped update could clear staging pixels without redrawing
  them. Full offscreen control composition is now separate from clipped screen
  presentation. The focused Snow reproduction changes **FAIL→PASS**.
- **Pagination:** automatic paging is removed from preferences, menus and help;
  obsolete saved settings are ignored. Explicit `more` remains available, with
  interactive advance/quit/cancel acceptance passing in Snow.
- **Preferences on another HFS volume:** testing exposed `PBHGetVInfo` index zero
  selecting the default volume during rename. Source-volume binding and
  destination-volume flushing are now explicit. Host tests cover duplicate IDs
  and names, spaces/case, recovery and failed resolution; a negative control
  detects a successful rename on the wrong volume. Snow theme/input/history
  checks pass. This is not full filesystem or updater acceptance.

Evidence: [before](viewport-check.txt), [after](viewport-v2.txt),
[host viewport](viewport-host.txt), [native rename](rename-host.txt),
[Snow usability](usability.txt).

## Matched redraw results

Each value below is the median total guest ticks for **24 completed large-jump
draws**, from three trials. Monaco 9, 80 columns, 24 history rows. Fixture creation
and report I/O are excluded. Baselines are paired within each executable; numbers
from separate builds must not be treated as one continuous comparison. Nested
phase counters overlap and cannot be added together.

Snow uses the Plus/4 MB/System 6.0.8 fixture at fixed acceleration. Physical results
come from the stock 68000 SE/4 MB; SEControl activity at some yield boundaries may
influence samples. Host elapsed time is not substituted for guest timing.

| Build / environment | Dense light, ticks | Dense dark, ticks | Outcome |
| --- | ---: | ---: | --- |
| **1703, physical SE** | 646→557→514 | 708→627→579 | Original→glyph planes→planes plus row cache. Throughput improves, but user sees progressive top-down repaint: **visual acceptance fails**. |
| **1704, Snow** | 486→548 | 548→609 | Snapshot, row composition and final whole-view copy duplicate work. **Rejected for slowdown; not deployed.** |
| **1705, Snow** | 481→436 | 543→565 | Direct composition helps light mode, but dark transfer and sparse fallback regress. Intermediate only. |
| **1706, Snow** | 479→424 | 542→533 | Early sparse rejection and RAM inversion remove major regressions. C inversion remains expensive. Intermediate only. |
| **1707, Snow** | **483→422** | **543→463** | Bounded 68000 inversion loop: **12.6% / 14.7% faster dense redraws**. Intermediate prototype; not physically or visually accepted. |
| **1708, Snow** | **479→425** | **544→465** | Faster sparse gate; **11.3% / 14.5% dense gains**, listing medians 249→249 / 290→291. Physical comparison below. |
| **1708, physical SE** | **554→490** | **620→537** | **11.6% / 13.4% dense gains**; listing283→281 /327→328. Final visual acceptance pending. |
| **1709, Snow** | **475→389** | **537→429** | Eliminate repeated per-row validation after whole-view validation. Composition227–230 ticks; physical comparison in1710 below. |

| **1710, physical SE** | **549→447** | **613→494** | **18.6% / 19.4% faster**; same1709 renderer, manual diagnostics; visual acceptance pending. |

Raw evidence: [1703 physical](physical/dense-transcription.txt),
[1703 Snow light](snow-1703/light.txt) / [dark](snow-1703/dark.txt),
[1704 light](coherent-1704/light.txt) / [dark](coherent-1704/dark.txt),
[1705 light](direct-1705/light.txt) / [dark](direct-1705/dark.txt),
[1706 light](direct-1706/light.txt) / [dark](direct-1706/dark.txt),
[1707 light](direct-1707/light.txt) / [dark](direct-1707/dark.txt).

1708 replaces the bytewise sparse gate with conservative wordwise prefix rejection.
17,290 host sanitizer and 17,290 actual-68000 cases pass. Listing differences are
now within trial variation. [1708 raw light](direct-1708/direct-stage-light.txt),
[dark](direct-1708/direct-stage-dark.txt), [pixels](direct-1708/direct-stage-pixels.txt)
and [scrollbar/scrolling](direct-1708/native-scroll-report.txt) preserve the results. The direct path is
limited to substantially changed ASCII Monaco history views. Selection, editable
rows, other fonts/characters and unsupported display geometry retain the native
fallback. No PowerBook acceleration claim is made.

## What the measurements show

1703's glyph planes reuse pre-shifted glyph contributions; its row cache also
reuses previously composed history pixels. Together they reduce physical dense
redraw time by roughly 20% light / 18% dark, but still publish rows individually.
That explains why faster completion alone did not satisfy the user's UX request.

1704's snapshot costs about 25–26 ticks per 25 staged frames, followed by another
34–43 ticks for final transfer. Removing both the snapshot and per-row copies
produced the direct-composition experiment. Dark `notSrcCopy` then proved costly:
about 159–160 ticks per 23 frames, versus roughly 36–38 for `srcCopy`.

1707 instead inverts composed bytes in RAM and uses `srcCopy`. Its inversion cost
is **37–38 ticks**, down from **104–109** in 1706; final transfer costs **33–34**.
Glyph composition still costs about **262–264 ticks** per 23 staged dense frames.
These phase measurements guide further work; pixel equality does not prove the
absence of visible flashes or raster tearing.

Earlier bounded experiments remain preserved: [glyph planes](planes-light.txt)
([dark](planes-dark.txt)), [row cache](cache-light.txt)
([dark](cache-dark.txt)), and [unrolled glyph loop](unroll-light.txt)
([dark](unroll-dark.txt)). They are historical evidence, not separate acceptance.

## Memory and validation boundaries

The glyph table requires **24,320 B**. Direct presentation adds **21,888 B**,
for **46,208 B payload plus Handle bookkeeping**; its history-row cache is off.
1703's additional row cache instead brings experimental capacity to about 90 KiB.
All are derived rendering data, not authoritative terminal state.

Explicit cache disposal in the later probes leaves **+26 B light / 0 B dark** free
heap change. Earlier negative deltas describe retained cache capacity, not proof
of a progressive leak. Conversely, purgeable Handles alone do not establish safe
command admission: runtime preflights inspect `FreeMem` before allocation.
1708 uses native purge only when a runtime admission preflight would otherwise
refuse; optional rendering allocations preserve 32 KiB headroom without purging.
[Snow pressure results](direct-1708/heap-admission.txt) pass unlocked reclamation,
locked refusal, exhaustion and recovery: zero owned allocations, free heap +12 B,
largest block −1,864 B. This does not establish unchanged fragmentation or full
application-pressure acceptance. Host tests exercise the actual `cli_alloc` path.
Local allocation-failure tests are not a substitute for full pressure acceptance.

Completed focused checks:

- Host viewport sanitizers: partial output, canonical reads, wrapping, long input,
  cursor ownership, resize anchors and recoverable reflow allocation failure.
- [1707 Snow pixels](direct-1707/pixels.txt): **35 comparisons, zero failures**,
  covering clipping, occlusion, themes, selection fallback, dirty storage, purge
  and local allocation failure.
- [1707 scrolling](direct-1707/scroll.txt): **22 bursts + 20 views**, zero pixel
  mismatches; **149 scrollbar checks**, zero mismatches.
- [Inversion guards](direct-1707/invert-guards.txt): **140,039 host sanitizer cases
  and 3,518 emulated 68000 machine-code cases**, including exact access bounds and alignment.
- [Direct-output guards](direct-1705/m68000-guards.txt) and earlier
  [glyph-plane oracle](planes-host.txt) preserve independent pixel/bounds evidence.
- Snow usability reports zero failures for clipboard, typeahead, control-key
  dispatch, immediate themes and returning to the live view.

The saved [selection report](qol3a-selection.txt) is incomplete/overlapping; it is
not sufficient for full selection acceptance. Snow Command-period/Option-C and explicit `more` cancellation/recovery pass. Nano
edit/discard/reopen restores terminal/status0, with stable nativeheap355234 and
largest351620, tracked remaining0. See [interactive evidence](direct-1709/interactive/nano-results.txt). 1710 physical Control/Command-period cancellation and Snow Telnet restoration now pass with the limits in [1710 report](direct-1710/README.md). Still required: broader application-pressure tests, command/output
throughput checks, rapid-scroll visual inspection on the SE, and PowerBook
180/System 7.1 resize checks. Physical **1708** matched results are collected in [timings and screenshots](direct-1708/physical/timings.txt). The final human visual comparison remains pending.

Measured helper snapshots and build hashes are retained in each versioned evidence
directory, with the earlier [diagnostic overlay](probe/README.md) separately
labelled. No prototype is promoted to the release candidate by this report.

[1709 focused results and limitations](direct-1709/README.md).
