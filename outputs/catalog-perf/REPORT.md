# Streaming directory performance — 15 November 2026

Snow only: patched Macintosh Plus emulator, 4 MB, System 6.0.8 media, native
Monaco 9 rendering. Physical SE untouched. Measurements below are guest 60 Hz
ticks unless explicitly described as host captures. No push or publication.

## Enumeration breakdown

An instrumented 1,000-entry catalog scan (no drawing) took 386/409/391 ticks.
Native PBGetCatInfo calls consumed 284/285/289 ticks, parameter setup 13/13/10,
name conversion/copying 30/33/24 and item/event handling 12/23/18. These are
quantised samples with extra instrumentation overhead; compare the preceding
normal-build 336–364 ticks, not an asserted slowdown. Successive 100-entry
segments cost approximately 37–39 ticks after setup: no growing quadratic cost
was observed. Evidence: scan-diagnostic.png.

The native HFS calls dominate. A general System 7 PBCatSearch backend is not
available on the System 6 baseline. We have not bypassed the File Manager or
added a raw catalog reader/cache. Apple's [indexed enumeration guidance](https://leopard-adc.pepas.com/technotes/fl/fl_31.html)
requires restoring the parent ID, which GetCatInfo overwrites; explicit volume
references remain mandatory. Indexed enumeration is not a snapshot against
concurrent changes by other applications.

Matched buffer-reuse experiment (same diagnostic image): original 393/350/348,
reuse 328/335/335 ticks. Warm original pair 350/348 versus new median335 saves
about 14 ticks (0.23 s, ~4%). Initial original393 is not treated as a comparable
warm result. No drawing in either. Evidence: reuse-0.png, reuse-1.png.

## Presentation experiments

Existing listing is already streaming with bounded row storage. It does not wait
for enumeration to finish before displaying entries. Bulk presentation cadence
was 32 ticks; cancellation/event polling remains every two ticks.

Simply adding timed presentation at six or three ticks is rejected: 100-entry
listing medians 110 -> 213 -> 336 ticks, with total three-pass painting 116 ->
421 -> 794 ticks. More frequent painting alone is substantially slower. These
benchls times exclude shell dispatch/loading and can exclude final presentation;
they are not equivalent to the previous complete-command measurements.
Evidence: cadence-0.png, cadence-6.png, cadence-3.png.

Matched actual rendering shows that forcing all scrolling is also wrong. Across
six native frames, a four-row step costs 82 ticks using the old heuristic versus
57 using pixel movement; twelve-row steps instead worsen from 82 to 107, and
24-row steps from 83 to 182. Cached rendering does not share the same tradeoff.
See render-matched.txt. Native raster counters include copying; do not sum them.

The retained native heuristic values each changed row's fixed Toolbox overhead
at 24 rather than eight character-cost units. This selects pixel reuse for small
moves but retains in-place patching for large nearly-identical listings. Cached
weight remains eight. No drawing implementation, clipping guard, scrollbar
staging or cursor design change. More frequent timed painting was not retained.

## Changes and focused validation

- `src/listing.inc`: one reusable synchronous HFS parameter block, restoring the
  explicit volume/parent/index on every call; encode returned names directly
  rather than copying through another buffer. Reject returned names over HFS's
  31-byte maximum. No pre-scan, whole-directory buffer or permanent allocation.
- `src/app.c`: measured native row-overhead heuristic; opt-in diagnostic hook.
- `src/shell.c`, `tests/terminal/catalog-guest.inc`: diagnostic-only matched
  scroll comparison. Excluded from normal builds.
- `tests/commands/listing-host.py`: actual production listing loop against HFS
  mocks that overwrite parent IDs. ASan/UBSan pass for explicit volumes, mixed
  directories/files, hidden names, escaping, maximum/malformed names, EOF,
  I/O errors, missing volumes and cancellation.
- Existing volume fail-closed host tests pass, including regression mutation.
- `tests/commands/catalog-guest.sh`: disposable two-volume guest regression.
- Normal build 2026111511 compiled: resource fork 938844 bytes (+13 versus1510),
  bundled core CLX 12068 bytes (-20). No new production idle allocation.
  [Experimental MacBinary](experimental-build/Clarice.bin), SHA-256
  `ea3008f69c727eff6c24b7ce05c03c1282bb8dbb797f73603dd828dafb099de4`.

Normal scan measurements are now 369/323/323 ticks (first/repeated; 6.15/5.38/5.38 s),
with zero painting, first entries at 13/14/13 ticks, and 16 event ticks across all
three scans. The preceding normal-build warm values were 338/336 ticks. This is
still too slow; the improvement is modest and does not remove the dominant native
HFS cost. Normal 100-file listing internals measured 133/95/100 ticks, with 116
paint ticks across the three runs. These are not complete shell-command timings.

Normal native pixel regression passes 24 output bursts plus 20 history views:
zero text mismatches, 153 scrollbar staging/reference checks, zero mismatches.
Actual screenshots during an unpaged 1,000-file listing show visible entries
advancing before completion (verified-cancel-native-off-running.png and
streaming-later.png); the latter captures an in-progress row repaint, not an
atomic whole-screen frame. Completed Ctrl+C output is coherent with status130.
Streaming existed already and is preserved; smooth one-line-at-a-time bulk
presentation is not claimed. Both-volume guest transcript passes (catalog-result.txt and guest-validation.txt):
relative/absolute paths, spaces, encoded percent names, hidden entries, creation
metadata, separate data sentinels and safe disposable creation/removal. Missing
volume/path errors remain -35/-43 with status1; no boot-volume fallback.
All four actual Ctrl+C cases pass: native/cached assembly, paging off/screen,
status130 and restored prompt. Runtime reports zero tracked allocations left,
425596 bytes free heap. The native/cached buffers and staging scheme are unchanged.
Cached pixel tests from the preceding task remain evidence, not rerun here. Initial automation stopped at Snow's
floppy conversion/writeback dialogs before Clarice launched; those screenshots
and attempted commands are not acceptance evidence. The disposable cloned disk
was converted, writeback enabled, and the actual app launch verified before
restarting the guest checks.

## Remaining limits and next focused step

This is a modest enumeration improvement, not a solution to five-second directory
scans. Larger improvements require distinguishing native File Manager CPU cost
from disk/cache service time within the measured trap; current counters only
time the boundary. Do not infer disk speed or recommend changing OS cache from
these measurements. Next investigate that split and the cost of moving/presenting
small native pixel regions before reducing the bulk paint interval again.
No raw HFS reader, permanent directory cache, OS patch, background enumeration
or new asynchronous-I/O lifetime was introduced. No product decision is required
for the retained changes; any such larger alternative needs separate assessment.
Physical SE and its files remain untouched. Temporary benchmark variants were
replaced with the normal commands. The test Snow was quit cleanly to extract
results; the unrelated emulator was left alone.
