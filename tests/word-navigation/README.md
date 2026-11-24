# Q1-008 word navigation

Command–Left skips whitespace then moves to the beginning of the previous word;
Command–Right skips the current word and following whitespace, reaching the next
word's beginning or end of input. Words are byte runs separated by ASCII whitespace, not
shell tokens: quotes and escaped spaces do not change navigation boundaries.
Shift does not extend selection. Physical Control/Option combinations retain
existing substitution rules. Only idle shell command editing claims this shortcut;
external editors, serial/terminal input, canonical read, search and paging retain
their existing handling. Plain arrows and Macintosh menu shortcuts are unchanged.

`python3 tests/word-navigation/host.py` tests production helpers, dispatch guard,
arrow switch cases and wrapped composition with ASan/UBSan. Existing viewport and
control-input tests protect the shared input paths.

`python3 tests/word-navigation/probe.py` creates a disposable source copy under
work/word-navigation/src. Build with the existing Retro68 CMake toolchain into
work/word-navigation/build; install only in a disposable Snow Sys6Unix volume.
The probe runs once after activation and writes :word-navigation. It tests native
handler dispatch, history/editing, external-screen isolation, exact caret-only
pixel changes in both themes, and repeated event-to-draw ticks/heap stability.
The diagnostic pixel buffer and probe code are absent from normal builds.
Timings are synchronous guest-handler/draw elapsed ticks, not host input latency
or physical-SE performance. Normal-build acceptance must also use real key events.

The project Snow Plus patch suppresses horizontal host arrows (see
outputs/emulator-input/README.md); use its SE/ADB model for real-key acceptance.
Do not interpret that emulator limitation as a Clarice key-handling failure.
