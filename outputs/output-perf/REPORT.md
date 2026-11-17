# Listing formatting and output performance

15 November 2026. Normal experimental build **2026111514**. No push/publication.

## Controlled Snow comparison

Plus/68000, 4 MB, existing System 6.0.8 image, normal speed, Monaco 9,
80 columns, native QuickDraw, paging off, 128 retained lines and 96-tick bulk
presentation. Alternating old/new variants within one process; three runs each.
The probe submits real `ls -la` through `execute()` and final prompt presentation.
Equal-depth `/tmp/base/ls` and `/tmp/next/ls` PATH fixtures contain the respective
commands; diagnostic-only switches select their old/new terminal copy kernels.

| Files | Previous median | New median | Improvement |
| --- | --- | --- | --- |
| 100 | 156 ticks / 2.60 s | 153 ticks / 2.55 s | 1.9% |
| 1,000 | 611 ticks / 10.18 s | 577 ticks / 9.62 s | 5.6% |

For 1,000 files, command-module median falls **562 → 529 ticks**; paint stays
**83 → 82 ticks**, with seven frames in every run. Loading is 36–38 ticks for
old and 37 for new; PATH lookup 16–17 ticks. These counters overlap; they must
not be summed. All 12 runs return zero, record zero partial-line scheduled
frames and finish with identical free heap (420,248 bytes).
[Raw runs](matched-paired.txt), [structured measurements](measurements.json).

Separate-boot results are retained, not substituted for this controlled result:
old warm 601 ticks; initial new warm 569/568; later new 612/611/611. An approximately
44-tick shift also affected the 100-file case after diagnostic/file-copy activity.
Its cause is not isolated. The paired comparison controls that session variation;
9.62 s is not a promise for all runs/directories. Separate `benchls` phase captures
also differed in paint work, so their totals are not an equivalent A/B comparison.
Native HFS costs are unchanged; [earlier investigation](../hfs-cost/REPORT.md).

## Focused changes and cost

- `src/commands/decimal.c`, `core.c`, `src/listing.inc`: write two right-aligned
  decimal size fields directly into each final metadata row. Remove temporary
  digit reversal/string-length/padding/copy work; keep human-readable formatting,
  HFS flags, creation timestamps and complete-row emission unchanged.
- `src/kernels.S`, `src/app.c`: bounded 68000 printable-run copy and even-aligned
  132-byte row clearing. High MacRoman bytes, odd input/output addresses, controls
  and wrapping retain their previous semantics. No overread or wider input loads.
- Diagnostic-only old/new selection and real-listing probe; absent from normal
  build. Focused formatter, writer and actual machine-code tests extended.
- Core image 12,096 → 12,376 bytes. Packaged app 938,368 → 938,880 bytes;
  resource fork 938,220 → 938,632 bytes. Normal idle free heap
  426,280 → 426,152 bytes (128-byte reduction), tracked allocation remainder zero.
  No new permanent cache, heap buffer or partition increase.

## Executed validation

- Actual 68000 Unicorn: 75,540 numeric fields, 8,911 printable-run cases, eight
  aligned clears; guards, register convention and read bounds pass.
- Host: 4,000 independent printf-oracle formatting cases; 3,000 randomized
  normal/serial writer chunks; focused HFS iterator and volume-resolution checks
  pass, including invalid paths/cancellation. No unrelated suite reruns.
- Normal Snow guest: redirected 1,000-file output is byte-for-byte equal to the
  previously accepted fixture; all 1,002 rows, sizes, dates and final newline.
  Human-readable full output also verified. [Output checks](guest-validation.txt).
- Real Ctrl+C during unpaged output and pager returns status 130 and a usable
  prompt. Final displayed line/prompt complete; intermediate output remains batched.
- Native scrolling: 24 bursts + 20 views, zero pixel mismatches; 153 scrollbar
  staging/reference checks, zero mismatch. Normal runtime owned remainder zero.
- Production and diagnostic Retro68 builds pass; `git diff --check` passes.
  Existing editor/safe-save, HFS, loader and networking implementations unchanged.

## Physical SE

Physical SE, CPU code 1, user-reported stock mode/4 MB/6.0.8 MultiFinder,
Monaco 9, 80 columns/24 rows, native drawing, unchanged 452-file directory.
The bounded creation run was cancelled cleanly (130) after 452 files; no claim
that the physical dataset equals the Snow 1,000-file fixture. Creation dates vary.
SEControl automatic heartbeats were paused during the measured benchmarks.

| Measurement | Installed build 1506 | Normal build 1514 |
| --- | --- | --- |
| Listing-loop ticks, three passes | 506, 482, 542 | 373, 323, 328 |
| Listing median | 8.43 s | **5.47 s (35.2% reduction)** |
| First-entry ticks | 41, 46, 44 | 22, 33, 36 |
| Three-run paint ticks | 505 | 163 |
| Ordinary `ls` loader + execution ticks | 602 | 366 |
| Of those, loader ticks | 35 | 42 |
| After-run free heap | 424,920 bytes | 425,970 bytes |
| Invocation-owned remainder | 0 | 0 |

Listing-loop counters exclude command loading/final report and prompt. Ordinary
invocation counter includes loading/cleanup but excludes PATH and shell's final
prompt. This is the cumulative deployment gain from 1506, not an isolated measure
of today's formatting kernels. [Raw physical evidence](physical-measurements.json).

Native synthetic scrolling remains **249 ticks / 30 frames = 7.2 fps**, worst
frame eight ticks; earlier physical result was 259 ticks. Cached C/ASM took
324/262 ticks and temporarily allocated 29,704 bytes; four release cycles stable.
The same normal application and 80×24/Monaco 9 configuration in Snow took
**225 ticks for 30 native frames (8.0 fps)**, versus SE **249 (7.2 fps)**.
SE elapsed time is 10.7% longer for this workload; throughput is 90.4% of Snow.
This one matched run does not establish a universal ratio or replace measured
I/O/startup comparisons. SEControl remained resident, with heartbeats quiet.
[Snow report](snow-scroll-report.txt), `snow-scroll-complete.png`.
Both environments have a slow drawing path; the provisional 60% assumption is
not supported by this particular result.

All synthetic checks passed. Native reference check: 22 bursts + 20 views,
zero pixel mismatch, 147 scrollbar checks with zero mismatch. Ctrl+C returns
130 during ordinary output and in the pager. History thumb dragging changes view;
no sub-frame latency or subjective smoothness acceptance is inferred from screenshots.

The user identified a completely missing scrollbar section, captured in the old
build. It did not recur after 1514 listings, paging, dragging, moving/covering/
reactivating and resizing the window. Screenshots and the reference check provide
focused evidence; the exact earlier failure cause was not isolated. Drawing
fluidity is still unacceptable and remains the next measured target.

Full shutdown/restart restored SEControl/Wi-Fi after MacTCP -23015; earlier ARP
failure/listener evidence remains in `se-connection.json`. No network/OS changes.
Private update downloaded in 63 s, staged in 4,344 ticks and verified in 2,063.
Automatic relaunch succeeded; `--check` confirmed 2026111514. Historical error 28
was not reproduced or separately fixed. Recovery application:
`Macintosh HD:.cu-000001c6-78c4021a.old`, data 0/resource 939,251 bytes, observed
in the directory (fork contents not exported/recompared this session).
`appinstall repair ls` and `repair benchls` explicitly installed the new core,
retaining `/bin/.clx-ls.old` and `/bin/.clx-benchls.old`. Other commands untouched.

## Build and remaining limits

[Normal Clarice MacBinary](experimental-build/Clarice.bin),
[matching core command](experimental-build/core.clx),
[SHA-256](experimental-build/SHA256SUMS).
Private development source: `http://<MACBOOK_LAN_IP>:8080/p14/update.txt`.
Application installation deliberately preserves existing commands. Deploying this
core requires explicit `appinstall repair`/replacement, preserving existing recovery
files. Do not confuse application version with installed `/bin/ls` version.

Nine-plus seconds remains slow. Timestamp-reuse benefit depends on directory
contents. This is bounded, complete-line chunking, not line-by-line animation.
No new nano startup claim or physical acceptance is inferred from Snow.
