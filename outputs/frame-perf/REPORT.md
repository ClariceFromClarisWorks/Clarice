# Per-frame drawing cost — 15 November 2026

Normal build **2026111515**. Focused changes to native terminal model movement
and row comparison; selection, cursor, pixel presentation, scrollbar composition,
HFS, command execution and bulk-output cadence are unchanged. No push/publication.

## Cause and change

The linked libc `memmove` and `memcmp` require four-byte alignment for their
longword paths. The 68000 only requires even alignment, and terminal rows can
start at two modulo four. Thus moving/comparing already drawn rows repeatedly
used slow byte loops. Disassembly and isolated guest timings confirmed the cost. The baseline
link placed `last_screen` at `0x3c7e6`, i.e. two modulo four.

- Use supported System 6 `BlockMove` for overlapping native text-model movement.
- Compare the complete, even-aligned 132-byte model row with a bounded 68000
  equality routine, including its cursor marker. It returns only equal/changed;
  it does not replace general libc comparison or alter selection handling.
- Add opt-in `scrollbench frames` phase measurement, with alternating old/new
  paths inside the same executable. Diagnostics are absent from the normal build.

Changed implementation: `src/app.c`, `src/kernels.S`, diagnostic dispatch in
`src/shell.c`, build stamp `src/update/version.h`. Tests: `frame-guest.inc` and
`frame-68k.py` under `tests/terminal/`. Existing unpublished work is preserved.

## Matched Snow measurements

Plus, 4 MB, System 6.0.8 media; Monaco 9, 80 columns × 24 history rows, light theme.
Three alternating repetitions, 24 frames each, no report I/O inside timed loops.
Guest ticks are 1/60 second. [Raw phases](paired-phases.txt),
[structured measurements](measurements.json).

| Workload | Total ticks before → after | Paint ticks before → after | Worst frame ticks |
|---|---:|---:|---:|
| Varied output | 181 → 126 | 159 → 103 | 7 → 5 |
| One-line history movement | 201 → 145 | 201 → 144 | 9 → 7 |
| Four-line history movement | 310 → 261 | 309 → 260 | 14 → 12 |

Output drawing cost falls **35.2%**; total output workload falls 30.4%.
One-line history total falls 27.9%; four-line history falls 15.8%.
In the output case, model movement falls 21 → 5 ticks and row checking/painting
87 → 49 ticks. Pixel movement remains approximately 33 → 31; composing text and
scrollbar work are effectively unchanged. Raster includes copy; shift includes
pixels/model, so these counters must not be added twice. Tick quantisation
affects short phases. The probe retains zero allocations.

Normal-build `scrollbench quick`: **225 → 157 ticks for 30 frames**, about
**8.0 → 11.5 fps**; worst frame 7 → 5 ticks. Baseline is the preceding stock Snow
run; the final normal build uses the existing keyboard-fixed Snow executable.
Cached ASM stays at 233 ticks in both, but the primary attribution is the
same-executable alternating phase test above, not separate emulator runs.

## Real scrollbar dragging: smaller gain

Same real 1,000-file listing, same six host mouse steps and view geometry.
Guest paint work **84 → 79 ticks over six draws (6.0%)**; worst completed draw
remains 14 ticks. Host median last observed text change **251 → 246 ms**, within
the roughly 50 ms sampling granularity. Five of six steps were detected in both;
the first-step automation limitation remains. These are step-and-pause tests,
not continuous-drag throughput or precise mouse-to-photon measurements.

This does **not** establish a perceptibly smoother large-jump drag. Such jumps
still repaint many changed text spans. Smaller incremental scrolling benefits
more. One attempt safely aborted when Snow lost host focus; it is not a pass.

## Focused validation

- 532 actual M68000 comparison cases: equal rows, every differing byte, both
  even alignments, no overread/write, preserved registers/stack.
- Existing selection-delta host regression and 1,560 cursor/view guard cases pass.
  The user separately reports physical click-and-drag selection works brilliantly;
  that path was deliberately preserved.
- Diagnostic light and normal dark native pixel checks: each **22 bursts +
  20 views, zero pixel mismatches**, **147 scrollbar checks, zero mismatches**.
  These check complete rendering and staged scrollbar integrity, not high-speed
  video flicker certification. The no-visible-erase presentation path is unchanged.
- Normal quick test: zero failures, ring/wrap/reference tests and four temporary
  cache release cycles stable. Normal Ctrl+C during listing and at pager returns
  **130**, with a usable prompt. Screenshots are retained in this directory.
- Owned Snow image was stopped before offline export; original preferences were
  restored and the temporary Finder launch copy removed. Updated root app remains.
  Unrelated running emulator/workspaces were left alone.

## Build and hardware

[Normal experimental application](experimental-build/Clarice.bin), 939,008 bytes.
SHA-256 `c25459d972e4127e98398e8645eb404d97b7d5cf52f3bb24dd2388889e10bebf`.
Resource fork **938,856 bytes**, +224 bytes; no new resident buffers/cache or
partition increase. Final Snow sampled free heap is 425,932 bytes; invocation
owned remainder zero. Safe-save/updater/CLX commands are unchanged.

One separate display limitation was observed after the command-line dark-to-light
switch: old rows retained their prior background until repainted (`normal-memory.png`).
Steady-theme pixel comparisons passed; theme-switch invalidation is not claimed
fixed by this work, and no cursor changes were made.

The user also requested an updater follow-up: a viewport-visible `Y/n` prompt
and explicit `-y` acceptance option. This is recorded in ROADMAP.md, not added
to this drawing change.

## Physical SE acceptance

User restored stock 68000 mode after a separate unofficial accelerator experiment.
4 MB, System 6.0.8, MultiFinder, Monaco 9/80×24. Quiet SEControl during timed loops.
The API reports processor 1/System 0607, the existing documented resource/API
version discrepancy. [Structured hardware evidence](physical-measurements.json).

| Same normal-build workload | 1514 before | 1515 after |
|---|---:|---:|
| Native scrolling, 30 frames | 248 ticks / 4.13 s | 174 ticks / 2.90 s |
| Native scrolling throughput | 7.3 fps | 10.3 fps |
| Worst measured frame | 8 ticks / 133 ms | 5 ticks / 83 ms |
| 452-file listing-loop median, three runs | 336 ticks / 5.60 s | 329 ticks / 5.48 s |
| Paint across those three listings | 168 ticks | 145 ticks |
| Sampled free heap after listings | 425,964 bytes | 425,746 bytes |

Physical native scrolling takes **29.8% less time** (single run per normal build,
consistent with the preceding 249-tick baseline). Actual directory throughput
improves only about 2%; it remains dominated by work outside drawing. The 218-byte
sampled heap difference is consistent with small code growth; no new bitmap or
persistent working allocation. Invocation-owned remainder is zero.

- Native check passes **22 bursts + 20 views**, zero pixel mismatches;
  **147 scrollbar checks**, zero staging/reference mismatches.
- Real scrollbar drag moves the retained view and leaves its complete track,
  thumb and arrows visible. No missing section reproduced. Physical drag latency
  was not measured by the slow Wi-Fi screenshot interface.
- Ctrl+C interrupts ordinary output and pager, both status **130**, usable prompt.
  Paging restored to screen and the final app left frontmost at the prompt.
- Private Wi-Fi update downloaded, staged/verified, replaced and automatically
  relaunched the app; installed build **2026111515** was checked. Prior app retained
  as `Macintosh HD:.cu-0000039b-78c4021b.old`, resource 938,632 bytes, matching the
  prior artifact's fork length. Earlier backups and installed commands preserved.
  Error 28 did not recur; no updater code was changed.

This is a measured drawing improvement, **not** completion of terminal fluidity.
Large thumb jumps remain the weaker case. Native text painting across many rows
and scrollbar rendering remain significant costs. The existing post-paint drag
cadence was not changed; continuous-drag pacing remains a focused measurement
opportunity. No new nano, filesystem, accelerator or updater optimisation claim.
