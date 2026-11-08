# Listing and native rendering optimization — 9 November 2026

Test platform: Snow Macintosh Plus, 4 MB, System 6.0.8, MultiFinder; Monaco 9, 80 columns and 26 text rows plus prompt. Emulator measurements are not real-hardware measurements.

## Performance

The creation-time thousand-file full-output listing improved from **14.45 seconds to 11.70 seconds median**, a 19.0% reduction. Final runs: 713, 700, 702 ticks at 60 ticks/second; first entry 16, 27, 27 ticks. Three-run drawing time fell from 895 to 587 ticks. This remains a substantial wait, not an instant listing.

The intermediate candidate measured 743, 733, 734 ticks (12.23-second median). The retained candidate additionally prepares only the changed characters and configures the reusable row font once per paint. Variability and the coarse timer prevent attributing every saved tick to one isolated change.

The benchmark disables paging to measure all 1,000 entries. It excludes PATH lookup/module loading and the final prompt redraw; three repetitions share disk-cache state. Names are equal length and metadata largely repeated, so this is not representative of every directory. The date formatter does not cache repeated dates. Interactive listings still page and pause enumeration.

## Changes

- Plain HFS fork sizes use native 68000 DIVU long division rather than general division/remainder library calls. Human-readable sizes retain their formatter.
- Type/creator formatting is skipped unless `--full` requests it.
- Misaligned native windows copy exactly the changed span rather than rounding to an alignment that cannot enable the direct assembly copy path.
- The reusable offscreen row port's font is set once per paint, and only changed characters are prepared.

Creation timestamps, file flags, directory blanks, stream output, paging, two-tick event polling and 32-tick bulk repaint threshold are retained. No extra heap/cache/scrollback allocation. The native command image is 11,200 bytes, 244 bytes larger than the previous build, and is freed after each invocation.

## Validation

The actual compiled M68000 decimal conversion passed 75,540 independent integer comparisons: every value through 65,536 plus seeded random full-width values and endpoints. Tests check exact output, an odd destination address, guard bytes, preserved registers under this compiler's ABI and balanced stack. The compiler uses D2 as scratch; the test covers D3–D7/A2–A6.

The terminal writer passed 1,500 randomized equivalence cases under address/undefined-behaviour sanitizers. CLX packaging checks passed.

Native light-theme scrolling passed 24 output bursts and 20 reverse/jump history views with zero pixel mismatches against full redraws. All 153 scrollbar staging checks showed zero premature changes to the live control. The separate varied-text/assembly/cache-release quick regression passed; native 194 ticks for 30 frames, cached C 423 and cached assembly 405. This single quick run is coverage, not a repeated before/after performance comparison. Native remains the default.

Dark-theme native verification also passed 24 bursts and 20 views, with zero pixel mismatches and 153 clean scrollbar staging checks (306 combined across both themes). Guest metadata tests and the complete redirected 1,000-file listing passed: every name appears once, with correct fork sizes and retained creation date/time. Space advanced the pager; Ctrl+C at the pager and during unpaged output returned status 130. The continuous-output cancellation stopped at file0046, well before completion.

These tests cover the exercised drawing paths; they do not establish universal flicker-free behaviour on every font/window arrangement or physical hardware.
