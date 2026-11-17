# Automatic rendering and matched real listings

15 November 2026. Snow Macintosh Plus, 4 MB, System 6.0.8. No physical SE tests.

## Comparison

`scrollbench listing` in the diagnostic build calls the real `execute()` path for
`ls -la /mnt/Sys6Unix/Bench/n100` and `n1000`, through final prompt presentation.
Same disk, Monaco 9, 80 columns, 26 rows, seeded scrollback, paging disabled.
Three runs per combination, alternating native/cached order. A small listing
primes command loading first. Cache/font construction and screen seeding happen
outside the timed interval: these are warm command comparisons, not app-startup
measurements. All 24 executions returned status 0.

Median guest seconds (60 ticks/s):

| Files | Repaint interval | Native total / paint | Cached ASM total / paint |
| --- | --- | --- | --- |
| 100 | 32 ticks | 2.83 / 0.78 | 2.98 / 0.95 |
| 1000 | 32 ticks | 13.20 / 3.78 | 12.60 / 3.20 |
| 100 | 16 ticks | 3.22 / 1.17 | 3.13 / 1.10 |
| 1000 | 16 ticks | 16.33 / 6.87 | 15.03 / 5.67 |

Raw evidence: `matched.txt`, `summary.json`, `result-screen.png`.
These totals include submission/dispatch/output/final redraw, unlike earlier
catalog-only or internal benchls measurements. Faster synthetic text redraws do
not establish a faster complete directory listing.

## Policy and changes

Normal rendering is automatically native QuickDraw. It wins the smaller real
listing, avoids cache setup and requires no additional resident framebuffer.
This is a measured default, **not** a claim that native wins every workload or
that a runtime workload classifier has been added. Cached ASM saves 0.60 seconds
(4.5%) for the large listing, at a measured 29,704-byte heap cost; that modest gain
does not justify keeping this cache permanently resident. No per-command engine
switching or repeated cache construction was introduced.

Keep the 32-tick bulk-output cadence: halving it adds 24% to the native large
listing. Output already streams before enumeration ends; there is no delayed
replay queue. This work does not yet deliver modern-terminal smoothness. Of the
13.20 seconds, about 9.42 remain outside drawing. Previously isolated HFS catalog
costs remain relevant; this comparison does not further attribute those costs.

Removed the renderer/cache row from Preferences and shortened the dialog.
Font, size, colours, scrollback, paging, columns and fill-screen remain.
Legacy text/binary renderer values are ignored; subsequent saves omit the key.
Normal `prefs cache`/`prefs renderer` return status 2 with an automatic-rendering
explanation. Diagnostic builds retain nonpersistent overrides for comparisons.
No idle memory addition; existing native mode and drawing safeguards retained.
A demonstrated dialog colour-change defect repainted only the former dialog
rectangle. Apply now invalidates the entire terminal window, without adding
redraw work to ordinary input/output. Retested dark-to-light in the final build
(`theme-fixed.png`, `theme-history-fixed.png`, both directions).

## Validation

- Preferences ASan/UBSan host parser/formatter tests pass, including obsolete
  renderer keys, round trips and invalid input (`preferences-host.txt`).
- Normal 68000 build 2026111512 succeeds (`build-normal.log`).
- Normal guest starts from a legacy `renderer=asm` configuration with existing
  font, paging, scrollback and geometry retained (`normal-start.png`).
- Ctrl+C during continuous output and at the pager returns status 130; inspected
  `verified-cancel-automatic-*-status.png`. Running screenshots show output before
  completion. Host timing of automation is not an interrupt-latency measurement.

- Normal native pixel regression: 24 bursts plus 20 reverse/page/jump views,
  zero mismatches; 153 scrollbar staging/reference checks, zero mismatches
  (`native-scroll-report.txt`, `normal-pixels.png`). These checks protect drawing
  correctness and the no-blanking path; they are not a high-speed video analysis.
- Guest dialog layout inspected; theme changed through the dialog, saved and
  retained after quitting/reopening (`preferences-window.png`,
  `preferences-restart.png`). Exported `saved.conf` preserves other settings and
  has no renderer key. Obsolete ASM setting was deliberately supplied at startup.
- After cancellation tests: runtime tracked remainder 0; sampled free heap
  426,282 bytes (`normal-final-runtime.png`). No new renderer allocation.

- Final-build actual scrollbar thumb drag after a real 100-file listing: five
  detected text updates in 130.9–184.7 ms (median 137.4 ms), one initial step
  undetected (`normal-drag.json`, `normal-drag.png`). Host screen sampling measures
  first visible change, not completed-frame latency or continuous drag throughput;
  no matched before/after speedup claim. Final normal build does not instrument
  the guest drag loop.

## Build

`experimental-build/Clarice.bin` is the normal MacBinary application;
`experimental-build/SHA256.txt` identifies it. No push, publication or hardware
installation. Existing application/runtime/editor/update work preserved.
