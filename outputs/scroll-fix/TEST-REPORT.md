# Shift-arrow and scrolling correction

8 November 2026. Plus / 4 MB / MultiFinder guest validation. Manual thumb dragging is confirmed by the user; automated dragging remains limited as described below.

Baseline: Plus, 4 MB, MultiFinder, verified System 6.0.8; see ../system608/TEST-REPORT.md for the 0607 API-version discrepancy. Original ROMs and installation media remain untouched.

## Changes

- Plus Shift+Up/Down normalize observed physical key codes 4D/48 before text editing. A temporary guest diagnostic established character codes 2F/3D with Shift held; main-keyboard Shift+slash remained code 2C, character 3F. The diagnostic was removed. Plain arrows and main-keyboard punctuation retain their behaviour. On a physical M0110A, these shifted arrow sequences overlap with keypad divide/equal; this handling intentionally gives the arrow/scrollback interpretation inside Clarice on the Plus.
- Native scrolling composes changed text in the existing 4,096-byte row buffer. Text is transferred without a prior on-screen erase. Overlapping screen copies replace ScrollRect's exposed-strip erasure; covered views use model redraw with QuickDraw clipping. Forced redraws erase margins, not the whole text area. The optional cached renderer likewise avoids the whole-window erase.
- Live thumb tracking clamps to the existing bounded retention range, paints only changed positions, and rate-limits intermediate paints to two guest ticks. This is a scheduling cap, not a promised frame rate.
- The row-copy fast path uses the existing reference-tested 68000 routine only for aligned, visible monochrome destinations; all other cases use QuickDraw. A rejected candidate exposed an odd-address error; source coordinates and alignment guards were corrected before final acceptance.

## Performance comparison

The first all-QuickDraw row-buffer candidate passed a 30-frame diagnostic in 192 native ticks, versus roughly 169 ticks extrapolated from the prior 90-frame baseline. It was not selected as the final implementation. The corrected final candidate passes the full 90-frame, three-run suite: native 526/525/526 ticks (maximum frame 5/6/5), cached C 694/693/693 (7/7/7), cached assembly 567/567/567 (6/6/6). Native remains fastest. Its median is 3.7% above the prior 507-tick median; the small cost buys row composition without an on-screen blanking pass. These are guest ticks at 60 Hz, not host timings or physical hardware measurements. The cached allocation delta remains 29,704 bytes, with four release cycles stable.

## Verification limits

Emulator results are not physical Plus measurements. Pixel equality confirms completed rendering, not every intermediate scanout frame. Removing erase-before-draw addresses the specific blank-flash mechanism; absolute absence of display tearing across hosts is not established by screenshots.

## Observed checks

- Full scrolling/reference/memory suite: PASS, zero failures.
- Native: 20 output bursts plus 20 reverse/page/jump views, zero pixel mismatches.
- Cached: the same 40 comparisons, zero pixel mismatches.
- Native inverse colours: the same 40 comparisons, zero pixel mismatches; screenshot retained. The extracted native report is from this final inverse-colour run. Light colours restored afterward.
- Emulator keyboard regression suite: 5 passed.

## Thumb-drag limitation

Track/page clicks move the view correctly. A temporary guest diagnostic proved a held click enters thumb tracking (320 iterations over 50 ticks, unchanged pointer position). Automated drag attempts did not enter that loop and produced no drag report. The user subsequently confirmed that dragging the scrollbar manually moves the view. A local Snow experiment forwarding event positions moved the guest pointer to the final position but did not resolve automated dragging; it was rejected and the established keyboard-patched emulator restored. The experiment and build log are retained only in work/scroll-fix. Diagnostic code was removed from the delivered application.

User verification: “Dragging works manually.” This closes the functional thumb-drag check in the Plus guest. This is user-observed evidence, separate from the automated pixel and timing tests; it does not establish a measured drag frame rate or guarantee absence of all transient flicker.

## Final build regression checks

On the restored, established keyboard-patched Snow build: Shift+Up during a paused second directory page shows the previous 22 lines; Shift+Down restores files 0021–0041. Plain Up recalls `echo two`; Down returns to the empty pending input. Ctrl+C stops the listing and displays `^C` and a new prompt. Main-keyboard Shift+slash still inserts `?`; Ctrl+C clears that unfinished input. Screenshots retained. The final guest is left running at an idle prompt with output available for a manual scrollbar drag.
