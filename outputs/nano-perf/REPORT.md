# Nano end-to-end measurements — 14 November 2026

Snow v1.5.0, Macintosh Plus, 68000, 4 MB, System 6.0.8; no overclock or
fast-forward. Existing safe-save image and actual GNU nano 9.2 tiny / ncurses 6.5.
No physical-hardware acceptance, push or publication. **Three-second target not met.**

## Measured result

| Launch | Before | After |
|---|---:|---:|
| Empty existing file, first nano after guest boot | 15.65 s | 15.28 s |
| Empty file, repeated | 12.52 s | 11.98 s |
| 8 KiB file, first after the empty runs | 18.12 s | 17.70 s |
| 8 KiB file, repeated | 17.93 s | 17.50 s |
| 8 KiB file, first nano after a separate guest boot | not measured | 20.92 s |

These are guest TickCount intervals from shell dispatch to the first painted,
input-waiting editor, not loader-only times or invocation lifetime. The initial
shell command echo/draw precedes the dispatch marker and is excluded; therefore
these are slightly optimistic versus Return-to-ready acceptance. Typed `z` and
screenshots confirm the resulting editor accepts input. Each run cancelled without
saving, leaving the same fixture. Single cold observations, not statistical estimates.
The 8192-byte LF text fixture has ordinary 65-character lines.
The separate cold 8 KiB run repeats at 17.50 s; its raw evidence is cold0/1.txt.
Cold file open takes 315 ticks, ncurses 77, first paint 96 and loader 537.

### Stage breakdown (ticks, 60 ticks/second)

| Stage | Before cold empty | After cold empty | After repeat empty | After repeat 8 KiB |
|---|---:|---:|---:|---:|
| Image/header reading | 61 | 60 | 61 | 60 |
| Header/checksum validation | 356 | 329 | 329 | 329 |
| Import resolution | 18 | 17 | 17 | 17 |
| Relocation checks/application | 21 | 22 | 22 | 22 |
| Loader preparation/close | 102 | 108 | 0 | 0 |
| Portable runtime entry to upstream main | 1 | 1 | 0 | 1 |
| Native screen plus ncurses initscr | 80 | 78 | 27 | 26 |
| nano open_buffer (path/open/read/buffer creation) | 89 | 92 | 89 | 303 |
| Main-loop entry to first painted input wait | 8 | 8 | 7 | 95 |
| Total dispatch to ready | 939 | 917 | 719 | 1050 |

Unlisted time includes shell dispatch, nano shortcut/window setup and work between
markers. The file stage is not disk-only timing. Loader event servicing is included
in the loader stages, not additive: before repeat empty 41 ticks; after 19 ticks.
`measurements.json` and raw before/after text preserve every observation.

## Small optimization retained

- Same exact FNV-1a checksum, with cheaper equivalent 68000 doubling and word-sized
  shift for the high-byte term. No integrity bypass, table, cache or format change.
- Poll cancellation/events per 4096 checksum bytes instead of 1024. Total checksum
  validation falls from 355–357 to 328–329 ticks (about 7.5%, 0.45 s). Cancellation
  during loading returns 130 before entering nano, restoring the shell and heap.
- No new production allocation. Optional diagnostic state compiles out of the
  normal build. Normal command loading still validates imports and relocations.
- Plus Option-W maps to the existing search control while an editor owns input,
  matching the existing Option-O/X fallbacks. No cursor/rendering algorithm changed.

Validation alone still takes ~5.5 s. Reading/constructing the 8 KiB buffer takes
~5.1 s, and its first screen takes ~1.6 s. Upstream reads byte-by-byte through getc;
that identifies a future profiling area, not proof that getc alone explains the
file-stage time. No speculative file-reader/ncurses rewrite attempted.

## Editing, saving and cleanup

- Typed `performance test ` into the 8 KiB file; exact prefix and all original bytes
  verified after saving. Forty Down events scroll beyond the initial viewport.
  Option-W / `trial` / Return finds the word and leaves the cursor on it.
- Single-character event-handling-to-paint: 5 ticks empty, 6 ticks in the populated
  editor (~83–100 ms), unchanged before/after. This excludes host input transport
  and prior event-queue delay. Mixed editing/scroll/search/save run: 66 paint batches,
  sum 319 ticks, max 62 ticks; this is not a per-operation latency distribution.
  Screenshots capture successful scrolling/search; dedicated per-scroll/search
  latency percentiles were not instrumented.
- Native safe-save interval: 116 ticks (1.93 s) for the edited 8 KiB file; 93 ticks
  (1.55 s) for the small resource-bearing fixture. Includes write/close/replacement,
  excludes time in the filename prompt. Both editor exits status 0.
- `save-note` data exactly `perf ` plus prior contents. Resource fork, type/creator,
  Finder flags and creation date unchanged; no transaction siblings remain.
  `validation.json`: seven diagnostic checks plus two normal-build checks passed. Exported Finder position is not a
  nonzero-placement test; retain the previous safe-save qualification.
- Portable allocation peaks: 68,755 bytes empty, 83,748 bytes opening 8 KiB,
  84,025 bytes for the editing run. Remaining tracked allocations zero after every
  normal exit and cancellation. Diagnostic image 198,064 bytes, separate from those
  peaks; native terminal screen and stdio/HFS allocations are also separate. This
  is tracked portable peak memory, **not a continuously sampled whole-process peak**.
- Four launch/cancel runs and both successful save runs all return to free heap
  397,600 bytes, largest block 393,070. Loader cancellation has the same values.
  No retained-memory growth observed in this bounded sequence; not an endurance
  or fragmentation guarantee. Before-build free heap was 32 bytes higher.

## Focused regression evidence

- Exact 68000 checksum execution: 2,028 reference-equivalence cases, including
  seeds, odd addresses, tails, memory guards and register/stack preservation.
- Existing loader/installer host fault suite: 41 scenarios pass.
- Safe-save ASan/UBSan host model: 27 fault points, 27 interruption points,
  short writes, disk-full, paired failures/rollback, new-file failures, cancellation,
  read-only/locked originals and 20 repeated saves; no retained contexts.
- Snow actual edits and extraction above. No new power-loss/atomicity claim.

## Build and reproducibility

`build/Clarice.bin` and `build/nano.clx` are the updated normal 68000 application
and separate experimental editor, diagnostics disabled. The final build was also
executed in Snow: edited/saved the 8 KiB fixture, verified every byte, reopened it,
and exited normally twice. Both status 0, remaining 0, heap 398,290 and largest
block 393,726 bytes. Normal image 197,896 bytes; owned peaks 83,904 / 83,748.
Loader 545 first / 431 repeat ticks. End-to-end markers are intentionally absent
from this normal build; the detailed startup times above belong to the diagnostic build. Installation instructions,
licence and SHA256 hashes accompany them. Instrumented artifacts are kept separately
in `diagnostic-build/`. Use existing pinned Retro68 toolchain and Snow fixture.

Changed areas: `src/checksum.S`, checksum chunk in `src/module.inc`; optional
`src/nano-perf.inc`, app/editor hook sites, runtime/nano markers, `src/sys6.h`,
`src/CMakeLists.txt`, `scripts/patch-nano-perf.py` and link script. The existing
safe-replacement implementation is unchanged. No unrelated suite reruns.

Diagnostics: configure app with CLARICE_NANO_PERF=ON and compile nano files.c,
nano.c, platform.c and runtime.c with PORT_NANO_PERF; patch script adds no-op hooks
in ordinary builds. `status runtime` then reports the 13 absolute marks. Indices:
0 dispatch, 1 runtime entry, 2 upstream main, 3/4 initscr start/end, 5/6 file open
start/end, 7 main loop, 8 first painted input wait, 9/10 unused, 11/12 save start/end.
Only the first occurrence is recorded per invocation. Scripts in automation/
record how the actual guest input was sent; workspace/disk live under work/nano-perf.
Do not mount that image with hfsutils while Snow is running.

Further startup work needs a focused follow-up; the three-second target cannot be
claimed from this change. Physical SE measurements remain pending.
