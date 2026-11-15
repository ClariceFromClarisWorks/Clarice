# Direct aligned glyph composition — 15 November 2026

Focused experiment against accepted build **2026111521**. Retained after all
three matched physical-SE pairs improved. No renderer rewrite, new bitmap,
permanent allocation, preference, cursor change, commit, push or release.

## Opportunity and implementation

The preceding physical end-to-end runs attributed 133–148 ticks across 24 dense
redraws to the separate alignment pass: median **97.9 ms/frame**, about 21% of
473.6 ms. That justified an experiment; the isolated CopyBits benchmark alone
was not used to predict the gain.

Eligible Monaco 9 ASCII history spans now compose directly at the screen's bit
phase inside the existing 4096-byte buffer. The existing four-glyph grouping is
retained. Aligned word/long stores merge boundary bytes without a second row
shift; odd-byte groups use even addresses and preserve the preceding byte.
Only carry/tail bytes are cleared. No glyph storage is added or retained beyond
the existing buffer. QuickDraw still handles clipping/obscured windows.

Short spans, other fonts/geometry, selections and unsupported characters retain
the established path. The separate alignment helper remains for those cases;
it is effectively absent in the measured dense ASCII workload. Initial dirty
pixel tests passed, but a stricter audit found a zero-OR write beyond an exactly
fitting tail. The shared cell loop now touches a second byte only when glyph
bits cross that boundary. This was corrected before physical installation;
write hooks reject touching glyph storage even when the value would not change.

## Matched complete redraws

Final diagnostic 2026111524; three alternating pairs, 24 large history jumps per
workload, 80×24 Monaco 9/light/128 retained lines. Seeding and report I/O excluded,
first glyph preparation included. Physical SE is stock 68000/4 MB/System 6.0.8;
SEControl heartbeats/captures suppressed during timing. Each comparison executes
both paths in the same binary; absolute values from different builds need not
match exactly. Earlier prototype 1522 measurements are retained separately.

| Workload | Median ticks before → after | Mean frame before → after | Reduction |
|---|---:|---:|---:|
| Snow directory-style | 228 → 228 | 158.3 → 158.3 ms | unchanged |
| Snow dense | 579 → 471 | 402.1 → 327.1 ms | 18.7% |
| Physical directory-style | 264 → 265 | 183.3 → 184.0 ms | one-tick difference |
| Physical dense | 678 → 599 | 470.8 → 416.0 ms | 11.7% |

Physical dense pairs: **678/599, 679/601, 678/598**. Every pair improves; paired
heap delta zero. Across all 288 matched redraws, Snow takes 4,520 ticks versus
5,418 on SE: approximately **20% faster overall**, with the SE delivering 83% of
Snow's speed. The retained path alone varies from 16% to 27% faster in Snow by
workload; do not reuse the earlier provisional 60% SE-speed assumption here.
This is not a general loading/I/O/nano multiplier. Directory-style differences are too small to claim a benefit.
Dense redraws remain expensive. These are complete draw-call timings including
the scrollbar, not complete `ls` timings or continuous-drag input latency.
[Measurements](measurements.json), [Snow raw](snow24-paired.txt),
[physical summary](se-paired-summary.png).

## Remaining cost and priority

Physical medians across 24 dense frames (60 Hz counters):

| Phase | Before | After |
|---|---:|---:|
| Text/glyph work | 204.2 ms | 252.8 ms |
| Presentation, including alignment | 158.3 ms | 61.1 ms |
| Alignment alone, included above | 97.2 ms | 0.7 ms |
| Row preparation | 34.7 ms | 33.3 ms |
| Scrollbar | 34.7 ms | 34.7 ms |
| Terminal-model composition | 14.6 ms | 11.8 ms |

Nested coarse counters overlap; phase medians do not sum exactly to total.
Direct composition adds about 49 ms of glyph work while removing about 97 ms
from presentation. Snow understates that extra composition cost: its larger gain
must not be substituted for physical results. No instruction-level explanation
of the emulator difference is claimed. Text/glyph work is now about **61%** of the
physical dense frame; further substantial gains would need another compositor
experiment, not more CopyBits tuning.

**Recommendation: pause renderer work and profile current loading/nano startup
on the SE next.** The latest controlled Snow editor evidence is 7.23 s empty /
10.08 s for 8 KB, with a repeated loader near 187 ticks (3.12 s). This task's
cold nano smoke run independently records **291 loader ticks (4.85 s)**: read 61,
validation 85, imports 4, relocations 19, preparation 110. Full payload checksum
is already skipped for this verified image; the remaining validation cost is
metadata/structural work. Its total invocation lifetime includes a host wait and
is **not** a new startup measurement. Target current cold allocation/preparation
and metadata processing before considering a native-loader redesign. This task
makes no nano startup improvement or new physical nano timing claim.

## Validation and footprint

- Host: 1,339 actual M68000 direct-composition cases, all phases/tails/dirty bytes,
  bounded/even accesses and read-only glyph storage; 8,034 C glyph/phase/theme
  cases; 356 existing M68000 compositor/ABI cases across widths 1–8 and heights
  1/11/32. Existing 85 tail-clearing, selection-delta and 1,560 scroll guards pass.
- Snow final diagnostic: 322 clipping/dirty comparisons including actual window
  occlusion; zero failures and heap delta zero. Light/dark each pass 22 bursts +
  20 views, zero pixel mismatches and 147 clean scrollbar checks.
- Snow real dragging and multiline selection pass. 80 captures over 4.15 s,
  median 50.5 ms interval, zero sampled transient blank bands; shorter flashes
  cannot be excluded. Two host input attempts safely stopped on focus loss;
  only the unfinished steps were resumed. Output and pager Ctrl+C both return
  130. Genuine nano opened the 8 KB fixture and exited; native pixel/bar checks
  afterward pass, owned peak 101,736 bytes, owned remainder zero.
- Physical: matched performance and 322 clipping comparisons pass; heap delta
  zero. User watched bidirectional rapid dragging and reported **no blanking or
  missing scrollbar**. Remote button state ended released.
- Physical light/dark each pass 22 bursts + 20 views and 147 clean bar checks.
  Multiline selection passes; output/pager Ctrl+C return 130. Diagnostic free heap
  stays 410,282 bytes after the checks, invocation-owned remainder zero.
- Normal 1525 passes Snow pixels/bar and both Ctrl+C cases. Physical private-LAN
  update and automatic relaunch completed; installed-version check confirms
  1525. Native pixels/bar pass and output/pager Ctrl+C both return 130. After
  cancellation, free heap is 423,998 bytes (414.06 KB), largest block 423,502,
  invocation-owned remainder zero: 558 fewer free bytes than the 1521 sample,
  consistent with the code growth below. This is a final heap sample, not an
  extended soak. Captures: `se-normal-off-status.png`, `se-normal-screen-status.png`,
  `se-normal-memory.png`. `se-recovery.png` confirms retained applications,
  including the 940,037-byte resource-fork baseline at
  `Macintosh HD:.cu-00000314-78c40224.old`; rollback was not repeated this task.
  Final SEControl status reports released mouse button and no input error.

Normal **2026111525** uses the accepted bundled command resources, all drawing
profiling disabled. MacBinary 940,800 bytes, resource fork 940,593 bytes, data fork
empty: **556 bytes more code/resources than 1521**. No new persistent allocation;
768 KiB partition and 64 KiB stack reserve unchanged. Do not describe code growth
as zero memory cost. SHA-256:
`1e3e5ffc76fed81536a69a4aae627a08be7c655dd4c66c49756c38fb64caa6cd`.

Source: `src/native-glyphs.inc`, `src/kernels.S`, the small `draw_native` integration,
and diagnostic dispatch. Tests extend existing glyph, jump and clipping probes;
`tests/terminal/direct-68k.py` adds the actual-CPU direct-output guard. Reproduce
using the pinned toolchain and existing [build procedure](../jump-perf/REPRODUCE.md).
Diagnostics: `scrollbench direct`, `directpixels`, `glyphpixels`; normal builds
exclude these modes. Accepted checkpoint **33f0075** remains unchanged.
