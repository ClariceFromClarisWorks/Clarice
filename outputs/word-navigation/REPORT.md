# Q1-008 — Command–Arrow (build 2026111955)

Shell-only Command–Left skips whitespace then reaches the previous word's start.
Command–Right skips the current word and subsequent whitespace to the next start
or end of input. Six ASCII whitespace bytes separate words; quotes and backslashes
are ordinary characters. Plain arrows retain character movement. Command shortcuts
are unchanged; Control/Option combinations retain existing substitution behaviour.
Shift alone with Command does not extend selection. External editor, canonical
read, serial, search and paging ownership bypass the new shell event routing.

Three small stateless helpers and the existing arrow cases use existing native
modifier/line-editor/differential-rendering paths. No renderer or external-program
input changes, allocator calls, buffers or persistent state added. BSS unchanged;
68000 text236412→236712 (+300), data34852→34872 (+20), MacBinary432000→432384 (+384).

## Acceptance

- ASan/UBSan production helpers, dispatch guard, arrow cases and wrapped composition:
  empty/single/multiple words; six whitespace types; leading/trailing/repeated
  whitespace; quotes/paths; all8192 modifier combinations; ownership exclusion;
  recalled-line editing;1023-byte wrapped commands; repeated boundaries.
- Existing viewport and control-input regression tests pass.
- Snow Plus4MB/System6.0.8 diagnostic: native event-handler dispatch, all three
  Control preference modes, history/editing, external screen arrow preservation,
  exact caret-only pixel differences across wrapped rows in both themes. No other
  pixel changes.120 movements each: plain logic5ticks/draw-inclusive347/worst3;
  word logic9/draw-inclusive411/worst4. Heap and largest block deltas both zero.
  These are60Hz guest synchronous handler+draw elapsed ticks (word average57ms,
  worst67ms for this180-byte workload), not host input latency, CPU-only timing,
  or physical-SE measurements. No full-window repaint/reflow was added.
- Normal build in Snow SE4MB/System6.0.8: actual host Command–Left/Right and plain
  arrows; insertion at the observed caret before Return; Copy, Command-period,
  Paste; exact echo output; Up history then word navigation/edit; prompt recovery;
  runtime remaining allocations0. Normal and offline builds pass, including
  existing production loader/import verification.

The existing Snow Plus keyboard patch deliberately drops horizontal arrows before
Event Manager (outputs/emulator-input/README.md). A diagnostic key trace confirms
no horizontal key events arrive, although ordinary text/Command-Q do. Therefore
real-key acceptance used the SE/ADB emulator; Plus correctness uses actual guest
handler events. No emulator modification or physical hardware access. Physical
Macintosh acceptance remains pending.

Prior1954 distribution/recovery retained; no GitHub push. Publication verification
is recorded alongside this report when complete.
