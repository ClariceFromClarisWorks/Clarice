# 1712 — live bulk output, corrected timing probe (Snow only)

Renderer identical to1711. Diagnostic fixture generation is now outside timed
intervals; counters split terminal ingestion and completed drawing. The extra
6,336B batch storage exists only during this diagnostic, not normal rendering.
293 full/narrow/hole-clipped, theme and interactive-fallback pixel comparisons
pass at maximum Snow acceleration, Plus/4MB/System6.0.8. Report is closed/flushed
and collected over the existing native FTP client; subsequent FTP/prompt recover.

Each pair below covers six48-line dense output batches, with matched bytes and
completed paints. Guest ticks; not host elapsed time or whole cat/ls performance.

| Clip | Light old→new | Dark old→new |
| --- | --- | --- |
| Full |133→115|147→122|
| Narrow |130→112|148→124|
| Hole |175→155|188→164|

Full-clip total reduction13.5% light/17.0% dark. Painting alone108→89ticks
light/122→98dark; ingestion25→26/25→24ticks. Details in
snow-live-output.txt/timings.json. All six
dense comparisons improve. Small four-row batches and sparse listings stay on
the incremental/span renderer, varying−3..+2ticks over six paints. No additional
production cache, bitmap or permanent allocation. Physical acceptance pending;
1710 remains on the SE, and the release candidate is unchanged.

This is a promising isolated candidate, not an accepted production change. The
user-reported1710 boxed-minus pointer has an unresolved trigger; Finder switching
restored the arrow. Do not conflate that anomaly with these pixel comparisons.
Human sub-frame observation, physical live-output timings and repeated-pressure
acceptance are still needed before promotion.

## Reproduction

experiment-from-2df1684.patch contains the complete experimental src/tests overlay
against accepted2df1684, including preceding viewport, glyph and heap-admission
work. It is NOT the release candidate. Apply in a separate checkout, then build
src with the pinned Retro68 toolchain and CLARICE_DRAW_DIAGNOSTICS=ON. SHA-256
source hashes are in source-hashes.json. Existing local build resides at
work/direct-terminal-v8/build/Clarice.bin. No utility rebuild is required.

Use scrollbench live on an uncovered Monaco9/80-column512×342 display. The probe
writes /tmp/live-output.txt; timed work excludes fixture formatting, bytewise
reference comparisons and inter-frame yields. No disk/image modification while
Snow is running. Build ID2026111712; no push, release or physical deployment.

Build MacBinary290560B; SHA25684a65be3fb0a33fc39e1996fdeb0ce32915f4a23ee1427a2001989bf003046d4.
The source overlay applies cleanly to2df1684 (git apply --check). Temporary
FTP collectors and Snow-only PTY fixture stopped after results were collected;
SEControl and the existing development update service remain available.

## Physical SE result, 17 November

1712 updated/relaunched successfully. Stock SE68000/4MB/System6.0.8.
293 completed-image comparisons pass, failures0/cancelled0.
Six dense48-line batches, full clip: light155→135ticks (12.9%),
dark183→150 (18.0%). Painting alone124→104 and152→119ticks;
ingestion31ticks unchanged. Narrow clips154→134/174→144; holes204→178/232→200.
These are matched terminal ingestion+paint operations, not complete ls/cat time.
One pair per case, six paints per pair; physically repeated trials remain useful.
Fallback cases do not engage the new path and vary −11..+18ticks, so no small-case
speed claim. Stage storage and memory policy unchanged from1710.

The patterned, deliberately clipped test images are not normal terminal output.
User noticed them; explained their purpose. Final completed pixels are compared
against the old renderer. Short-flash/visual acceptance remains separate.
System6 actual keyboard/menu/scrollback clipboard workflows pass; see physical/clipboard.
