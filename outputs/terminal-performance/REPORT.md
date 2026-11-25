# Batch 2 performance — build 2026111964

Two contained changes; normal 68000 build, Snow Plus/4 MiB/System 6.0.8.
Physical-SE performance/acceptance remains unmeasured.

- Extend existing pixel scrolling only for a one-row forward live-input advance,
  with both carets in the excluded bottom row. Preserve selection, source-caret,
  clipping/visibility and cost guards. Move the matching row models with pixels;
  existing differences repaint exposed/changed input. Other cases retain fallback.
- Validate Preferences before comparing with current accepted values. Equal CLI,
  dialog and font/size menu settings skip layout, redraw and persistence. Dialog
  exposure still follows Window Manager updates. One byte records a missing or
  failed save so an equal request can retry; final flush errors remain visible.

## Matched measurements

Guest elapsed TickCount units (60/s), fixed Snow fast-forward, same 66×24 Monaco 9
window and dense history. Not host elapsed time, calibrated CPU time or physical
SE estimates. Each variant has 12 posted-key transitions across one-, two- and
three-row command boundaries. The same diagnostic binary can select the original
admission condition; timing excludes full-redraw pixel oracles and file writes.

| Measure | Original | Candidate |
|---|---:|---:|
| Light wrap draw, total /12 | 244 ticks | 109 ticks |
| Dark wrap draw, total /12 | 271 ticks | 110 ticks |
| Worst posted-event to visible, light/dark | 21/24 ticks | 10/10 ticks |
| Glyph rows reconstructed per wrap | 24 | 2 |
| Rasterised/presented row area per wrap | 101,750 pixels | 8,712 pixels |
| Existing framebuffer area moved per wrap | 0 | 95,832 pixels |

Wrap drawing improves 55–59%; worst queued latency falls from 350–400ms to 167ms
in this guest configuration. Total destination area touched increases about 2.7%:
this saves glyph reconstruction rather than eliminating the necessary screen move.
An earlier matched run measured 244/274→104/107ticks; the repeat confirms the gain.
Ordinary typing remains72 ticks/40 samples, worst 2, matching accepted build 1963.

Original unchanged CLI columns/rows/size/theme/control: 76–81 ticks, one layout,
one save/write, 24 rows repainted. Candidate:below one tick, zero layouts/saves/
writes/rows. Native dialog creation remains 52–56 ticks; this is not a zero-cost UI.

## Validation and costs

Host: 5,922 wrap/model/caret combinations, seven equal settings, invalid arguments,
first save and failed-save retry, no-op under layout-allocation failure; prior
clipboard/clipped-update/tracking regressions, 460 renderer comparisons, 8,505
geometry cases and allocation-failure rollback/retry pass with sanitizers.
Guest diagnostic: 245 checks, including 139 exact pixel comparisons; both themes,
clipping, selection, soft/hard clipboard breaks, editor restoration, 48 queued
wrap characters plus backspace, dialog OK/Cancel, native save failure and retry.
The probe restores theme pixels before dialog comparisons and finishes pending
post-dialog paints; normal dialog button handling is checked separately.

Normal shipping build: 446-character command typed through the keyboard produces the exact
421-byte file. Equal CLI settings and real dialog OK/Cancel preserve the config
checksum; editing a field then Cancel leaves the saved settings intact. Actual
60×20/dark changes persist across a normal quit/relaunch. Vi accepts two edited lines and restores the prompt.
Ctrl+C and Command-period both cancel a loop with 130; subsequent commands succeed.
Two warm 30-invocation batches: free heap 325418→325422 bytes, largest block 323098→323098,
remaining command allocations 0. Geometry reflow uses the existing history storage.
Host key bursts sent before Telnet startup finished lost characters; settled,
paced keyboard input and all 48 directly queued boundary events pass. This does
not establish a new maximum hardware/emulator keyboard injection rate.
Publication verification and normal-build evidence are recorded alongside.
Code +444 bytes, data +20, BSS unchanged 41448. Core MacBinary +512bytes (438272).
No new bitmap or heap allocation; the retry flag is the only new production state.
All 49 standard utility payloads remain byte-identical to 1963. Offline HDA Core
forks/Finder metadata, Release and 49 utilities, and 51 tar members match the seal.
Previous 1963 distribution/recovery material is retained. No physical access or
GitHub push. Stop after these two performance findings.
