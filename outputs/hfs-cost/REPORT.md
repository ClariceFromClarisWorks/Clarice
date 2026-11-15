# Native HFS catalog cost — 15 November 2026

Snow Plus, 4 MB, existing System 6.0.8 image; normal Clarice **2026111513**.
The physical SE was not contacted. No production code changes, new cache,
OS settings changes, application rebuild, push or publication.

## Result

The roughly five-second catalog cost is reproducible inside the native File
Manager path. Asynchronous catalog calls offer **no measured improvement** here.
The sequential walk reads each catalog block once, not repeatedly within a pass.
There is no demonstrated redundant native read to remove safely in Clarice.

| Experiment, 1,000 entries/calls | Three runs, ticks | Median seconds |
|---|---|---|
| Production `benchscan`, first/repeated passes | 379 / 323 / 322 | 5.38 |
| Direct synchronous catalog walk | 297 / 302 / 300 | 5.00 |
| Direct asynchronous catalog walk | 300 / 301 / 300 | 5.00 |
| Repeated first-entry lookup, cache-hot control | 173 / 170 / 171 | 2.85 |

Guest TickCount / 60; not host wall time or physical SE measurements. The probe
includes parameter setup, a small metadata fingerprint and normal cancellation
polling. It excludes path resolution, command loading and rendering. Production
`benchscan` also includes path/name handling; its first pass is not a cold-boot
disk test. Earlier per-call instrumentation measured 284–289 ticks in the native
trap itself. Do not count the probe/production difference as an optimisation.

All six sequential probe passes returned 1,000 entries, error 0 and the same
name/ID/flags/creation-date/fork-length fingerprint `c6c2ccc1`. Async calls had
zero pending-wait iterations: they completed before the caller could overlap
other work. This is a finding for this local HFS configuration, not every driver.

## Where the time goes

- Enabled Snow's existing `SNOW_SCSI_TRACE_CDB=1` diagnostic, without rebuilding
  or patching the emulator. Timing comparisons used the same stock Snow binary.
- The production three-pass capture contains 1,206 reads: **1,185 catalog reads**
  and 21 other reads. This interval includes command loading and path lookup;
  it must not be divided by three and labelled the isolated scan cost.
- The direct probe exposes **six identical 338-read cycles**, each reading
  **338 distinct 512-byte catalog blocks (169.00 KiB)**. Each cycle is bounded by
  successive reads of the first leaf block, LBA 677; fixed invocation/path work
  is outside this comparison. Sync and async use exactly the same block sequence.
- Catalog extents were identified from HFS metadata **with Snow stopped**:
  LBAs [626,1137) and [4977,5488), 511.00 KiB catalog logical size. All 338
  cycle blocks lie in those extents. No live-image mount or raw catalog reader
  was introduced into Clarice.
- After the first hot lookup loads its path, the trace has no further reads
  during the three hot runs. Those 3,000 calls still take 514 ticks. Thus native
  processing remains substantial even without fresh disk traffic.
- The 2.15-second sync-versus-hot median difference is a useful contrast, **not
  an exact disk-time subtraction**: repeated-index and sequential searches do
  different File Manager work. Snow's image backend uses host memory mapping;
  the guest still pays File Manager, SCSI-driver and transfer processing costs.

Apple's indexed enumeration guidance and MoreFiles' `IterateDirectory.c` use
the same native walk, restoring the parent ID on every call. The broader
`PBCatSearch` API requires System 7/extended File Manager support; it is not a
System 6 baseline substitute. See [Apple FL31](https://leopard-adc.pepas.com/technotes/fl/fl_31.html).

Changing the system-wide disk cache was not tested or silently applied. A raw
catalog implementation or persistent directory cache would introduce additional
correctness/invalidation obligations. Neither is justified by this focused test.
Keep the current native path; the next lower-risk target is the remaining
formatting/output/dispatch work outside these native calls. No new work started.

## Verification and changes

- Added only `tests/commands/catalog-cost.c`, an unbundled, read-only 68000 probe,
  plus this evidence and the current-state update. Its CLX image is 1,720 bytes;
  it uses stack storage and no heap cache. Async parameter/name storage remains
  live until completion, including cancellation. Only the 1,000-file fixture was
  performance-tested. The probe is bounded to 1,001 entries, not a general tool.
- Normal guest real Ctrl+C passes with paging off and while stopped at the pager:
  prompt restored, `echo $?` = **130** in both. Reused the existing keyboard-patched
  Snow for these functional checks only; stock Snow's modifier limitation remains.
- Chunked output advances before completion; final `file0999.txt` line and prompt
  are complete. Screenshots below. Drawing code is untouched. Prior full pixel/
  scrollbar checks remain in [chunk-perf](../chunk-perf/REPORT.md); they were not
  rerun, and still images alone do not prove absence of every transient artifact.
- Runtime after cancellation: free heap **426,280 bytes (416.29 KiB)**, largest
  block 426,144, tracked remaining 0. No persistent footprint change.
- Focused ASan/UBSan production listing-loop and volume-resolution host tests
  pass, including explicit volumes, overwritten parent IDs, spaces, malformed
  names, cancellation and fail-closed invalid/ambiguous paths. No new two-volume
  guest claim; earlier unchanged-code coverage is retained.

Evidence: [raw probe results](catalog-cost.txt), [measurements](measurements.json),
[catalog mapping](catalog-extents.json), [read pattern](read-pattern.json),
[production SCSI trace](scan-scsi.log), [probe SCSI trace](probe2-scsi.log),
[probe screen](probe2-screen.png), [unpaged Ctrl+C](verified-cancel-automatic-off-status.png),
[pager Ctrl+C](verified-cancel-automatic-screen-status.png),
[final line](normal-complete-last-line.png), [runtime](normal-final-runtime.png),
[listing host tests](listing-host.txt), [volume host tests](volume-host.txt).

Reproduction: compile `tests/commands/catalog-cost.c` with the existing Retro68
68000 freestanding flags and `scripts/pack-command.py`; install it only as a
disposable `/tmp/catalog-cost` while the test image is offline. In the guest run
`/tmp/catalog-cost /mnt/Sys6Unix/Bench/n1000 > /tmp/catalog-cost.txt`.
No application/runtime rebuild is required. Existing build 2026111513 remains
the [current experimental build](../chunk-perf/experimental-build/Clarice.bin).
