# Faster listings and stable scrollbar presentation

Verified in Snow on the Macintosh Plus, 4 MB, System 6.0.8 under MultiFinder. Emulator measurements are not physical-hardware results.

The thousand-entry `ls -la` benchmark improves from 1027/1008/1029 guest ticks to **739/723/722 ticks**: median **12.05 seconds**, roughly **30% faster** than the prior 17.12-second median. First file presentation takes 15/32/31 ticks. Catalog-only enumeration takes 323/325/326 ticks. Three runs share cache state, use equal-length fixture names and repeated metadata, disable paging, and exclude parsing/history and final prompt repaint. The listing is still not instant; more work remains, especially on real, diverse directories.

Printable runs now update terminal rows in batches rather than recalculating terminal state for each byte. Long listing rows are assembled and emitted once. The bulk paint threshold remains 32 ticks; event polling remains two ticks, with immediate first-row and pager presentation. No OS cache changes or larger history allocation were used.

Full redraws previously erased the scrollbar rectangle before repainting it. That rectangle is now excluded from background erasure. Control updates are composed in a fixed 1,368-byte offscreen bitmap, with one clipped transfer to the screen. No live-control hiding is used. This adds about 1.34 KB of fixed scratch storage within the existing 384 KB application partition.

Validation:

- 1,500 randomized terminal-writer comparisons pass against the previous executable reference, under address/undefined-behaviour sanitizers. Cases cover five widths, three retention limits, chunk boundaries, control/high-byte input, wrapping and anchored history.
- Native pixel comparison passes 20 output bursts and 20 reverse/jump history views.
- **141 scrollbar staging checks detect zero premature on-screen changes** during text/control composition. This checks the intermediate erase path, beyond the previous final-image-only tests.
- Quick rendering/retention/cache-release regression passes: native 167 ticks, cached C 223, cached assembly 181 for 30 frames; four cache-release cycles remain stable.
- Paged `ls -la` pauses and advances with Space. Ctrl+C cancels both at the pager and during unpaged enumeration, returning to the prompt before completion.
- File integration assertions pass after the changes, including all 1,000 distinct redirected filenames and both fork sizes.

The native renderer remains the default. The scrollbar test validates its covered code paths and final pixels; it does not establish a universal absence of visual artifacts in every window arrangement or on physical hardware.
