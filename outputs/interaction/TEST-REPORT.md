# Interactive editing validation — 9 November 2026

Target: disposable Snow Macintosh Plus, 4 MiB, System 6.0.8, MultiFinder.
These are emulator observations, not real-hardware measurements.

Implemented: wrapped command editing; cursor composed into buffered text;
1024-slot bounded typeahead; sequential activity catch-up; explicit
backslash-Return continuation and Ctrl+C cancellation.

Host validation:
- Actual compositor exercised under AddressSanitizer/UndefinedBehaviorSanitizer
  across widths 3–130, command lengths up to 1023 and cursor positions.
- Escaped, quoted and chained continuation recognizer cases pass.
- mrsh execution removes escaped newlines between words and inside double quotes.
- Activity tests cover delayed start, four overdue frames in sequence,
  interactive suppression, completion and 32-bit tick wraparound.
- Existing shell suite: 31/31 comparisons and 3/3 cancellation cases pass.

Guest observations on the first interaction build:
- An 80-character command wraps to a second row, with a solid block at the end;
  Return executes its full contents.
- `echo before` followed by backslash-Return and `after` displays a continuation
  prompt and prints `before after`. Ctrl+C cancels an unfinished continuation.
- `echo queued` typed during `tail -f` appears intact after Ctrl+C.
- A complete command typed while sort runs executes after normal completion.
- Activity appears in the top-right after quiet time and disappears on return.

Final-build checks also passed: cached rendering at the bottom two rows, light
and dark inverse-block cursors, joined-word continuation (`abc` + `def` becomes
`abcdef`), and Up recalling the joined command cleanly. Defaults were restored
to native rendering and light colours afterward. Screenshots
verify stable results, not every intermediate display frame; no quantitative
zero-flicker claim is made from screenshots alone. The buffered paint path
removes the previous explicit cursor erase/repaint interval.

Final guest scrolling regression: **PASS, zero failures**. At 54 columns and
24 output rows, each 90-frame run took 585/586/586 ticks natively,
1059/1059/1059 ticks with cached C, and 968/969/968 ticks with cached assembly.
TickCount is 60 Hz. This exercises randomized-looking full-width rows and is
not an `ls` timing or a real-hardware measurement. Four cache allocation/release
cycles were stable (29,704-byte heap delta). Native remains the default.
The guest's 0607 API version value is the known System 6.0.8 reporting quirk;
the test system image is the verified 6.0.8 installation.

Final-build typeahead also replayed a complete command entered during the file
and inspection suite, displaying `final-typeahead-ok` after that suite returned.
The binary hashes are in checksums.txt. The updated build is installed on the
project's disposable Plus disk. GitHub publication remains gated on completion
of the broader utility validation; this report does not claim that release.
