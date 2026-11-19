# Editor responsiveness — 19 November 2026

Snow-only, Plus/68000/4 MiB, reported System 6.0.8. Fixed maximum FFWD;
measurements below use cycle-driven **guest ticks**, not the accelerated host
wall clock. No physical-SE/PowerBook tests or public release changes.
Subsequent authorised integration/publication is recorded below.
See [method](METHOD.md) and machine-readable [results](summary.json).

## Retained changes

1. Core separates cursor-only dirtiness from changed text. Native QuickDraw
   inverts only the previously displayed and new cursor cells. Changed text,
   exposure, activation and forced redraw keep the existing buffered row path.
   No new buffer or allocation; **12 bytes** permanent state (confirmed by m68k
   symbol sizes). Normal Core 1942 MacBinary **394,752 B**, versus 394,496 B (+256).
2. BusyBox vi 1.37.0 conservatively remembers whether its displayed text is
   current. Known non-mutating movements/count prefixes can skip formatting and
   comparing the whole viewport when the viewport/offset remain unchanged.
   Every other command invalidates before and after execution; a pending edit
   remains dirty across later queued movements. Erasure/resizing/forced redraw
   invalidate too. No upstream downgrade, removed functionality or new cache.
   Normal CLX **51,392 B**, versus 51,244 B (+148); invocation peak **23,396 B**,
   versus 23,394 B (+2). Invocation state is released on exit.

Core changes are in `codex/vi-response-audit-20261119`; vi changes are separately
isolated in `codex/vi-refresh-audit-20261119`, based on accepted vi checkpoint
3a34f50. Existing `/bin/vi` remains the recovery baseline in this fixture;
`vi-final` was the normal experimental port during comparison. The accepted
normal port is now installed as `vi` in build1943 (see integration below).

## Three distinct measurements

Matched local-window workload: 120 alternating moves in an ordinary 8 KiB,
128-line file, followed by normal quit. Startup is excluded; counters start on
the first key event. The final command-line/quit responses are included. Counts
of visible responses occasionally differ by one because events/output coalesce.

| Measure | Accepted baseline | Core cursor fix only | Core + normal vi fix |
|---|---:|---:|---:|
| Vertical event → completed presentation | 232–233 ms | 182–183 ms | **83–85 ms** |
| Horizontal event → completed presentation | 180 ms | 151 ms | **52 ms** |
| Vertical vi read → first output, aggregate ticks | 1,023–1,035 | 1,018–1,031 | **287–293** |
| Vertical painting, aggregate ticks | 494–499 | 127–131 | **126–127** |
| Vertical cells touched, representative run | 29,040 | 10,001 | **10,001** |
| Horizontal cells touched | 9,840 | 401 | **401** |

Vertical end-to-end response is about **2.8× faster / 64% shorter**, not 15×.
Horizontal response is about 3.5× faster. Event dispatch itself averaged roughly
0.2–0.3 tick (3–5 ms); there is no evidence that event-queue delay dominates these
paced movements. Painting/parsing/read-to-output are **elapsed guest active-path
counters**, not exclusive CPU cycles. Read-to-output excludes waiting for the
next ordinary movement key; it must not be interpreted that way for prefixes,
ESC ambiguity waits or burst cases. Timing overhead is diagnostic-only.

Requested planning assumption: SE durations = Snow durations ×1.2. That gives
approximately **100–102 ms vertical and 62 ms horizontal**, versus ~279/216 ms
before. These are estimates, not measured physical performance or acceptance.
The one-tick/instantaneous response goal remains unmet.

The isolated 240-movement feed/draw benchmark improved **981 → 108 ticks** and
**38,400 → 480 touched cells**, with no text redraw. This is useful attribution,
not the end-to-end success claim.

## What actually executes / remaining costs

Mac Event Manager → Core key mapping/queue → BusyBox `read_key`/`do_cmd` →
`sync_cursor`, `refresh`, status → buffered ANSI → Core `screen_feed` → dirty
rows/cursor → QuickDraw. **BusyBox vi does not use ncurses.** Its existing
read-key timeout concerns ambiguous/incomplete ESC input; complete arrows did
not show a large extra delay relative to hjkl. Its existing queued-input refresh
suppression remains unchanged.

Separate vi stage probe (includes initial/exit rendering, unlike input counters):
format/compare **752 → 61 ticks**, cursor layout **62 → 73**, status/output
**298 → 284** for the vertical case. Status includes output/terminal work and
must not be added to Core timings as independent CPU work. The initial probe's
post-cleanup stdio report caused an exit crash; reporting was moved before
runtime cleanup, and those failed-run results were discarded.

Scrolling is still slow. The matched scroll sequence emits the same 140,940
bytes before/after; Core cursor-only work reduces painting **3,712 → 3,432 ticks**,
but reconstructing changed text dominates. Page painting is **816 → 786 ticks**;
there is no convincing material end-to-end page-scroll gain. Do not advertise
coalesced-response averages as a page speed-up. `20j`, arrows, page and burst
results are retained alongside the ordinary-motion measurements.

Next useful measurements, in order:

- Status generation/output and row-granularity redraw after movement. Reduce
  redundant work without suppressing useful line/status information.
- Shared full-screen text presentation during actual scrolling. vi presently
  expresses it as changed rows, not a native pixel-scroll request. Clarice's
  ordinary shell already reuses pixels; that is a different path.
- Reusing the accepted glyph compositor carefully: its glyph storage overlaps
  the editor's 64-pixel staging strip, so simply calling it here is unsafe.
  Any follow-up must account for that ownership and prove an end-to-end benefit.

No evidence supports downgrading nano/ncurses to solve this vi issue. Core's
cursor fix is reusable by full-screen clients using `app_screen_write`; ordinary
shell/grep output uses another renderer and is not claimed faster here. Nano
startup was not rebenchmarked or claimed improved.

## Correctness and limits

- 440 actual Snow QuickDraw framebuffer comparisons: both themes, partial
  clipping, dirty staging contents, clears, cursor hide/show, reverse attributes,
  intermediate cursor visits and scrolling: **zero mismatches**.
- Host ASan/UBSan: 200k parser/dirty equivalence, 100k parser bounds, 100k batched
  cursor/content full-render oracles, 300 production editor-render comparisons,
  20k inverse-run comparisons; selection delta, paste and scroll guards pass.
- vi: **52** incremental-vs-forced redraw document/cursor comparisons in Snow:
  counts/arrows, edits, undo, named paste, repeat, settings, long/horizontally
  scrolled lines, substitutions, viewport scrolling and queued edits/motions.
  The first long-input oracle sampled before vi finished processing; the final
  harness waits for stable output and sends bounded chunks.
- Local edits at file/viewport edges in both themes and exact 112-character burst
  save passed. Repeated vi runs show zero tracked survivors and stable native
  heap within matched runs (328,956 B free; largest 328,508 initially, 327,546
  after additional test-command installation). Theme/edit setup caused a one-off
  lower heap plateau in the earlier session; it did not decline per invocation.
- Normal Core1942 + normal vi: exact `20j`/20-down-arrow positioning,120-key
  movement burst, save/readback, Ctrl-C editor ownership, Command-period status130
  and subsequent shell execution pass. Final native heap/largest block remain
  **333468/331266 B** across the checks. See `normal-acceptance.json`.
- Actual vi growth failure/overflow and existing safe-save/recovery host tests
  pass. Safe HFS implementation itself was not changed.
- Existing `input-layout.py` fails its old scrollback expectation `first==5`.
  Its input-compositor body is byte-identical to baseline ceb3094 (SHA-256
  63e9d916376e7ec590f2ee93004bff1b88536d5adf91df59442dc2d3889d29e7).
  This is recorded as a pre-existing test limitation, not a passing regression.
- Disposable Snow updater handoff failed before editor testing (existing updater
  issue); diagnostic Core was installed into the stopped fixture using existing
  HFS tools. The accepted physical/shared builds were never touched.

## Later physical acceptance (not performed)

Use the same 128×64-byte fixture and font/window geometry. Compare accepted vi
and the new vi with the matching Core: alternate j/k and h/l, then 20j versus 20
down arrows, hold movement keys, cross viewport edges, page, type and undo.
Check both themes, window exposure/clipping, selection after exit, Ctrl-C inside
vi, Command-period cancellation, safe save/reopen and stable free/largest heap.
Measure guest event-to-draw and active-path counters separately; capture video
for brief blanking that screenshots cannot resolve. Do not treat FFWD host
elapsed time or the 1.2 estimate as hardware evidence.


## Authorised main integration and private publication

Source merge a503540 produces normal build2026111943, with the accepted mounted
updater and main's newer ls implementation preserved. Full supported build/seal
passes for49 standard commands; optional vi1943 is byte-identical to the tested
normal vi. Core394752 B; vi51392 B. No profiler in the distributed build.

Snow ran the complete HTTP update1942→1943 through `clarice update -y`, exited,
replaced and automatically relaunched, displaying completion. All51 final
catalogue entries match version/length/CRC; incomplete marker absent. Installed
vi validates, sort/grep and ls smoke checks pass. Exact20j/20-arrow positioning,
120-key movement burst, saves, Ctrl-C ownership, Command-period130 and shell
recovery pass on the integrated normal build. Native free/largest heap remains
333468/331266 B across the final checks. See integrated-acceptance.json and
integrated-catalog.json. The host catalogue assertion initially expected the
repository's sixth filename field; the installed catalogue correctly has five.
Only the test assertion required correction.

Integrated host tests pass:300 production render cases,100k cursor/content
comparisons,20k inverse cases, control mappings, actual vi growth failure,
recovery/save injection, repository/build consistency and native updater recovery.
The pre-existing input-layout assertion remains a documented limitation.
This successful update does not resolve or disprove intermittent error28.

Matching complete files are available at:
- HTTP: http://192.168.1.112:8089/full/index.txt
- Mounted AFP: /mnt/Macintosh Exchange/Clarice/index.txt

The existing8083 listener belongs to an older isolated test fixture and is left
untouched.8089 reuses the existing trusted-peer development server for the
MacBook, SE and PB addresses. The new index was activated after payload readback;
all previous55 files and index-before-1943.txt are preserved. Source-1943.tar.gz
contains matching source and BusyBox upstream source. No device deployment,
public release, release tag or release-candidate branch change.
