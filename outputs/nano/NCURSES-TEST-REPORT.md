# ncurses integration tests — 9 November 2026

Baseline: Snow Macintosh Plus emulator, 68000, 4 MB, System 6.0.8, MultiFinder. These are emulator observations, not real-hardware measurements. Experimental branch `codex/ncurses-port`; published v0.3.0 unchanged.

## Initial guest probe

A separately loaded `/bin/cursesdemo` uses real upstream ncurses 6.5. Observed:

- Reverse-video title, positioned text and solid block cursor render in the movable Clarice window; Finder remains accessible.
- Up reports ncurses `KEY_UP` (259). Down followed by Ctrl+C advances the event count; Ctrl+C reaches the application as byte 3 in this initial probe.
- Q returns normally and restores shell output/history and prompt.
- A second run interrupted with Command–period returns status 130 and restores the prompt.
- After both normal and interrupted runs: free heap 441646 bytes, largest block 437224, command image 100424, tracked allocation peak 28806, tracked remaining 0. Equal post-run free heap is evidence for these two invocations, not proof against every possible leak.
- Load times were 338 and 277 guest ticks. Total invocation times include manual interaction and are not a throughput benchmark.

## Later session observations (recorded here during Task 1)

The following were observed in the earlier chat's guest screenshots, not newly
executed during this review: `cursesdemo --check-runtime` printed PASS and returned
0; `--signals` handled Ctrl+C (signal count 1), then Q exited; raw mode processed
Ctrl+C/Up as input; Command–period returned 130. After handled and cancelled runs:
heap 441646, largest 441394, image 103300, owned peak 28931, remaining 0. Loader
measurements were 329/281 ticks. Later sigaction/mask and file-runtime edits were
not retested by those observations. No nano editing or performance result follows
from this evidence.

## Host checks

`screen-test.txt`: address/undefined-behaviour sanitizer checks of cursor positioning, erasure, reverse attributes, scrolling and visibility plus 100000 pseudo-random bytes.

`platform-test.txt`: sanitizer checks of inherited/private environment, overwrite/unset semantics, invalid names, synchronous signal registration/delivery/ignore, canonical editing and partial reads, raw Ctrl+C, input timeout, dimensions, readiness and flush.

## Remaining validation

When the relevant implementation task is authorised, verify clean rebuild and retest the latest sigaction/mask/file-runtime changes; earlier raw/ISIG/environment probes do not cover those later edits. Nano editing/saving, low-memory failure and long-session scrolling are not yet tested. Screenshots establish layout and state transitions; they do not measure flicker or keyboard latency.
