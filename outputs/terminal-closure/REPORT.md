# Batch 2 final closure — build 2026111965

Product commit **8d136a1**. Two narrow corrections on local main; normal 68000 Snow Plus/4 MiB/System 6.0.8.
No physical testing or GitHub push.

- Clipboard logical endings now travel with characters through the existing
  composed/presented row model. Its spare byte at COLS-2 stores the boundary;
  COLS-1 remains the caret. No new allocation or screen buffer. Metadata-only
  commits do not paint. Pending output, ring reuse and editable input cannot
  reinterpret selected pixels. Soft wraps, real endings and final-cell selection
  retain their existing behaviour.
- Preferences menu dismissal no longer invents dirty state. Legitimate native
  exposure updates still paint with clipping, and pre-existing dirty state is
  preserved. CLI and dialog compare screen-clamped grid dimensions before
  layout/save; real font, fill, theme and geometry changes retain existing paths.
  Validation and failed-save retry remain intact.

## Acceptance and measured cost

The focused clipboard, menu and clamped-size host reproductions each fail on
0689d24 and pass on the correction. Host sanitizer tests also cover prior clipped
updates, selection boundaries, tracking cancellation, 8,505 geometry combinations,
allocation-failure rollback/retry, input wrapping and 460 renderer comparisons.

Guest diagnostic: 271 checks pass, including 143 exact pixel comparisons. Both
light/dark menu OK/Cancel produce zero full pending paints, one legitimate clipped
exposure update, zero layouts and zero disk writes. Native dialog creation and
exposure still cost 72–75 elapsed guest ticks; this is not a zero-cost dialog.
Min/max clamped CLI pairs take 1/0 ticks with zero rows, layouts, saves or writes.
40 ordinary typing samples total 67 ticks, worst 2 ticks. These are TickCount
elapsed measurements at fixed Snow acceleration, not calibrated CPU or physical
SE times; the earlier 1964 typing window differed, so no timing speedup is claimed.
The avoided work is confirmed by counters and baseline failing host assertions.

Normal shipping build: real menu 999×999 OK retains the effective 80×27 grid and
config checksum; editing a field then Cancel preserves configuration. A partial
`echo keep` command survives dialog Cancel and subsequent input produces exactly
`keep123`. Three 30-iteration batches of clamped preferences plus file reads leave
free heap at 324636 bytes, remaining command allocations zero. Largest block varies
321596→306620→321596 bytes, recovering without progressive loss. Ctrl+C and
Command-period return 130 and subsequent commands work. Diagnostic cases separately
cover both themes, clipboard, cursor, editor restoration and clipping.

Text +272 bytes, data +16, BSS unchanged at 41448; total code/data +288 bytes.
Core MacBinary +256 bytes (438528). No new persistent buffer or allocation;
all 49 standard utility payloads are byte-identical to 1964. Offline HDA Core
forks/Finder metadata, Release and 49 utilities, and all 51 tar members match the
sealed distribution. Publication verification is recorded alongside; prior1964
artifacts/recovery remain available. Physical acceptance remains pending.
