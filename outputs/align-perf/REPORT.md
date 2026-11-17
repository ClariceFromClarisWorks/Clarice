# QuickDraw row alignment — 15 November 2026

Focused follow-up to normal build 2026111519. The existing 4096-byte drawing
buffer, QuickDraw clipping, glyph storage and terminal model are retained.
No new persistent allocation, bitmap, renderer preference or cursor change.

## Implementation and selection

For monochrome 64-byte destination strides and spans at least 128 pixels wide,
shift the staged source bits right within their existing rows so source and
screen offsets match modulo 16. Adjust only the source rectangle. Bounds and
size checks retain the previous path when there is insufficient room or the
geometry is unsuitable. Already aligned spans need no shift. Clipping still
uses QuickDraw; the previous fully-visible/even-aligned copier retains its
existing visibility and clipping checks.

The bounded 68000 loop consumes words backwards, combining adjacent words with
one register shift. It never reads before the first validated word. Complete
boundary words can change outside the source span; those pixels are not
presented. Glyph storage remains untouched for the 11-pixel Monaco row. Other
row heights stay inside the same 64-row port. No operating-system patches or
additional framebuffer path.

Two rejected variants are retained as measurement evidence, not production code:

- A straightforward C shift made Snow dense redraws **618 → 909 ticks**; rejected.
- The small assembly loop applied to every span improved dense redraws, but
  directory-style rows regressed **233 → 247 ticks**. Short changed spans already
  copy cheaply. The final policy retains the old path below 128 pixels; this is
  a conservative cutoff, not a claim of an exhaustively tuned crossover.

## Matched complete redraws

Three alternating pairs in each diagnostic executable, 24 large history jumps
per workload. Monaco 9, 80×24, light, 128 retained lines. Seeding/report I/O is
excluded; cold glyph preparation is included. SEControl heartbeats and captures
were suppressed during physical timing. CPU code 1; user-configured stock SE,
4 MB/System 6.0.8 (System API reports 0607).

| Workload | Before ticks | After ticks | Mean frame before → after | Change |
|---|---:|---:|---:|---:|
| Snow directory-style | 234 | 234 | 162.5 → 162.5 ms | unchanged |
| Snow dense | 617 | 588 | 428.5 → 408.3 ms | 4.7% less time |
| Physical SE directory-style | 266 | 267 | 184.7 → 185.4 ms | one-tick difference |
| Physical SE dense | 724 | 682 | 502.8 → 473.6 ms | 5.8% less time |

Medians above. Physical dense pairs: **724/683, 725/682, 706/679**; every pair
improves. The baseline varies more than the candidate, so do not promise an
exact percentage outside this matched test. Heap delta is zero in both paired
runs. No new end-to-end `ls`, file-I/O or nano-startup speed claim. Dense history
is still far from fluid.

[Raw Snow runs](snow-paired.txt), [structured measurements](measurements.json),
[physical summary](se-paired-summary.png), [physical phase counters](se-dense-phases.png).

## Remaining cost

Physical dense median counters after alignment, divided across 24 frames:

- Text/glyph preparation and composition: **300 ticks, about 208 ms/frame**.
- Presentation including the new shift: **224 ticks, about 156 ms/frame**.
- The shift itself: **141 ticks, about 98 ms/frame**.
- Subtracting the shift leaves roughly **58 ms/frame** for the existing
  presentation helper/QuickDraw work.

These are coarse 60 Hz phase counters; nested phases overlap and subtraction is
approximate. Nevertheless text composition is consistently the largest bucket.
The preparation pass consumes much of the earlier isolated 118 → 37 tick copy
saving. This is a modest end-to-end improvement, not a 70% redraw improvement.

Next focused opportunity: investigate producing the correctly aligned bits during
existing glyph composition, avoiding the extra shift pass. Measure the combined
cost and repeat the tail/bounds/clip proof before accepting that change. This is
**not implemented** here; no broad renderer rewrite has begun.

## Correctness evidence

- Host: **8,806 actual M68000** C-guard/assembly cases, all offsets 0–15,
  partial character groups, arbitrary prior bytes, overlapping moves, row/port
  boundaries and invalid-input fallbacks; whole-buffer guards match. Read/write
  hooks verify accesses within each row span and register checks verify the
  assembly boundary. An initial assertion incorrectly treated D2 as preserved
  across the compiled C wrapper; the wrapper uses it as scratch. Correcting the
  test boundary required no production change; all 8,806 cases then passed.
  Existing 510 glyph-row cases, selection delta and 1,560 scroll guards pass.
- Snow and physical diagnostic: **322 dirty-buffer comparisons** pass, including
  nonrectangular clipping, rows actually obscured by another window, and full
  terminal redraws while covered/uncovered. The random strip is deliberate test
  data; the test invalidates the window afterward to restore normal contents.
- Snow and physical light/dark tests each pass **22 bursts + 20 views**, zero text
  mismatches, and **147 clean scrollbar checks**. The initial physical dark capture
  was during the test; its completed report was read afterward in
  [se-dark-report.png](se-dark-report.png).
- Snow: actual selection and thumb dragging work. An 80-sample real drag showed
  zero transient blank text bands (4.15 s, median interval 50.5 ms, maximum
  60.6 ms). This cannot exclude shorter flashes. The user separately watched
  bidirectional physical dragging and reported **no blanking or missing scrollbar**.
  Remote button state ended released.
- Genuine nano opened the 8 KB fixture and exited; subsequent native text/bar
  checks passed. Invocation-owned peak 101,736 bytes, remainder zero. No new
  nano timing or safe-save modification is claimed.
- Snow diagnostic and normal 1521: output and pager Ctrl+C return **130**.
  Normal pixel/scrollbar checks also pass.
- The clipping diagnostic disposes its explicit temporary bitmap, regions and
  window. Its final FreeMem sample differs by -44 bytes in Snow and -20 on SE;
  this includes Window Manager/event state and is not attributed as a renderer
  allocation. The production alignment code contains no allocation calls.

Normal **1521** was then installed on the physical SE, automatically relaunched,
and confirmed by `clarice update --check` ([version](se-installed-version.png)).
Output and pager Ctrl+C both returned 130; actual thumb dragging and multiline
selection worked. SEControl ended connected, error 0 and mouse button released.
Free heap after listing/cancellation: **424,556 bytes (414.61 KB)**, largest block
424,128 bytes, owned remainder zero. This is 254 bytes below the prior 1519
sample, not a controlled peak comparison. The recovery listing retains 1519's
939,778-byte resource fork alongside earlier versions. No updater error was
observed. [Memory](se-normal-memory.png), [selection](se-normal-selection.png),
[recovery](se-recovery.png).

## Build and reproduction

Normal build **2026111521**: `Clarice-2026111521.bin`, 940,288 bytes; resource fork
940,037 bytes (**+259 bytes over 1519**), empty data fork. Application partition
and stack reservation are unchanged. No new persistent buffer/cache allocation;
the small executable increase must not be described as zero code-memory cost.
SHA-256 `c0dc52f6f0d28d264e16cf52a1187b08b6f1fe8bb681a3642922064fcc85c288`.
Accepted command resources are reused; no bundled utility rebuild or release.

Use the existing pinned Retro68 container and [checkpoint build procedure](../jump-perf/REPRODUCE.md).
From the repository root, run `tests/terminal/align-68k.py` with the existing
Unicorn 2.1.4 environment, then `native-glyphs.py`, `selection-host.py` and
`scroll-guard.py` with Python 3. The opt-in `CLARICE_DRAW_DIAGNOSTICS` build adds
`scrollbench alignment`, `alignpixels` and `glyphpixels`; normal builds exclude
those diagnostic commands. `alignment` writes `/tmp/align-profile.txt`.

For optional host observation, compile `tests/terminal/snow-drag-capture.swift`
with the command in its header. Match its documented Snow window geometry and
pass the actual thumb coordinates. Check before/after images to verify the drag
moved the view; sample counts alone do not prove input delivery.

The normal application and intermediate captures remain local artifacts. The
development checkpoint on `codex/se-alignment-2026111521` includes focused
source/tests and selected primary evidence, preserving checkpoint 7185b68 and
published release history. No release or tag is created.
