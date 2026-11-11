# QOL-3A — selection performance

11 November 2026. Completed; included in the subsequently authorised 0.4.0 release.

## Finding and targeted fix

QOL-2 did not resolve selection latency. The mouse-drag handler has its own synchronous polling loop, separate from keyboard event batching. Every poll set `force_draw` and repainted the whole terminal, including when the pointer remained in the same cell. Selection painting also inverted individual cells during that full redraw.

The drag handler now changes only the pixels between the previous and new selection endpoints, one QuickDraw inversion rectangle per affected row. Reversing across the anchor uses the same interval operation. An enabled existing framebuffer is updated in place; native mode needs no framebuffer. Starting a new selection or changing scrollback removes the old highlight, pending content is painted first, and releasing the mouse samples the final endpoint without forcing a repaint. SystemTask is serviced during tracking. Editor-owned screens are excluded from terminal selection.

No new persistent allocation, cache, queue or global production state. No clipboard commands, cursor/typeahead, listing or startup changes. Existing viewport-relative selection semantics remain: select within the displayed scrollback page; dragging outside the viewport does not introduce automatic scrolling or selection spanning multiple pages.

## Plus guest measurements

Existing Snow emulator and disposable clone of QOL-2's image, Macintosh Plus / 4 MB / System 6.0.8 / MultiFinder, normal video-paced emulation, Monaco 9. The original user's Snow process/image was not modified.

The diagnostic build runs the exact old or new drag-paint operation on 12 endpoints extending and reversing across multiple lines. It measures guest TickCount around each operation, then compares live pixels to a forced full redraw. This isolates the identified rendering bottleneck; these are **highlight-update costs, not host mouse-to-display latency**. Each tick is approximately 16.7 ms. Allocation/readback/reporting are outside the timed operation.

| View | Before total / 12 updates | Before worst | After total / 12 updates | After worst |
| --- | --- | --- | --- | --- |
| Native, live, light | 770 ticks (12.83 s) | 78 ticks (1.30 s) | 1 tick (17 ms) | 1 tick |
| Native, 35 lines back, dark | 814 ticks (13.57 s) | 82 ticks (1.37 s) | 2 ticks (33 ms) | 1 tick |
| Cached assembly, live, light | 674 ticks (11.23 s) | 71 ticks (1.18 s) | 6 ticks (100 ms) | 1 tick |
| Cached assembly, 35 lines back, dark | 710 ticks (11.83 s) | 74 ticks (1.23 s) | 4 ticks (67 ms) | 1 tick |

Twelve stationary polls previously consumed 537–678 ticks; afterward all twelve together measured zero ticks (below timer resolution, not zero CPU time). All 56 pixel comparisons matched full redraws: 48 endpoint changes plus eight checks of scrollback transitions and highlight removal. The raw final log reports scrollback 1/36 after those additional checks; timed endpoint changes were at 0/35. Free heap delta was zero in every case before and after. The temporary diagnostic readback buffer is released and absent from normal builds.

Raw results: [before](before.txt), [after](after.txt), [guest screen](guest-results.png). The expanded regression found two issues after the first speed improvement: stale cache highlighting when changing scrollback, and an unaligned cached source passed to the even-aligned assembly row copier. The final change clears the selection before changing views and checks source alignment in the fast copy condition, matching the native row path; other alignments use existing QuickDraw. See [alignment fault log](alignment-failure.log).

## Input and focused verification

Actual host HID mouse-down/drag/up events traversed Snow's mouse interface and produced retained multiline selections in the [live view](drag-live.png) and [scrollback](drag-scrollback.png). Process-targeted CGEvent mouse delivery did not produce a selection; switching to the same global HID delivery used by the existing click helper worked. No manual emulator interaction was required. These screenshots establish mouse integration, not a precision end-to-end latency measurement.

The focused host regression extracts the production delta function and checks forward/reverse movement, crossing the anchor, unchanged endpoints and native/cache consistency across sampled widths from 1 to 130 columns and heights from 1 to 37 rows, with ASan/UBSan: [pass log](host-selection.txt). This is a mock pixel grid, separate from the guest QuickDraw comparisons.

Final normal-build installed-path mouse/scrollback smoke checks also passed; see [release evidence](../release-0.4.0/TEST-REPORT.md). Normal application build succeeded with selection and QOL-2 diagnostics OFF: [build log](build-normal.log), [MacBinary](Clarice.bin). No unrelated test suites or utility builds were run.

## Scope and reproduction

Production changes: `src/app.c` (selection helpers, mouse handler and one cached-copy alignment guard), `src/CMakeLists.txt` (optional diagnostic switches). New tests: `tests/terminal/selection-guest.inc`, `selection-host.py`, `snow-selection.swift`. [Task-only app diff](app.patch). Start-of-task hashes confirm all other pre-existing source/test/script files remained unchanged: [scope check](scope-check.txt). QOL-1, QOL-2, Task 2 and unpublished editor/port work are preserved.

Run the focused host test with `python3 tests/terminal/selection-host.py`. For guest instrumentation configure `CLARICE_SELECTION_DIAGNOSTICS=ON` and `CLARICE_SELECTION_FIXED=ON`; the probe executes once after launch, writes `/tmp/qol3a-selection.txt`, and reports SELECTION-PROBE-DONE. For the old path, the preserved baseline `work/qol3a/baseline-app.c` plus the same diagnostic include used the switches ON/OFF respectively. Finish the application with the existing finish-application script, install only into a stopped disposable image, run in `work/qol3a/plus.snoww`, exit to Finder before stopping Snow, and extract the report with existing hfsutils. Normal builds must have both switches OFF. The HID helper assumes the test Snow window is foreground at host position (220,52), size (1000,782); it is a small reuse of existing CGEvent automation, not a new framework.

Limits: emulator evidence only, not physical hardware; measured Monaco/native and cached assembly configurations, not every installed font or window overlap. Existing viewport selection boundaries are unchanged. No architectural change or product decision is needed. No further QOL implementation undertaken; publication subsequently authorised.
