# Quiet-command activity indicator

Validated in Snow's Macintosh Plus / 4 MB / System 6.0.8 guest, not physical hardware.

The one-character overlay is in the last text cell of the first row, clear of the scrollbar. It is not written into output, copies or scrollback. The sequence is `| / - \\`, clockwise, at 30-tick intervals after 300 ticks without terminal output. It restores the saved cell when hidden. Storage for that cell is 256 bytes; rendering uses the existing offscreen prompt port.

`tests/terminal/activity.c` checks threshold, half-second phases, wraparound, completion, new output and screen ownership. These assertions passed. The terminal writer still passed 1,500 randomized equivalence cases.

Guest diagnostic `tests/ports/activity.c` ran silently for 900 ticks. The indicator was visibly at top-right while running and absent after normal completion. The subsequent native scrolling regression also passed 24 bursts and 20 history views with zero pixel mismatches, and 153 scrollbar staging checks with zero premature screen changes. Its interactive-ownership variant showed no indicator while running. Interrupting a second silent run with the emulator's Option-C mapping displayed ^C, restored the prompt and removed the overlay.

Future editors must explicitly claim screen ownership through `app_activity_interactive(1)`. Canonical input and paging also suppress the overlay. It is cooperative: programs must yield to update or cancel. The overlay is suppressed for font cells wider than 32 pixels or taller than 64 pixels. This test does not establish a faithful nano/top port or real-hardware animation timing.
