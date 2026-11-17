# Row clearing and presentation — 15 November 2026

Focused experiment after checkpoint `7185b68b0144d8c869990f61a6c4a278ca69e367`.
The checkpoint remains unchanged. No new assembly, bitmap, permanent allocation,
renderer preference, cursor change, filesystem change or loader optimisation.

## Retained change

`native_glyph_clear()` clears only the 1–3 bytes per scanline used by the
compositor's final 1–3 characters. Complete four-character groups overwrite all
three destination bytes. A full 80-character row therefore needs no clearing.
Unused padding is not presented. Short rows, partial groups and nonzero previous
pixels are explicitly tested; glyph storage at bytes 704–3743 stays untouched.
Other fonts, extended characters, selected text and forced redraws retain their
existing paths. Production changes are confined to `src/native-glyphs.inc` and
the build stamp; the other changes are opt-in diagnostics and tests.

## Matched end-to-end results

Three alternating old/new pairs in the same diagnostic executable, 24 large
jumps per case. Monaco 9, 80×24, light theme, 128 retained lines. Seeding, report
I/O and reporting redraws are excluded; cold glyph preparation is included.
Physical SEControl heartbeats/capture were suppressed during timing. Physical
CPU code 1; 4 MB/System 6.0.8 as configured by the user (System API reports 0607).

| Machine/workload | Before ticks | After ticks | Mean frame before → after | Reduction |
|---|---:|---:|---:|---:|
| Snow directory-style | 248 | 230 | 172.2 → 159.7 ms | 7.3% |
| Snow dense | 637 | 615 | 442.4 → 427.1 ms | 3.5% |
| Physical SE directory-style | 289 | 267 | 200.7 → 185.4 ms | 7.6% |
| Physical SE dense | 732 | 701 | 508.3 → 486.8 ms | 4.2% |

Table uses medians. Physical directory pairs: 289/267, 289/267, 288/266;
dense: 732/704, 732/700, 732/701. All pairs improve; heap delta is zero.
Physical dense worst frame 31 → 30 ticks. These are isolated same-image results,
not a claim that different diagnostic binaries have identical baseline timings.
The benefit is modest but reproducible with a small implementation; retained.
Dense scrolling remains far from fluid.

[Raw Snow runs](snow-paired.txt), [measurements](measurements.json),
[physical summary](se-paired-summary.png), [physical complete tail](se-paired-tail.png).
The summary capture was taken while `cut` was still drawing; its final records
are independently visible in the completed tail capture.

## Where presentation time goes

Batched copies avoid the severe tick-boundary aliasing in per-row phase counters.
Each case makes 300 copies of a 464×11 monochrome row, `srcCopy`, no scaling;
three repetitions. Temporary 704-byte destination is on the diagnostic stack.
The small control uses 16×11. No production copy path was changed.

| Copy case | Snow median ticks | SE median ticks |
|---|---:|---:|
| Existing presentation helper | 112 | 122 |
| Direct QuickDraw call, current offsets | 109 | 118 |
| QuickDraw, aligned screen destination | 33 | 36 |
| QuickDraw, mismatched offsets in ordinary memory | 108 | 117 |
| QuickDraw, aligned ordinary memory | 31 | 35 |
| QuickDraw, tiny ordinary-memory row | 22 | 24 |
| Existing assembly copier, aligned ordinary memory | 18 | 21 |
| QuickDraw, tiny screen row | 25 | 28 |
| QuickDraw, source offset matched to unchanged screen destination | 33 | 37 |

Snow destination offset was 8 pixels modulo 16; physical SE offset was 7.
Matching offsets recovers the fast path without moving the window. On the SE,
the direct QuickDraw call costs about 6.56 ms/row versus 2.06 ms with matched
offsets. The ~70% difference also exists in ordinary memory: alignment handling
dominates, rather than framebuffer bandwidth or display-format conversion.
The helper's checks add only about 0.22 ms/row. Aligned copies still have real
data movement and QuickDraw setup/clipping costs; they are not free.

The existing assembly copier requires even addresses. Do not relax its eligibility
check for arbitrary byte/bit offsets: that could cause 68000 address errors.

Separate clearing batches (2,000 calls) corroborate the redraw gain: physical
warm full clear 102 ticks; no-tail clear 4, three-character-tail clear 28.
The first full-clear sample was 113 ticks; its extra cost is not attributed.
Snow medians are 80, 3 and 26 ticks. No heap change in either copy probe.

[Snow presentation](snow-presentation.txt), [physical first half](se-presentation-head.png),
[physical second half](se-presentation-tail.png). Hardware values are transcribed
in measurements.json; no hardware raw-file export is claimed.

Recommended next experiment, **not implemented**: align the completed row's bits
within the existing buffer to the destination offset and pass the corresponding
source rectangle to QuickDraw. Measure the alignment work as part of total redraw.
The copy-only saving suggests roughly 100 ms/frame is available before alignment
overhead for this dense workload, not a promised end-to-end gain. Keep clipping,
cursor handling and obscured-window behaviour with QuickDraw; retain fallbacks
where width plus offset does not fit the 512-pixel row. Test all bit offsets,
partial widths, both themes, cache boundaries and overlapping windows before
considering any deployment. No large renderer rewrite is warranted by this data.

## Validation and build

- Host: 85 actual M68000 C-clear/compositor cases with arbitrary prior bytes,
  all lengths 1–85 and whole-buffer guards; 510 glyph-builder cases including
  cold rebuilds, themes and fallbacks; selection delta and 1,560 scroll guards.
- Snow diagnostic: light/dark dense ASCII plus extended-character fallback each
  pass 22 bursts + 20 views, zero text mismatches and 147 clean scrollbar checks.
  Actual selection and thumb dragging work. Ctrl+C returns 130 during output and
  at the pager. Paired/micro probes return zero heap delta.
- Normal Snow 1519: native pixel/bar check passes, and output/pager Ctrl+C return
  130. The guest disk contains the normal build; the unrelated Snow guest remains
  untouched. No new nano startup or HFS throughput claim.
- Physical diagnostic light and dark checks each pass the same 22 bursts +
  20 views / 147 scrollbar checks, with zero text or scrollbar mismatches.
- Physical normal 1519 automatically relaunched after private Wi-Fi update.
  The installed build number is visible in `se-normal-off-running.png` (the
  earlier version-check capture was taken before the check completed).
  Actual thumb dragging changes the history view; multiline selection works;
  output and pager Ctrl+C each return 130. Remote mouse state ends released.
  See `se-normal-thumb.png`, `se-normal-selection.png`,
  `se-normal-off-status.png` and `se-normal-screen-status.png`.
- Physical sampled free application heap is 424,810 bytes (**414.85 KB**),
  largest block 424,716, invocation-owned remainder zero (`se-normal-memory.png`).
  This is 68 bytes below the earlier 1517 after-listing sample, consistent with
  the small code increase; it is not a controlled peak-memory comparison.
  Both paired probes have zero heap delta; no new permanent allocation exists.
  The previous normal application (resource fork 939,714 bytes) and diagnostic
  application remain in the recovery listing (`se-recovery-files.png`).
- No high-speed physical video was taken: pixel oracles and interaction captures
  support rendering correctness, not a new measured flicker/continuous-drag
  latency claim. Existing dark-to-light preference invalidation remains outside
  this change. No new updater error was observed.

Normal build `Clarice-2026111519.bin`: 940,032 bytes, data fork zero, resource
939,778 bytes (**64 bytes above 1517**), no added bitmap or idle allocation.
SHA-256 `3acf3aecb5ad5cc25d7af3fdfabb586bc9293d2a2e93eb35860b496b3af59924`.
Builds reuse the accepted command resources; no bundled-tool rebuild is implied.
The application remains a local generated artifact; selected primary captures
are tracked. This experiment was left uncommitted at its acceptance and is now
included in `codex/se-alignment-2026111521`, preserving checkpoint 7185b68 and
published release history.

Reproduce host checks with `work/date-test-env/bin/python tests/terminal/glyph-clear-68k.py`
(Unicorn 2.1.4 and the pinned Retro68 container), then `python3` on
`tests/terminal/native-glyphs.py`, `selection-host.py` and `scroll-guard.py`.
The existing opt-in draw build exposes `scrollbench clearing`, `present` and
`glyphpixels`. Normal builds exclude those diagnostics. See prior checkpoint
[build instructions](../jump-perf/REPRODUCE.md). No release or tag.
