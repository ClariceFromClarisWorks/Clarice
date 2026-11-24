Focused checks: `python3 tests/geometry/host.py`,
`python3 tests/terminal/viewport-host.py`, preferences format and reflow C tests,
`python3 tests/terminal/editor-render.py`, `python3 tests/terminal/editor-inverse.py`.

Snow: copy bench.sh into /tmp/geometry-bench; use Monaco9, 440×264 content,
fixed maximum acceleration, no concurrent build. Run
`sh /tmp/geometry-bench > /tmp/result 2>&1` once after launch on each build.
The three samples clear the terminal before ten50↔70-column changes.
Fixture echo output goes to the redirected result file, not the viewport; these
are empty-viewport resize/persistence costs, not a dense-history drawing benchmark.
Times include native resize, reflow, drawing, event service and preference-file saves (1949 also rebuilt font glyphs);
60Hz guest elapsed time is not isolated CPU time or a physical-SE forecast.

Native UI acceptance: grow the window, open Preferences, verify actual columns/rows;
edit both, Cancel/reopen (unchanged), then OK/reopen (new dimensions); quit/relaunch.
Exercise positive under/oversize requests, both themes, a long unfinished input,
scrollback and selection. Run vi, attempt growing (existing editor lock retained),
save/quit and verify geometry and prompt restored. Preserve the prior preferences
when testing the first-launch default. Physical Macs are not part of this fixture.
