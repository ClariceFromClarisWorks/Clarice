# Nano startup: same-operation HFS metadata reuse

18 November 2026; Snow Plus, 4 MiB, System 6.0.8, fixed maximum acceleration
(about 15×). Physical SE untouched. Core1816/runtime1.10 instrumentation; upstream
nano9.2/ncurses6.5 with the existing safe-save adapter. No renderer changes.

## Change and evidence

Nano's Macintosh `stat` and `realpath` adapters previously resolved the path and
then repeated the final catalog lookup. `cli_resolve_info` returns metadata already
obtained during that same resolution. It does not retain a cache, add allocations,
skip path components, or weaken symlink/volume/bounds checks. Root and parent-path
cases still fetch metadata when the intermediate result cannot be reused. Missing
metadata returns an error; the existing creation-capable resolver remains unchanged.
The additional import requires runtime1.10; existing commands remain compatible.

Normal Core MacBinary grows 256 bytes (278528→278784). Diagnostic nano package
269508→269384; loaded image199204→199152. No new permanent RAM allocation.

We reused prior nano profiling and pinned upstream source. The improvement is in
the Macintosh adapter, not a replacement for nano or ncurses algorithms.

## Matched warm measurements

`same-name.json`: both variants installed as `/bin/nano-profile` through versioned
repository updates (baseline optional revision1817, candidate1818), same Core,
terminal geometry/theme and fixtures. Services disabled while measuring. Warmup
excluded; three repetitions per fixture. TickCount marks end at first painted input
wait, before automation enters input; screenshots and host delays are excluded.
These accelerated-guest counts are comparative evidence, not physical timings.

| Fixture | Before totals (ticks) | After totals | Mean before→after |
|---|---|---|---|
| No filename |222,224,235|224,225,225|227.0→224.7|
| Existing empty file |317,284,298|261,272,272|299.7→268.3|
| 8192 bytes, 128 short lines |452,440,429|432,417,418|440.3→422.3|
| 8192 bytes, two long lines |352,353,355|341,341,350|353.3→344.0|

The empty-file baseline varies in unrelated phases; do not attribute its entire
31-tick total change to this patch. File-open phase savings are repeatable across
both this test and the earlier alternating differently-named comparison (`paired.json`).

| Phase, mean ticks | Empty before→after | Short before→after | Long before→after |
|---|---|---|---|
| Loader |107.7→103.0|106.0→101.3|102.0→102.0|
| ncurses initialisation |32.7→27.0|27.0→27.0|27.0→27.0|
| Setup |35.0→30.0|30.0→30.7|30.0→30.3|
| File opening/buffer construction |74.0→62.3|127.7→114.7|134.0→124.0|
| Initial layout |24.7→24.7|50.0→50.3|27.0→27.7|
| First refresh |4.7→4.0|80.7→81.0|16.7→15.7|

Other submission/runtime bookkeeping is included in total. Rendering/allocator/read
counters in raw output are nested in these phases, not additional additive costs.
Earlier profiles show actual FSRead only ~4–8 ticks, versus ~60–81 ticks in file
parsing, so raw file transfer is not the dominant file-opening cost.

Owned peaks are identical:74572 no file,75112 empty,101736 short,95662 long.
All24 same-name runs report remaining=0. Native free heap ranges364160–364174
before /364174–364202 after; largest block360464–360574 /360450–360500.
These small fluctuations are recorded rather than claimed to be exact equality. No permanent cache or allocation added.
The small, consistent 8KiB gains justify retaining this bounded change provisionally;
physical performance acceptance remains pending. No new cold-boot claim.

## Correctness

- Production resolver host ASan/UBSan: root, relative/absolute, dot/dotdot, symlink,
  cycles, volume names/spaces, missing/error results, no stale metadata. Ordinary
  `/tmp/file` catalog calls3→2. Existing volume/rename host regressions pass.
- Actual safe-save host implementation:27 fault points,27 interruptions, both forks
  and metadata, short write, disk full, rollback, locked/read-only, cancellation,
  repeated saves and no retained allocations pass.
- Guest: two mounted volumes, relative/absolute and link traversal, failed unknown
  volume reads/writes, and unchanged distinguishable guards (`guest-paths.json`).
- Actual nano saves existing/new/second-volume files, reopens and discards an edit,
  restores terminal, preserves TEXT/ttxt and releases allocations (`save-check.json`).
  Resource-fork fault verification here is host evidence; no new physical or guest
  resource-fork fixture claim. Existing accepted guest safe-save evidence preserved.
- Normal and profiling Core builds completed; normal/profile import tables201/203
  pass. Private Snow currently runs diagnostic Core1816, not a published release.

## Remaining bottleneck / next focused experiment

Ordinary8KiB startup remains~422 ticks and misses the three-second target. Shared
loading costs~101 ticks; file setup/construction~115; final ncurses refresh~81,
plus setup/layout. Existing paint counters show intermediate screen drawing during
initial ncurses state changes (about19–20 draws/77rows on short-line startup).
Investigate whether those paints can be deferred until the required refresh/input
boundary, preserving interactive refresh and cancellation. No batching change or
new buffer has been implemented in this batch.

Reproduction helpers `measure.py` and `collect.py` reuse the existing Snow key tool
and authenticated local test client. Fixtures: empty existing file,128 lines of
63 ASCII characters plus LF, and2 lines of4095 characters plus LF. Keep services
off for measurements; collection enables the already-configured Telnet service.
Secrets, emulator images and diagnostic packages remain in ignored work storage.
