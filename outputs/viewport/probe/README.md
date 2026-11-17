# Diagnostic snapshot

`diagnostic-snapshot.patch` records the current isolated diagnostic overlay from
`work/viewport/build-root` relative to development sources. `source-hashes.json`
records the base and resulting source hashes. This is the **subsequent unrolled
planes experiment**, not an assertion that its result has passed. The raw planes
and row-cache reports predate this snapshot.

To reproduce without changing production, copy matching `src/` and `tests/` into
a disposable build root, verify the recorded base hashes, then apply the patch
there (`patch -p1 < /absolute/path/to/diagnostic-snapshot.patch`). If hashes differ,
review the differences rather than silently applying to another source state.
The overlay includes automatic diagnostic execution and is not a normal build.

Use the existing pinned Retro68 image/toolchain and enable:
`CLARICE_DRAW_DIAGNOSTICS`, `CLARICE_SELECTION_DIAGNOSTICS`,
`CLARICE_SELECTION_FIXED`, and `CLARICE_USABILITY_DIAGNOSTICS`.
The current paired selector in `tests/terminal/jump-guest.inc` is
`planes_enabled=variant+1; history_raster_enabled=0` (looped versus unrolled).
For the earlier comparisons, the controlled selector was respectively:

- Glyph planes: `planes_enabled=variant; history_raster_enabled=0`.
- History cache: `planes_enabled=1; history_raster_enabled=variant`.

Keep the same 24-jump workload, three repetitions, geometry and themes. Give each
experiment distinct output filenames. Do not equate the snapshot's current
unroll fixture with the exact earlier executable. Full reproduction and hardware
acceptance remain the responsibility of the continuing experiment.
