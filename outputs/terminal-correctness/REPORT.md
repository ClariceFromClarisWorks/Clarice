# Batch 2 terminal correctness — build 2026111963

Four fixes accepted in host tests and Snow Plus/68000, 4 MiB, System 6.0.8.
No physical hardware access. Normal utilities are byte-identical to build 1962.

- **Clipped updates:** when model changes are pending, finish BeginUpdate/EndUpdate
  and paint once using the normal visible region. Otherwise retain the clipped
  update. No compositor change, duplicated paint or new dirty-state machinery.
- **Mouse tracking:** thumb, held-arrow and selection loops stop on a queued key;
  ordinary dispatch handles cancellation/modifiers afterward. No key consumption,
  geometry change or menu dispatch inside tracking.
- **Clipboard:** derive real line endings from history wrap flags and the existing
  editable-input wrap rules. Soft wraps and unfinished last rows add no newline.
- **Selection:** permit the exclusive endpoint immediately after the last cell.

Acceptance: production-code host reproductions fail before and pass after;
460 existing renderer comparisons, viewport checks, 8,505 geometry combinations,
layout allocation-failure rollback and sanitizers pass. The native Toolbox probe
has **89 passing checks**, including **41 exact pixel comparisons** in both themes,
pending prompt/editor changes outside an update, further typing before Return,
generic glyph fallback, history jumps/scrolling, selection and editor restoration.
The same probe found ten failures in the original code.

Normal build: cancellation during selection (Ctrl+C), held scrollbar arrow
(Command-period), and held thumb (Ctrl+C) restores the prompt before mouse release;
all return 130 and subsequent commands succeed. Original selection tracking did
not return the prompt until release. Host captures are a bounded observation,
not a calibrated guest input-latency measurement. Vi opens, accepts two edited
lines, quits without saving and restores the terminal; remaining allocations=0.

Matched 40 key-plus-draw samples: **72 → 72 guest ticks**, worst **2 → 2 ticks**.
Probe total 2069 → 2100 ticks includes intentionally repaired full presentation
and test oracles; not a general scrolling benchmark. Snow fixed fast-forward;
no physical-SE performance claim. No persistent bitmap/heap allocation added.
Normal warm repeated 30-command batches: heap 325908 → 325914 bytes, largest
block 323220 → 323250; remaining command allocations always zero.
Text +348 bytes, data +4, BSS unchanged (41448); MacBinary +384 (437760 bytes).

Reproduce with tests/terminal/{correctness-host,tracking-host,viewport-host,
editor-render}.py, tests/geometry/host.py and tests/review-b/geometry.py.
The existing CLARICE_USABILITY_DIAGNOSTICS probe includes correctness-guest.inc;
tracking-snow.swift captures held-button cancellation on the normal build.
Raw focused results are alongside this report. Previous distributions remain
retained; publication verification records the sealed 51-entry HTTP/AFP collection
(Core, 49 standard commands and independently versioned vi), offline packages and
source provenance. Physical acceptance remains pending. Stop after these fixes.
