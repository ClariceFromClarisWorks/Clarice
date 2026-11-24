# Normal-build glyph reuse acceptance — 24 November 2026

**Pass for Snow integration; physical acceptance remains pending.** No production
change was required. Candidate: Core **2026111945**, implementation **2ad0967**,
source checkpoint **ab078b7**; optional vi remains the accepted 1943 binary.
Normal MacBinary SHA-256:
`a5d3c0e45902c25e72d272bcbaaf70426b33c70d2a227729ff29c3dc4b298140`.
This implementation was already integrated/published under earlier authorisation.
This follow-up performed no merge, push, publication or hardware deployment.
Q1 remains untouched.

## Functional and memory acceptance

Isolated Snow Macintosh Plus, 4 MiB, System 6.0.8, fixed maximum acceleration
(approximately 15x host pacing); existing normal production binary, no profiling
flags. Disposable files only. `normal.json` records six vi cycles (three per
light/dark theme), each with 65 down/up movements, four forward/back page pairs,
edit/save/exit and exact 8 KiB fixture readback. Nano inserted text, saved and
exited; exact saved text and successful subsequent shell command verified.
`prompt.png` confirms restored shell, cursor and intact scrollbar.

The six identical workflows each ended with heap **311338**, largest free block
**309370** bytes. Those samples follow cat, so they establish workflow stability,
not vi's individual allocation accounting. Additional direct editor-exit samples
in `direct-editor-cleanup.json` specifically avoid intervening external commands:

- Four vi runs, including Command-period cancellation after scrolling/paging:
  heap **333230**, largest **331568** bytes, unchanged each time; command peak
  **23396** bytes and **remaining=0** every run. Subsequent vi and shell work pass.
- Nano peak **77160** bytes; **remaining=0** after each exit. Four direct samples
  (last sample there plus `nano-cleanup.json`) have free heap **333222–333226**,
  ending at the initial **333222**, largest block **331580–331626**: small native
  bookkeeping variation, no observed progressive loss. This is bounded repeated
  testing, not an indefinite leak proof.
- Existing normal-build exact 20j/20-arrow position, 120-key burst, Ctrl+C editor
  ownership, Command-period status 130 and prompt recovery evidence is preserved
  in ../normal-acceptance.json; not needlessly rerun in full.

## Matched performance and correctness

Reuse the completed, matched diagnostic before/after measurements with the same
vi and 128-line, 8 KiB document. The normal binary intentionally lacks counters;
these are diagnostic guest event-to-presentation times, not normal-build wall
clock or physical-SE timings. Maximum is the worst **sampled** response, not a
hard upper bound. Tick resolution is approximately 16.7 ms.

| Workload | Before average / max | After average / max | Paint ticks before → after |
|---|---:|---:|---:|
| 65 down + 65 up, crossing viewport | 554 / 867 ms | 450–452 / 700–717 ms | 3432 → 2043–2055 |
| 10 forward/back page pairs | 752 / 850 ms | 594 / 667 ms | 789 → 453 |
| 120 vertical cursor movements | 84.15 / 100 ms | 84.43 / 100 ms | 127 → 124 |
| 120 horizontal cursor movements | 54.78 / 100 ms | 52.46 / 83 ms | 13 → 14 |

Scrolling paint is about **40%** cheaper; paging about **43%**. Complete response
improves about **18–19%** and **21%**, respectively. Bytes, requested cells and
response counts match. Paint ticks are elapsed active-path cost, not exclusive
CPU cycles; cells are requested drawing coverage, not framebuffer bus writes.
Cursor-only responsiveness is effectively unchanged. Heavy updates still have
substantial latency; this is not a claim of real-time 15x acceleration.

Existing **568 actual guest pixel comparisons, zero failures** remain intact
(../pixels-expanded.txt): dirty/partial groups, high-byte fallback, clipping,
themes and cursor transitions. Reran editor-render.py: **160 + 300 host
presentation comparisons pass**; the host glyph stub does not replace actual
guest pixel coverage. Previous selection, control-mapping, inverse and cursor
regressions remain applicable; no production source changed in this follow-up.
The old standalone native-glyphs harness still lacks current cache stubs and is
not claimed passing.

## Binary and allocation inspection

Normal target compiled `-Os -m68000`. `editor-68000.txt` contains the actual
inlined editor path in draw: calls existing native_glyph_row and glyph_planes_end,
retains QuickDraw presentation/clipping, adds no allocation call or new assembly.
Existing assembly kernels are unchanged. Preserved pre-change ELF comparison:
text **232160 → 232384 (+224 B)**, data **35588 unchanged**, BSS **41436 unchanged**;
draw grows **1210 → 1434 B**. The comparator is the preserved pre-integration
ce53aa7 build; exact distribution comparisons separately use Core 1943 and 1945:
CODE/1 **278015 → 278250 (+235 B)**, MacBinary **394752 → 395008 (+256 B)**.
Resource growth includes native packing/relocation metadata (resources.json).

The existing **4096-byte prompt_pixels** is unchanged: the editor reuses its
first 704 bytes and existing glyph storage. No new persistent bitmap, static
allocation or cache type. The already-existing **24320-byte purgeable glyph
handle** may be used by this path; it is unlocked and marked purgeable after
painting. This is reuse of existing reclaimable capacity, not a claim that all
native heap readings must equal a cache-cold baseline. Editor screen allocations
and command ownership remain unchanged.

**Recommendation:** retain the candidate. Snow normal-build acceptance is
complete; do not infer physical-SE acceptance or guaranteed worst-case latency.
No Q1 work or further rendering changes are needed for this acceptance task.
