# Clarice current state

15 November 2026, Adelaide. Normal build **2026111517** is now installed on the
physical SE through private Wi-Fi update, with automatic relaunch and version
check. Previous application and replaced `/bin/ls`/`benchls` remain recoverable.
Error 28 did not recur; its historical cause remains unknown.
ROADMAP.md is provisional history, not current authorisation.

Development checkpoint: `codex/se-performance-2026111517`; no release or tag.
Source, focused tests and benchmark evidence are retained; generated applications,
media, private connection settings and interim captures remain local.
[Reproduction instructions](jump-perf/REPRODUCE.md). No renderer changes or new
hardware tests in this checkpoint task.
Clean build passes; both application CODE resources match the accepted build.
The fresh package regenerates older bundled utilities, so it is not byte-identical
or newly hardware-qualified; [comparison](jump-perf/checkpoint-validation.json).

## Large history jumps — 15 November 2026

- Normal build **2026111517** reuses unused space in the existing row bitmap for
  installed Monaco 9 ASCII cells. A bounded 68000 loop composes four cells into
  three bytes. Other fonts/extended characters, selection, prompts/cursors and
  forced redraws retain QuickDraw; editor ownership invalidates the stored cells.
  No new bitmap allocation, partition increase, renderer preference or idle work.
- Three alternating old/new runs in one diagnostic executable, **24 large jumps**
  each: Snow directory-style **331 → 249 ticks (24.8% less time)**, dense text
  **1,071 → 645 (39.8%)**. Stock physical SE: **370 → 286 (22.7%)** and
  **1,159 → 731 (36.9%)**. Same Monaco 9/80×24 setup; quiet SEControl.
  Physical mean frames **257 → 199 ms** and **805 → 508 ms**, respectively.
  First-use cell preparation costs 4–5 ticks; included in the measured draws.
- Snow actual step-and-pause dragging: worst draw **14 → 11 ticks**; gestures
  covered different view ranges, and host final-change medians are within capture
  granularity. **Fluid continuous dragging is not yet established.** Dense history
  remains slow; composition and per-row native presentation remain the focus.
  The existing two-tick post-paint drag cadence is unchanged.
- Host: 356 actual M68000 compositor cases, 510 glyph-builder cases, 532 row
  comparisons, selection delta and 1,560 cursor/view guards pass. Snow light/dark
  dense-text checks each have zero mismatches across 22 bursts + 20 views and
  147 scrollbar checks. Physical diagnostic dense-text/scrollbar checks also pass.
- Normal Snow: Ctrl+C output/pager statuses 130, selection works, nano opens the
  8 KB fixture and exits, then native pixel/scrollbar checks pass; owned remainder
  zero. Physical normal build: actual thumb dragging and selection work, complete
  scrollbar retained, Ctrl+C output/pager statuses 130, mouse button released.
  No high-speed physical video/flicker latency measurement; no cursor changes.
- Normal resource fork **+858 bytes** over 1515. Physical sampled free heap
  **414.92 KB (424,878 bytes)**, 868 bytes below the previous after-listing sample;
  owned remainder zero. Paired jump test heap delta zero on both machines.
- Wi-Fi update automatically relaunched and verified **2026111517**. Prior normal
  app: `Macintosh HD:.cu-000005db-78c4021c.old` (resource 938,856 bytes); diagnostic
  1516 also retained. Earlier backups preserved; no new error 28 observed.
- Prior 1515 native BlockMove/even-aligned row-comparison optimisation remains;
  its separate physical 30-frame result was 248 → 174 ticks.
  [Prior report](frame-perf/REPORT.md). No new nano-startup or HFS throughput claim.
- Outstanding: previously recorded dark-to-light preference invalidation and
  updater viewport-visible `Y/n`/explicit `-y`; no work on those in this task.
  [Report/build](jump-perf/REPORT.md), [Snow evidence](jump-perf/measurements.json),
  [physical evidence](jump-perf/physical-measurements.json).
- Next recommendation, not implemented: measure and remove redundant row clearing
  before the four-cell compositor's full-byte writes. Physical dense-frame text
  work is about 235–239 ms, presentation 155–166 ms, layout/history 11–15 ms.
  The text counter combines clearing, glyph work and setup; the clearing share
  is not separately measured. [Analysis and verification plan](jump-perf/REPORT.md#next-focused-opportunity-checkpoint-review).

## Listing throughput — 15 November 2026

- Build **2026111514** removes temporary numeric formatting/copy work and uses
  bounded 68000 printable-run copying and aligned row clearing. No HFS changes,
  new permanent cache, partition increase or drawing/cursor redesign.
- Controlled alternating old/new Snow runs, real `ls -la` through final prompt:
  **1,000 files 10.18 → 9.62 s (5.6%)**; 100 files 2.60 → 2.55 s. Three-run
  medians; equal-depth command paths, same session/dataset/geometry. Module median
  562 → 529 ticks; paint 83 → 82; seven large-listing frames throughout.
- Chunked 96-tick bulk presentation, complete rows, immediate first/pager/final
  display and two-tick input polling remain. Earlier cadence/output work reduced
  1,000-file time 13.20 → 10.03 s; [previous evidence](chunk-perf/REPORT.md).
- Native HFS sync/async walks both measured 300 ticks, with 338 distinct catalog
  block reads; no safe speedup was demonstrated. [HFS evidence](hfs-cost/REPORT.md).
- Actual 68000 tests: 75,540 numeric fields, 8,911 copy cases, eight row clears;
  host formatter/writer/HFS/volume regressions pass. Normal guest output matches
  accepted data byte-for-byte; Ctrl+C status 130 in output and pager. Native scroll
  pixel mismatches zero (24 bursts + 20 views); 153 scrollbar checks also match.
- Normal idle free heap **416.16 KB**, 128 bytes below prior build; invocation
  owned remainder zero. Core +280 bytes; application resource +412 bytes.
  Renderer remains automatic native QuickDraw, with no renderer preferences.
- Separate-run timing shifts of about 44 ticks remain unexplained; use the
  interleaved paired result, not the fastest isolated run. Nine-plus seconds is
  still slow; timestamp reuse varies with content. No new nano measurement.
- Physical SE (CPU code 1, user-reported stock mode/4 MB/6.0.8): unchanged
  452-file directory, old build 1506 → normal 1514 listing median **8.43 → 5.47 s
  (35% faster)**. Ordinary loader/execution counter **10.03 → 6.10 s**; this excludes
  PATH/final prompt. Cumulative deployed improvements, not today's isolated gain.
  After-listing heap 415.99 KB (425,970 bytes), owned remainder zero; Ctrl+C 130 in
  output and pager. Setup was cancelled at 452 files; do not call it a 1,000-file test.
- Physical missing scrollbar section was captured in old build 1506. It has not
  reappeared in 1514 after listings, pager, thumb dragging, overlap/focus switching
  or resizing. Native check: 22 bursts + 20 views, zero pixel mismatches; 147
  scrollbar checks, zero mismatch. This is focused evidence, not a universal fix claim.
- Full power cycle restored Wi-Fi after MacTCP -23015; cause unproved. New update
  downloaded in 63 s, staged in 72.40 s, verified in 34.38 s, automatically relaunched.
  Previous app: `Macintosh HD:.cu-000001c6-78c4021a.old` (resource 939,251 bytes).
  No updater/firmware changes made. Other installed commands preserved.
  [Current report/build](output-perf/REPORT.md), [physical measurements](output-perf/physical-measurements.json).

## Implemented and tested scope

- Native System 6 shell, external `/bin` utilities, HFS `/mnt`, symlinks, PATH,
  preferences, bounded history/scrollback and finite spool pipelines. These are
  Unix-like subsets, not full POSIX compliance or live Unix processes.
  [Userland](userland-next/TEST-REPORT.md), [preferences](preferences-text/TEST-REPORT.md).
- QOL-1 cross-volume resolution, QOL-2 input batching/fail-closed overflow and
  QOL-3A changed-region selection have focused host/Plus guest evidence:
  [filesystem](qol1/TEST-REPORT.md), [input](qol2/TEST-REPORT.md),
  [selection](qol3a/TEST-REPORT.md). Physical cursor rendering is working; Snow
  artifacts alone do not justify changing it.
- Independent CLX command installation preserves existing applications at startup;
  explicit replacement/repair retains recovery and checks runtime compatibility.
  [Task 2](task2/TEST-REPORT.md), including 43/43 guest acceptance checks.
  Installation-verified fast-launch code and explicit strict verification are
  present; do not infer a new nano benchmark from host tests in clx-fast/.
- Genuine GNU nano 9.2 tiny and ncurses 6.5 edit and save in Snow. Same-volume
  staged saves preserve the resource fork and applicable Finder metadata, with
  fault/recovery tests. [Safe-save evidence](safe-save/TEST-REPORT.md).
  This is recoverable replacement, not atomic or guaranteed power-loss safety.
- Runtime allocation is demand-driven within available application heap and
  recovery headroom; the former universal 96 KiB ceiling is obsolete. Focused
  demand/exhaustion tests reached 346,080 bytes with zero retained allocations.
  Stream update/seek/error services pass focused guest tests.
  [Runtime evidence](runtime-completion/PROGRESS.md).
- Serial 57600 8N1, local PTY shell and bounded E-Kermit binary transfers have
  host/Snow coverage. [Serial-3](serial-3/TEST-REPORT.md). Physical short echo,
  reconnect and binary transfer evidence is in [hardware progress](hardware-update/PROGRESS.md);
  tested USB serial setup requires host pacing of 2 ms per byte for Kermit.
- SEControl is separate development infrastructure. Physical Wi-Fi screenshots,
  keyboard/mouse control, Finder/Clarice launch and disconnect recovery work.
  [Graphical-control report](hardware-control/graphical/REPORT.md).
- External HTTP wget uses MacTCP, bounded streaming, incomplete-file cleanup,
  errors/cancellation and progress displays. Physical CLX download, verification
  and appinstall have passed. No HTTPS/authenticated internet update claim.

## Performance and limits

- Normal application partition request is 768 KiB, with 64 KiB stack reserve.
  History payload is 16,896 bytes; normal rendering uses no bitmap cache.
  Diagnostic cached-renderer comparisons allocate 29,696 bytes temporarily.
  Command/editor working allocations are released; there is no resident nano.
- Nano's three-second target remains unmet. Latest recorded controlled repeated
  dispatch-to-ready times after small-allocation-page optimisation: empty 11.15 s,
  8 KiB 14.07 s (previous 12.17/17.68 s). Temporary 8 KiB workload peak increases
  about 21 KiB; no idle arena. [Timings](nano-startup/REPORT.md).
- Native APPL/CODE alternatives were reviewed, not installed as a general worker
  backend. [Loader review](loader-review/REVIEW.md). Separate command partitions,
  general live pipes, complete awk system()/command pipes, SDK, Telnet and SSH
  remain unimplemented or incomplete; no new work is authorised by this list.
- Physical SE: user reports 4 MB, System 6.0.8, MultiFinder and BlueSCSI networking.
  MiniPerformer+ remains installed but was bypassed with programmer switches;
  subsequent CPU code is 1, consistent with 68000. Earlier CPU-code-3 timings
  are accelerator-state-unverified and must remain separate.

## Current updater acceptance

Private HTTP manifest/package uses CRC-32, lengths and native compatibility;
CRC detects accidental corruption and does not authenticate the source. Both
forks are staged beside the application. An independent native helper retains the
previous application, commits only after the running app exits, and relaunches.
The System 6 unavailable HOpenResFile trap and documented resource-fork scratch
bytes have been addressed; native launch uses a working-directory reference.

Snow full normal-build configured-source workflow passed: 2026111402 to
2026111502, automatic relaunch, --check and pwd. Extracted previous/new forks
and applicable Finder metadata match (hardware-update/snow-full-integrity.json).
Snow HTTP transport is an HFS fixture, not network acceptance.

Physical SE **passed** the same normal `clarice update` workflow over MacTCP/Wi-Fi.
Downloaded 919.50 KB in 58 s (15.85 KB/s); staged readback 2113 ticks (35.22 s).
After confirmation the independent helper installed and automatically relaunched
build 2026111502; --check and pwd succeeded. Active app is
`Macintosh HD:Clarice Update`. Retained previous target:
`Macintosh HD:.cu-00000a12-78c4020e.old` (data 0/resource 941423 bytes).
Original Clarice and Clarice Dev also remain unchanged. Configured source:
`http://<MACBOOK_LAN_IP>:8080/update.txt` in `/etc/clarice/update.conf`.

SEControl's heartbeat timed out during handoff; user reopened SEControl only,
then remote commands verified Clarice's automatic relaunch. Exact relaunch latency
was not measured; physical forks were not exported for host byte comparison.
The one-time binUnpk bootstrap needed its omitted bundle flag restored using a
disposable probe; no new production utility was added. Subsequent updates need
neither binUnpk nor manual Finder/restart operations.

[Tested package and command](hardware-update/accepted-build/),
[physical result](hardware-update/physical-update-acceptance.json),
[full evidence and limitations](hardware-update/PROGRESS.md).
Existing native rollback and 482 host fault scenarios remain relevant evidence.
No power-loss safety, authentication, public update service or peak-memory claim.
[Pending hardware checks](PENDING-HARDWARE-TESTS.md). No further updater expansion.

## Physical performance programme — in progress

- Full SE shutdown/restart restored Wi-Fi after MacTCP -23015. Exact network
  cause unproved. CPU code 1 rechecked after power cycle. First launch
  main-entry-to-ready 331 ticks (5.52 s), excluding OS loading.
- Static binary-search CLX import table preserves 180 production/181 diagnostic
  bindings. Physical `cat` import resolution **16 → 2 ticks**; observed loader
  **45–56 → 35 ticks**. After-run heap 424918 bytes, owned remainder zero.
  No persistent cache or allocation; isolated lookup code/constants shrink
  6970 → 3438 bytes. ASan/UBSan lookup checks, 61 loader/installer scenarios
  and validation of 24 real command images pass.
- Snow repeated nano: empty **449 → 434 ticks** (7.48 → 7.23 s), 8 KB
  **623 → 605 ticks** (10.38 → 10.08 s); loader **203 → 187 ticks**.
  Import phase 17–18 → 3–4 ticks; metadata validation remains 85 ticks.
  Before/after diagnostic configurations have different heap layouts; total
  change is not an isolated measure of the lookup improvement. Three seconds
  remains unmet. Physical nano measurements remain pending.
- Snow safe-save regression preserves resource fork, type/creator, flags and
  creation date. Repeated nano exits return tracked allocations to zero.
  Four selection probe cases pass with at most one tick/highlight, no pixel
  mismatch or heap delta. Synthetic one-tick input lost 24/240 characters;
  that stress case is **not passed**; physical confirmation remains pending.
- Physical scroll benchmark: 30 frames native 259 ticks, cached C 500, cached
  assembly 460; worst frames 8/16/15 ticks. Four cache release cycles stable,
  zero test failures. No rendering preferences or cursor changes made.
- Host-only quiet mode and bounded 120 s listener timeout added; 14 focused
  host tests pass. Screenshots work again after the power cycle.
- Pre-exit update warning is implemented and visible on physical hardware.
  Physical profile package download 56 s; staging 4023 ticks (67.05 s), readback
  2088 ticks (34.80 s). Automatic relaunch then unexpectedly quit with error 28.
  Opening the installed application from Finder succeeded; --check confirmed
  build 2026111506 and `cat` worked. Cause not isolated. The later normal 1514 update/relaunch passed; see above.
- The same profile package passed Snow automatic update/relaunch; extracted
  old/new forks and metadata match expected contents. Incremental Retro68 build
  dependency on the version stamp fixed after a mismatched package was safely
  rejected. Normal experimental build 2026111507 was not deployed; it is superseded by
  the accepted normal 1514 deployment above. No diagnostic nano installed on the SE.

Evidence: [measurements](physical-perf/measurements.json), screenshots and focused
logs in [physical-perf](physical-perf/). User priority is now Snow interaction and
rendering performance first, physical retest after meaningful gains, then updater
error 28. Keep the working application and previous versions.

### Snow interaction/rendering — 15 November

- These earlier runs were Snow-only while the SE was shut down. The provisional
  60%-speed estimate was not measured; current matched scrolling gives 90.4%
  throughput. Neither number is a general conversion for other workloads.
- Source audit found earlier scrolling/selection implementations still present
  relative to v0.4.0; installed SE binary/preferences equivalence remains unproved.
  Typing after selection now clears only the affected region: eight matched
  guest cases fall from 32–45 to 3–4 ticks with identical pixels. No allocation.
- ncurses uses the existing strip to batch rows and inverse spans; parser avoids
  dirtying rows solely for attribute bytes. 8 KB first paint 118 → 96 ticks;
  cold total ~12.27 s, warm empty 6.80 s, warm 8 KB 9.65 s. Three seconds unmet.
  Nano edit/search/save passed, both forks/metadata preserved, exit allocations
  zero. These preceding results are retained, not rerun in the drawing pass.
- Matched same-build drawing probe identifies redundant full cached-screen
  presentation and duplicate Control Manager drawing. Reuse native pixel scroll
  for cached presentation, copy exposed rows, retain the staged scrollbar bitmap.
  Full redraw and covered/clipped fallback remain. No new persistent allocation.
- Complete 100-file ls median: native 173 → 172 ticks (effectively unchanged),
  cached C 191 → 177, cached assembly 186 → 174 (6.5% shorter). Assembly output
  sequence 353 → 197 ticks (44.2% shorter); history 281 → 218 (22.4%). Native
  history 231 → 199 (13.9%). These are guest timings, not physical acceptance.
  Assembly worst history frame is still 18 ticks; smooth drag latency unproven.
- Light and dark native/cached pixel checks pass: each 24 bursts + 20 views, zero pixel
  mismatches and 153 scrollbar staging/reference checks with zero mismatches.
  Focused host selection and 1560 scroll-guard checks pass. Ctrl+C stops native
  and cached assembly 1000-file listings, paged/unpaged, with status 130 and prompt
  restoration. Stock Snow modifier delivery failed; existing patched Snow fixed
  the automation without Clarice input changes. Normal quick scroll/ring/copy
  checks pass; four cache release cycles stable (29704 bytes including overhead).
  Restored native heap 425716 bytes, tracked remaining zero. Guest API 0607 is
  the previously documented 6.0.8 media/resource discrepancy, not a new OS claim.
- Native strip batching and scrollbar changed-scanline-copy experiments were
  measured and discarded. Only the invalid-row cursor-bounds correction remains
  from the strip experiment. No cursor design change.
- Normal experimental build 2026111509 compiled; resource 938732 bytes (+238
  versus 1508), no new heap cache. Matched probe heap stable within renderer;
  whole probe delta -2 bytes. This stage was not deployed; later 1514 hardware
  acceptance is recorded above. No push or publication.

[Drawing evidence and limitations](draw-performance/REPORT.md),
[matched counters](draw-performance/matched.txt),
[normal experimental build](draw-performance/experimental-build/Clarice.bin).
Earlier interaction/editor evidence: [Snow measurements](snow-performance/measurements.json).
### Listing, enumeration and scrolling — 15 November

- Normal experimental build **2026111511**; Snow Plus/4 MB/System 6.0.8 only.
  The SE was unchanged during that earlier stage; current deployment is above.
  No push/publication.
- Earlier duplicate executable-selection removal remains: complete warm native
  ls medians 10 files 118→108 ticks, 100 files 173→163. Loader checks unchanged.
- Deeper 1,000-entry scan counters put 284–289 ticks (~4.75 s) in native HFS
  calls, versus 10–13 setup and 24–33 name handling. Cost per 100 entries stays
  roughly constant. Later [HFS investigation](hfs-cost/REPORT.md) shows 338 unique
  catalog reads per scan and substantial cache-hot CPU cost; exact CPU/I/O split
  remains unmeasured.
- Reuse the synchronous catalog parameter block and avoid an intermediate name
  copy; restore explicit volume/parent/index every call. Normal warm scans now
  **323 ticks (5.38 s)** versus prior 336–338; cold first pass369. Same-image
  controlled reuse experiment supports a ~4% gain. Still too slow.
- Native small-scroll heuristic now accounts for per-row Toolbox costs. Matched
  four-row rendering: six frames 82→57 ticks using pixel reuse; large moves can
  be slower, so retain patching there and leave cached weighting unchanged.
  No new cache/idle allocation; normal resource +13 bytes, core CLX -20 bytes.
- Listing already streams bounded rows; captured output advances before the
  scan completes. More frequent painting alone was rejected: 100-file internals
  110→213/336 ticks at six/three-tick intervals. Smooth single-line bulk display
  remains unmet. Prior actual drag captures remain ~250–298 ms native and
  ~195–254 ms cached; no new end-to-end drag-latency claim this pass.
- New ASan/UBSan listing-loop tests and existing fail-closed volume checks pass.
  Actual two-volume guest fixtures pass for spaces, hidden/encoded names, cd/pwd,
  relative/absolute listings, disposable operations and missing-path errors.
  Native pixel checks: 24 bursts/20 views, zero mismatches; 153 clean scrollbar
  checks. Four Ctrl+C cases pass (native/cached, paging off/screen), status130.
  Final tracked allocations zero. No cursor design or loader validation changes.
- Follow-up catalog evidence is now above; async gave no gain. Retain chunked
  presentation and investigate remaining non-native work before painting more often.

[Current evidence](catalog-perf/REPORT.md), [experimental build](catalog-perf/experimental-build/Clarice.bin).
Earlier [dispatch/drag evidence](list-drag/REPORT.md) retains its original measurements.
