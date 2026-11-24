# Completion checks

Host: `cc -O1 -fsanitize=address,undefined tests/completion/host.c -o /tmp/completion-test && /tmp/completion-test`.
Run `tests/terminal/viewport-host.py`, `input-layout.py`, `control-input.py` and `tests/geometry/host.py` with Python.

Guest diagnostics: build Core in a separate build directory with
`-DCMAKE_C_FLAGS=-DCLARICE_COMPLETION_DIAGNOSTICS`. This replaces `selftest` only in
that diagnostic build. Prepare `/tmp/completion/n1`, `n10`, `n100`, `n1000`, each
containing the indicated number of empty files named `e0000` onward. Run
`selftest > /etc/clarice/completion-results` in a disposable Snow image.
Never modify a mounted/running guest disk with host HFS tools.

The probe measures guest ticks, free heap and largest block, then injects typing
or Ctrl+C immediately after an event poll (through the native Event Manager).
The third input run temporarily reserves heap down to about 32 KiB contiguous
space and releases the reservation. Confirm every `correct=1` and matching
before/after heap. Timing builds are not distribution builds.

Normal build: type `cat /tmp/completion/n1000/e0`, Tab once (no listing), Tab
again (bounded matches/question). Try y, n, Return, other typing and Ctrl+C;
verify original input/cursor, status130 after Ctrl+C and subsequent execution.
Also exercise `ec<Tab>`, quoted/spaced filenames, case-insensitive prefixes and
editing between Tabs. Check both themes and prompt restoration.
