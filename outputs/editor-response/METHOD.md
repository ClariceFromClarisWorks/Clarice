# Editor response experiment — measurement method

Isolated Plus, 4 MiB, reported System 6.0.8 Snow; physical machines untouched.
Current Core base ceb3094 and installed BusyBox vi 1.37.0 (1905). vi does not
use ncurses. No editor downgrade or upstream code modification in this experiment.

Three different measures:

- Event timestamp to completed QuickDraw presentation: includes guest event
  queueing, vi work, parser and scheduling. Diagnostic counters exclude initial
  editor drawing. Consecutive queued keys can share one visible response.
- Active path guest ticks: isolated feed/draw loop, plus parser/painting and
  vi read-to-first-output counters in interactive runs. TickCount has ~60 Hz
  resolution; these are elapsed guest ticks, not exclusive CPU-cycle counters.
  Multi-key prefixes and waits must not be mistaken for pure processing time.
- Repainted row/cell counts: whole rows vs old/new cursor cells, including
  unchanged cells unnecessarily repainted. They count requested cells before
  native window clipping, not literal bus writes.

Snow's cycle-driven VBL/TickCount behaviour was already checked in the accepted
0.5 performance audit (100k/200k/400k iterations: 66/132/264 ticks at both normal
and FFWD pacing). Use fixed uncapped pacing here; host sleep and screenshot time
are not performance measurements. User-requested SE planning estimates multiply
Snow guest-time results by 1.2, and are not physical acceptance.

Both old and new draw paths run in the same diagnostic binary. Native QuickDraw
pixels are compared to forced full redraws, including dirty buffers, cursor
visibility, reverse attributes, clears, scrolling and partial clipping in both
themes. Diagnostic allocations/counters do not ship.
