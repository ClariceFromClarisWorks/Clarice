# Matched terminal drawing measurements — 15 November 2026

Snow Macintosh Plus, 4 MB, System 6.0.8; Monaco 9, 80 columns, 26 history rows,
light theme for timings. No physical SE testing or deployment. The SE is shut down.
The API reports 0607; this is the previously documented discrepancy in the
[6.0.8 media/resource verification](../system608/TEST-REPORT.md), not a new
inference that the guest runs 6.0.7. Existing system media were reused.

## Changes and cause

Cached scrolling moved its offscreen bitmap but then copied the whole visible
bitmap to the screen again. Reuse the existing native overlapping QuickDraw
screen scroll, then present only changed/exposed rows. Covered/clipped windows
retain the full cached presentation fallback. The stationary prompt no longer
blocks the assembly cache's history scroll; the C cache retains its guard because
moving its bitmap was slower in the earlier experiment.

Scrollbar setters already repaint changed control parts. Keep their existing
staged bitmap instead of clearing it and additionally calling Draw1Control on
every update. Full redraws still initialise it. The live scrollbar receives one
completed copy; no visible erase was introduced. Explicit pixel tests compare
the staged result with a full Control Manager redraw as well as checking that
intermediate drawing leaves the live scrollbar untouched.

No new persistent allocation. Normal application resource fork is 938732 bytes,
238 bytes larger than build 2026111508; MacBinary is 938880 bytes. The additional
reference-test snapshot is 684 bytes of bounded local storage. Native strip
batching and a changed-scanline scrollbar-copy experiment were not retained:
neither improved the relevant complete operation.

## Matched timings

[Matched raw counters](matched.txt) use old/new branches in the same diagnostic
executable, identical heap layout and seeded text, and a warmed real ls command.
Times are guest ticks (60 per second), including final presentation. Listing
figures are the median of three complete `ls -la` calls on the same 100 files;
output and history figures each cover 24 draws. No host typing is timed.

| Operation | Native before → after | Cached C before → after | Cached assembly before → after |
|---|---:|---:|---:|
| Complete ls, 100 files | 173 → 172 | 191 → 177 | 186 → 174 |
| Varied output, 24 frames | 189 → 190 | 382 → 226 | 353 → 197 |
| History, 24 view changes | 231 → 199 | 296 → 264 | 281 → 218 |

Assembly cached output elapsed time fell 44.2%; history fell 22.4%, versus 13.9%
for native history. Cached listing fell 6.5% (3.10 → 2.90 s); native listing was
effectively unchanged (2.88 → 2.87 s). Native remains competitive without the
29 KB optional rendering cache; no default preference change is justified.

The cached output copy phase fell 202 → 11 ticks; total paint fell 329 → 174.
Native history scrollbar work fell 75 → 46 ticks. Assembly history raster work
fell 148 → 65 ticks, although moving pixels adds cost elsewhere. Its worst frame
was 18 ticks versus 17 before: the improvement is not universal low-latency or
smooth-drag acceptance. Native worst history frame fell 10 → 9 ticks.

Free heap was stable across measured cases within each renderer: 423842 native,
394138 cached. Whole-probe heap delta was -2 bytes, not an exact-zero result.
Diagnostic counters and legacy comparison branches are excluded from the normal
build. Native raster counters include copy time; do not add those phases twice.

## Validation

- Focused host selection-delta and 1560 cursor/view guard cases pass; see
  selection-host.txt and scroll-guard.txt. Normal and diagnostic Retro68 builds pass.
- Normal build 2026111509, light theme: native and cached pixel checks pass,
  each with 24 bursts, 20 view changes, zero pixel mismatches and 153 scrollbar
  staging/reference checks with zero mismatches. Screenshots:
  light-native-complete.png and light-cached-complete.png.
- Native/assembly cached, paging off/screen: real Ctrl+C stops a 1000-file listing,
  restores the prompt and `echo $?` reports 130 in all four cases. See
  patched-cancel-*-status.png and patched-cancel-actions.json.
- The installed stock Snow failed to deliver modifier input; its cancel-*.png
  captures are failed automation, not passes. Reused the already-built repository
  Snow keyboard-mapping patch, without changing Clarice input code. It maps host
  Control to Option for the non-ADB Plus. Performance A/B results above all use
  the same stock Snow; patched Snow is used only for subsequent functional checks.
- Dark native/cached checks also pass: each 24 bursts, 20 view changes, zero
  pixel mismatches and 153 clean staging/reference checks. See dark-*-pixels.png.
- Normal `scrollbench quick` passes with zero failures: row-copy/cell checks,
  bounded history/wrap, incremental views and four cache release cycles stable.
  Cache allocation including heap overhead is 29704 bytes. After returning to
  native mode, `status runtime` reports heap 425716, largest block 425596,
  tracked remaining zero. See normal-quick-cleanup.png and normal-runtime.png.
  These normal-build functional results are not substituted into the matched A/B
  timings; the test workload and emulator build differ.

## Build and remaining work

[Normal experimental build](experimental-build/Clarice.bin), build 2026111509.
SHA-256: `89dcb6c6657ef34a9785f4d20e850eb94481ddd6d5e85de2c7b4d72a5ed9556f`.
No push, publication or physical update. Nano, safe-save and updater unchanged.

The complete listing is still dominated by work outside painting (new native
121 of 519 ticks over three listings). This pass does not identify that residual
as disk I/O, formatting or loading without further measurement. Actual drag
mouse-to-screen latency and physical performance remain unverified.

For planning only, the user's SE-at-60%-of-Snow-speed assumption means elapsed
Snow time ×1.67: the cached 2.90 s listing would be about 4.83 s on the SE. This
is an estimate, not hardware evidence or a guaranteed scaling factor. Updater
error 28 remains deferred. Next useful measurements are real drag latency and
the non-paint portion of complete directory listings, before further optimisation.
