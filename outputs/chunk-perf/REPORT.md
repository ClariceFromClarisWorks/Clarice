# Faster complete directory listings

15 November 2026. Snow Plus, 68000, 4 MB, System 6.0.8. Physical SE untouched.
Normal build 2026111513; no push or publication.

## Measurements and decision

Same real `ls -la` directories, Monaco 9/80 columns, 26 rows, seeded history,
paging off. Guest timing starts at execute() and ends after final prompt paint.
Three runs per case, warm command loading; screen seeding is outside the timer.
Previous matched baseline: ../auto-render/matched.txt. Current: matched.txt.

| Entries | Previous 32-tick cadence | Copy/date changes, 32 ticks | Final 96 ticks |
| --- | ---: | ---: | ---: |
| 100 | 2.83 s | 2.75 s | 2.45 s |
| 1000 | 13.20 s | 12.03 s | 10.03 s |

The large listing returns the prompt **24.0% sooner**. Its paint time falls from
3.78 to 1.37 seconds, with 19 → 7 frames. Non-paint time falls from 9.42 to 8.67
seconds. All twelve new runs returned 0 and reported zero scheduled bulk frames
with an unfinished output line; free heap was identical after every run.

Separate diagnostic benchls phases (ticks, medians):

| Phase | Before | After copy/date changes |
| --- | ---: | ---: |
| Resolve / initial entries | 29 | 15 |
| Catalog and filename encoding | 303 | 289 |
| Format | 103 | 66 |
| Emit into terminal model | 86 | 79 |
| Yield, including painting | 226 | 205 |

See phases-before.png / phases-after.png and measurements.json. These instrumented
internal timings exclude command dispatch/final paint and include measurement
overhead; do not add them to the end-to-end table. Emit ranges overlap, so no
isolated copy-loop speedup is claimed. Formatting is the clearest non-paint gain.
Prior direct trap measurements put native PBGetCatInfo around 4.75 seconds per
1,000 entries; it remains the largest cost. Native HFS operations are unchanged.

## Changes and complete-line presentation

- Bulk listing repaint interval is 96 ticks, independent of two-tick event polling.
  First entry, pager and final prompt presentation remain immediate. Intermediate
  frames may skip many output rows, but every entry still reaches the bounded
  terminal history or redirected output. No replay queue or directory-sized buffer.
- Existing long listings emit the entire metadata/name/newline in one write before
  yielding. Terminal writes do not pump events mid-row. Existing offscreen native
  row composition/copy remains intact: skipped frames do not introduce partial
  writes of the last logical line. Explicit updates/paging are still serviced.
- Reuse one previous formatted creation date only when its exact unsigned HFS
  timestamp matches. State belongs to the current listing, not the shell; changing
  timestamps recompute the value. No date precision or metadata is removed.
- Printable runs use advancing pointers rather than repeated indexed addressing.
  The 68000 compiler emits simpler byte copying. Controls, wrapping, high bytes,
  serial input branches and retained-history behaviour are unchanged.

The per-listing struct grows 28 bytes on the target; the former 17-byte local date
buffer is removed. No new heap allocation or permanent cache. The CLX core grows
28 bytes (12068 → 12096). Directories with differing creation timestamps do not
receive the date-reuse benefit. No claim that ten seconds is fast enough or that
96-tick chunks look like smooth modern animation: this prioritises prompt return.

## Focused validation

Host ASan/UBSan checks pass:
- Actual writer versus reference: 1,500 randomized chunked streams, five widths,
  three retention limits, controls/high bytes/wrapping/anchored history.
- Actual listing loop: explicit volumes and overwritten directory IDs, hidden
  entries, escaped/long/malformed names, errors and cancellation.
- 4,000 cached/cold formatter comparisons; mixed dates, directories, flags and
  sizes; exact-key invalidation, zero timestamp and final newlines.
- Existing HFS-volume regression and old-bug mutation check; all 59,718 existing
  creation-date cases including every day boundary across the HFS range.

Normal guest checks passed:
- Ctrl+C during continuous output and at the pager: status 130 in both inspected
  `verified-cancel-automatic-*-status.png` screenshots. Runtime owned remainder 0;
  free heap 426,280 bytes (416.29 KB), essentially unchanged from 426,282 before.
- `catalog-result.txt`: relative/absolute listing and pwd on Sys6Unix and Test 2,
  spaces/escaped names, distinct BOOT/SECOND guard contents, missing-volume/path
  status 1. Disposable touch/write/remove operations completed. The reused Test 2
  fixture had two directories already present (mkdir -48, visible in
  normal-fixture.png); no claim of fresh creation for those two directories.
  Expected failed lookups report -35 and -43 rather than showing boot contents.
- `chunk-list.txt`: exactly . and .. plus all 1,000 ordered filenames, exact expected
  fork sizes and creation timestamps, final newline. `guest-validation.txt` records
  host assertions against the exported real guest output.
- `native-scroll-report.txt`: 24 bursts + 20 views with zero pixel mismatches;
  153 scrollbar staging/reference checks, zero mismatches. The timed screenshot
  normal-native-pixels.png caught the test still running; the subsequently exported
  completed guest report is the acceptance evidence.
- `normal-complete-last-line.png`: final full file0999.txt row and returned prompt.
  This is not a claim of atomic whole-screen scanout or a high-speed flicker video.

Normal validation used the additional disposable Test 2 floppy; matched timings
used the unchanged single-hard-disk workspace. Snow media-choice dialogs were
handled before guest tests; an initial keyboard script was stopped at that dialog
and rerun once Clarice was visibly ready.

## Build and scope

experimental-build/Clarice.bin is the normal application with the updated bundled
core. experimental-build/core.clx is the matching independently installable
ls/cat/df multicall image. Existing installed commands are deliberately not
silently overwritten at startup: updating the app alone does not replace an
existing /bin/ls. The Snow fixture contains both updated components. Checksums:
build-sha256.txt. No physical deployment or update-infrastructure changes.
