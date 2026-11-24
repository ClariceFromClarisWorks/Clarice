# Shared editor glyph compositor

The full-screen editor now reuses Clarice's existing native Monaco9 glyph
compositor. Its output occupies the first704 bytes of the existing4096-byte
strip, preserving cached glyphs above it. Other fonts/high bytes use DrawText
and invalidate the overlapping glyph storage. No new bitmap or allocation.
QuickDraw retains clipping and colour conversion; cursor-only inversion remains.

Matched accelerated Snow Plus/4MiB/System6.0.8 results (guest ticks):

| Operation | Before | After |
|---|---:|---:|
| Scrolling paint, aggregate ticks |3432|2043–2055|
| Scrolling event-to-presentation |554ms|450–452ms|
| Page paint, aggregate ticks |789|453|
| Page event-to-presentation |752ms|594ms|

Identical requested cells/output bytes. Vertical cursor84ms and horizontal52–55ms
remain essentially unchanged. Counters measure elapsed active paths, not exclusive
CPU. No physical acceptance; SE×1.2 remains only the user's planning estimate.

568 guest comparisons against native DrawText pass, including dense ASCII,
partial groups, high-byte fallback, clipping, themes and cursor transitions.
Host460 presentation tests,100k cursor cases,20k inverse cases, control mapping
and selection-delta tests pass. The old standalone native-glyphs harness lacks
current cache-header stubs; it is not claimed passing.

The rejected4KiB character-shadow experiment reduced drawn cells but gave little
response improvement. Its source/evidence remains locally in outputs/editor-damage.
No private THINK C/CMaster renderer source was found. Historical lessons reused:
[Allen Stenger's THINK Class Library browser](https://preserve.mactech.com/articles/mactech/Vol.07/07.08/Browser/index.html)
and [Don Koscheka's scrolling explanation](https://preserve.mactech.com/articles/mactech/Vol.05/05.06/Jun89Letters/index.html).
They support visible-region work and pixel reuse, already used by Clarice's shell.
No historical source was copied. The measured editor improvement reuses Clarice's
own accepted compositor; it is not a claim to reproduce THINK C internals.

Normal Core1945 passes exact20j/20-arrow positioning,120-key burst, saves and
readback, Ctrl-C ownership, Command-period130 and subsequent shell execution.
Normal vi is byte-identical to accepted1943. See normal-acceptance.json.
