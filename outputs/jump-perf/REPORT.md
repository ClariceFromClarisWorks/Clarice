# Large history jumps — 15 November 2026

Focused continuation of build 1515. Normal build **2026111517** is installed and verified on the physical SE. No push/publication.

## Change and scope

The new Monaco 9 path reuses bytes 704–3743 of the existing 4096-byte row staging
bitmap for the installed system font's 95 ASCII glyphs. Two native DrawText calls
prepare it on first use. A bounded 68000 compositor packs four six-pixel cells
into three bytes, reducing repeated QuickDraw text work. Completed row spans
still use the existing clipped presentation path and separately staged scrollbar.

No new bitmap allocation, partition increase, rendering preference or OS hook.
One validity flag is added. Font changes and editor ownership invalidate the
cells. Unsupported fonts, sizes and extended characters retain QuickDraw;
selection, prompts/cursors and forced full redraws retain the original path.
The independent full redraw is also the pixel-test oracle. No selection, cursor,
HFS, cancellation, bulk-output cadence or scrollbar tracking changes.

## Controlled Snow results

Plus, 4 MB, System 6.0.8 media, Monaco 9, 80 columns × 24 rows, light theme.
Three repetitions alternate old/new raster paths inside one diagnostic executable.
Each case paints 24 large history jumps; seeding and report I/O are excluded.
Cold glyph preparation is included; no host input was sent during the clean run.

| Workload | Before | After | Less draw time |
|---|---:|---:|---:|
| Directory-style history, 24 jumps | 331 ticks / 5.52 s | 249 ticks / 4.15 s | 24.8% |
| Dense 80-character history, 24 jumps | 1,071 ticks / 17.85 s | 645 ticks / 10.75 s | 39.8% |
| Mean directory-style frame | 230 ms | 173 ms | |
| Mean dense-text frame | 744 ms | 448 ms | |

Worst dense frame improves 46 → 27 ticks (767 → 450 ms). Worst directory-style
frame remains 15 ticks: cold cell preparation costs four ticks (~67 ms) once per
invalidation. Retained heap difference is zero. Phase counters are quantised to
60 Hz and overlap; use complete draw totals, not sums of subphases.
[Raw paired results](snow-paired.txt), [structured measurements](measurements.json).

Actual step-and-pause thumb dragging after the real 1,000-file listing: five draws,
55 paint ticks, worst/detected-to-painted 11 ticks, no retained allocation.
Prior build 1515 recorded six draws, 79 ticks, worst 14. The automated gestures
covered different view ranges (104→67 versus 104→60); this is not a perfectly
matched latency comparison. Host final-change median 240 ms versus 246 ms is
within the ~50 ms capture granularity. The first step was undetected in both.
Do not claim this establishes fluid continuous dragging.
[Gesture samples](drag-listing.json), [guest counters](drag-listing.txt).

## Focused validation

- 356 actual M68000 compositor cases: both old and ASCII entry points, widths 1–8,
  heights 1/11/32, every Monaco row length 0–85, bounds and preserved registers.
- 510 production glyph-builder host cases with ASan/UBSan: all ASCII glyphs,
  both themes, widths 1–85, invalidation/reuse, unsupported-input fallbacks and guards.
- Existing row comparator: 532 cases; selection delta and 1,560 cursor/view guards pass.
- Snow dense ASCII plus extended-character fallback, light and dark: each passes
  22 bursts + 20 views against forced native redraw, zero text mismatches, 147
  clean scrollbar checks. Normal quick check passes with four stable allocation/
  release cycles. Normal native 30-frame run: 138 ticks (prior 1515: 157 ticks).
- Nano opens the existing 8 KB fixture and exits; subsequent native pixel tests
  pass (22 bursts + 20 views, 147 scrollbar checks). Sampled heap 425,060 bytes,
  invocation-owned remaining zero. This tests buffer handoff, not nano startup speed.
- Normal Ctrl+C during listing and paging: status 130, usable prompt. Mouse selection
  spans lines correctly, sampled changes 70–109 ms including host capture overhead.

## Limits

The improvement is specific to the usual Monaco 9 text view. Dense history is
still expensive; this is not completion of the wider terminal-fluidity objective.
Per-row setup, composition and native presentation remain measurable costs.
The existing two-tick post-paint drag cadence is unchanged. No new nano startup
or filesystem throughput claim. Previously recorded dark-to-light preference
invalidation issue and updater Y/n/-y follow-up remain separate outstanding work.

## Physical SE measurements

Stock 68000 mode confirmed by the user, 4 MB/System 6.0.8/MultiFinder, same
80×24 Monaco 9 configuration. Three paired runs in diagnostic build 2026111516;
SEControl quiet during measured frames. Results transcribed from the guest report.

| 24 large jumps | Before median | After median | Less draw time |
|---|---:|---:|---:|
| Directory-style history | 370 ticks / 6.17 s | 286 ticks / 4.77 s | 22.7% |
| Dense text | 1,159 ticks / 19.32 s | 731 ticks / 12.18 s | 36.9% |

Mean frame: directory-style **257 → 199 ms**, dense **805 → 508 ms**. Worst dense
frame **50 → 31 ticks** (833 → 517 ms). Directory-style worst 17 → 18 ticks
includes first-use preparation (4–5 ticks); warm cases max 17. Heap delta zero.
This workload is 13–15% slower on the SE than Snow after the change; do not apply
that ratio to unrelated workloads. [Hardware records](physical-measurements.json),
[all runs on screen](se-paired-summary.png), [phase counters](se-paired-tail.png).

Physical dense ASCII/extended-character pixel comparison passes: 22 bursts + 20
views, zero text mismatch; 147 scrollbar checks, zero mismatch. This was measured
in the diagnostic build before installing the normal application.

Normal **2026111517** downloaded through the existing private updater, replaced
the profiling application and automatically relaunched. Version check confirms it.
Actual history-thumb dragging moves the view and leaves track/thumb/arrows intact;
selection spans lines correctly. Ctrl+C during a listing and at the pager returns
130 and a usable prompt. All remote mouse operations release the button.
Physical drag latency/flicker was not measured with high-speed video; pixel checks
and completed screenshots are the available evidence, not a universal guarantee.

Normal sampled free heap **424,878 bytes (414.92 KB)**, owned remainder zero.
Prior 1515 after-listing sample: 425,746 bytes; difference **868 bytes**, consistent
with small code growth, not a new allocated bitmap. No new idle drawing work.

The previous normal app remains `Macintosh HD:.cu-000005db-78c4021c.old`
(resource 938,856 bytes, matching the previous artifact's fork length); diagnostic
1516 remains `.cu-0000063c-78c4021d.old` (949,013 bytes). Earlier recovery copies
also remain. No new error 28 observed. Existing unpublished work is preserved.
Snow test volume now contains normal 1517; original preferences restored and the
temporary Bench application removed. The unrelated Snow instance was untouched.

## Files and build

Implementation: `src/app.c`, new `src/native-glyphs.inc`, `src/kernels.S`.
Diagnostic-only dispatch/fixtures: `src/shell.c`, `tests/terminal/jump-guest.inc`.
Tests: `tests/terminal/native-glyphs.py`, `tests/terminal/glyph-68k.py`, reused
selection/scroll-guard/row tests. Private stamp: `src/update/version.h`.
Existing unrelated working-tree changes were preserved.

[Normal build 2026111517](experimental-build/Clarice.bin): 939,904-byte MacBinary;
data fork zero, resource fork 939,714 bytes (**+858 bytes versus 1515**). Same
application partition; no added bitmap allocation. SHA-256:
`bd6de3793030845a612200a86f6871be14e53454748a52f7d6762e7e503b9b7a`.
The diagnostic 1516 artifact is retained solely to reproduce paired measurements.

## Next focused opportunity (checkpoint review)

No further rendering change was made for this review. Existing physical phase
counters are visible in [the original capture](se-paired-tail.png). For the three
new-path dense runs (24 jumps each), the following ranges describe mean frame cost:

| Counter | Ticks per 24 jumps | Mean per frame | Interpretation |
|---|---:|---:|---|
| Complete redraw | 728–732 | 506–508 ms | Authoritative end-to-end draw cost |
| Compose | 16–21 | 11–15 ms | History traversal and logical screen composition together |
| Plan | 6–9 | 4–6 ms | Row comparison and changed-span planning |
| Prepare | 52–62 | 36–43 ms | Per-row drawing state/character preparation |
| Text | 338–344 | 235–239 ms | Eligibility, buffer clearing, glyph composition and theme work |
| Copy | 223–239 | 155–166 ms | Clipped row presentation into the window/screen |
| Scrollbar | 45–64 | 31–44 ms | Separate scrollbar maintenance |

Counters are 60 Hz, some nested, and short phases suffer tick-boundary aliasing;
do not sum them or treat small differences as precise attribution. Raster totals
(658–665 ticks) include row subphases. No separate history-versus-layout or
clear-versus-glyph timings exist. Normal misaligned window rows use CopyBits;
the copy counter includes its clipping/presentation overhead, not just raw VRAM
bandwidth. Text is roughly 47% of physical redraw; presentation roughly 32%.
Snow's text/copy split differs despite similar total trends, so hardware counters
take precedence. There is no HFS catalog traversal in these history jumps.

Recommended next experiment: isolate row clearing from glyph composition, then
remove unnecessary clearing if the measured saving warrants it. Every eligible
row currently clears 704 bytes. The six-pixel, four-character compositor then
overwrites three complete bytes per scanline, independent of their previous
contents. A full 80-character row has no remainder; only shorter rows' 1–3
remaining glyphs need the existing OR compositor's destination cleared. The
569-row benchmark clears 400,576 bytes (391.19 KB) before composing its text.
Most of those writes appear avoidable without a new allocation or new assembly.
Code outside the transferred span must not depend on the current zeroed padding.

Expected benefit is less work in the dominant text phase, with no persistent
memory increase. The clearing fraction is unmeasured: no honest millisecond or
percentage prediction is available yet. It cannot remove the separate ~160 ms
presentation cost. If clearing is cheap, investigate measured CopyBits setup and
clipping next; do not add a larger frame buffer speculatively.

Verification: accumulate clearing/composition costs over whole matched batches
to reduce tick quantisation; alternate old/new runs at identical view positions.
Test nonzero prior buffer contents, lengths 0–85, all ASCII, remainder lengths,
both themes, extended-character/font fallback, partial spans and cache/editor
handoff. Keep the independent QuickDraw pixel oracle, scrollbar guards, selection
and Ctrl+C tests. Compare Snow first, then the stock SE with quiet SEControl;
check both warm and first-use totals and unchanged heap. Stale remainder bits or
damage to the glyphs sharing the same buffer are the main correctness risks.
