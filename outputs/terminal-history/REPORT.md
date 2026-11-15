# Historical Macintosh terminal rendering review

15 November 2026. **Leave build 2026111525's renderer unchanged.** All three
codebases were inspected. None establishes a convincing, low-risk improvement
to its 416 ms physical-SE dense-history redraw within the existing memory budget.
This is a source investigation, not a benchmark of these applications. Ordinary
scrolling, selection and typing are outside the current performance problem.

## Sources and attribution

- **NCSA Telnet 2.7b5:** actual `2.7b5src.sit_.hqx` from the
  [specified archive](https://www.macintoshrepository.org/83-ncsa-telnet-2-x).
  Download SHA-1 matches its published `9e365137f85ffd84a0da85e172bd404db9a23453`.
  Inspected `source/Screens/{rsmac,rsinterf,vsinterf,vsintern}.c`, screen data
  definitions, parser and network/event callers. Credit NCSA/University of
  Illinois, Gaige B. Paulsen, Scott Bulmahn and the credited BYU contributors.
  The [NCSA 2.6 manual](https://pzacad.pitzer.edu/~dward/classes/skills/telnet/MacTelnet.pdf)
  places that version's source/documentation in the public domain; the 2.7b5
  archive retains University and third-party notices. This is not a blanket
  licensing conclusion for every later archive file.
- **BetterTelnet 2.0fc1:** actual source download from
  [Rolf Braun's page](https://www.club.cc.cmu.edu/~rbraun/macssh/), including its
  GPL version 2 `COPYING`. The four screen implementation files match the
  [macssh mirror at c57571f](https://github.com/macssh/bettertelnet/tree/c57571f55b347295772a0b0b50535e958cf1aa45)
  after newline/encoding normalization. Credit Rolf Braun and inherited NCSA/BYU
  authors. The published final release targets System 7/8/9; its surviving
  monochrome branches do not prove System 6 build compatibility.
- **Flynn:** [commit 6d12f6e](https://github.com/ecliptik/flynn/tree/6d12f6e6d942f9111fc4701de0b2f46e93a7c1c7),
  particularly `src/terminal_ui.c`, `terminal.c`, `terminal.h` and `main.c`.
  ISC licence, copyright Flynn contributors; its licence separately credits
  joshua stein's Wallops/Subtext and other component authors. This is a modern
  reference; hardware-era authorship is not evidence of either speed or quality.

No source was copied into Clarice. Research copies stay in ignored
`work/terminal-history/`, with original archives and separate readable copies.
The installed Unarchiver library extracted the archives; no dependency was added
to Clarice. [Source hashes and provenance](sources.json). Line numbers below
refer to normalized source lines, not classic-Mac CR files viewed as one line.

## What the renderers actually do

| Project | Character-to-pixel path | Scroll and redraw policy | Relevance to Clarice |
|---|---|---|---|
| NCSA | `RSdraw` sets/caches attributes, erases a run, then uses `DrawText` directly in the window. Installed/special VT fonts, no application glyph compositor found in this path. | `ScrollRect`, exposed/update regions, grouped attribute runs. Linked text history; indexed live lines. Large non-overlapping history moves redraw the view. | Pixel reuse, native fonts, run batching and clipping are already present. Direct visible erase/draw loses our completed-row presentation guarantee. |
| BetterTelnet | Essentially the same `RSdraw`; cached port/attribute state avoids redundant Toolbox setup. | Adds deferred jump scrolling: update the text model while drawing is suppressed, then move pixels and redraw exposed rows. Supports attributes in scrollback or the older compact representation. | The strongest incoming-output idea is already covered by Clarice's bounded model and chunked presentation. It does not avoid rasterizing a new dense viewport. |
| Flynn | Optional monochrome offscreen path caches system-font glyphs, shifts and ORs/AND-NOTs glyph scanlines in C; unsupported text uses QuickDraw runs. | Dirty rows plus shadow comparisons; pixel scroll on the display and buffer; coalesced dirty-strip copies, or whole content copy at 20 dirty rows. | Glyph reuse and dirty comparisons are already present. Its larger retained buffer buys copy batching, with a memory tradeoff excluded here. |

Source anchors: NCSA `rsmac.c:481–533` (`RSdraw`), `:616–687`
(`RSdellines`), `vsinterf.c:451–562` (`VSredraw`), `:917–1022`
(scrollback). BetterTelnet
[rsmac.c](https://github.com/macssh/bettertelnet/blob/c57571f55b347295772a0b0b50535e958cf1aa45/source/Screens/rsmac.c)
(`RSsetattr`, `RSdraw`, `RSdellines`) and
[vsintern.c](https://github.com/macssh/bettertelnet/blob/c57571f55b347295772a0b0b50535e958cf1aa45/source/Screens/vsintern.c)
(`VSIflush`, `VSIindex`, approximately lines 1331–1367).

Flynn's [terminal_ui.c](https://github.com/ecliptik/flynn/blob/6d12f6e6d942f9111fc4701de0b2f46e93a7c1c7/src/terminal_ui.c)
contains `offscreen_alloc` (233), `term_ui_draw` (752), `offscreen_blit_glyph`
(1598), `font_cache_rebuild` (1963), and `font_cache_blit_run` (2055).
Its glyph loop skips zero source scanlines **after clearing the destination**.
Clarice's four-character groups overwrite dirty destination bytes; simply
skipping zero output would leave old pixels behind. No specialized 68000 text
assembly or pre-shifted glyph-phase tables were found in the reviewed paths.
QuickDraw's OS implementation is a different layer and was not reverse-engineered.

Flynn aligns offscreen rowBytes to even bytes, but does not explicitly match
screen/source bit phase as Clarice now does. Its offscreen-enabled shadow is
50×132×2 = **12.89 KB**; ASCII glyph payload **7.42 KB**; special-glyph payload
**3.83 KB**, plus **512 bytes** lookup and descriptors. Its bitmap additionally
uses `2*ceil(window_width/16)*window_height` bytes, **16.50 KB** for 512×264
pixels. These are source-derived component sizes, not measured total application
RAM, and features/build variants differ. Clarice reuses **2.97 KB** of glyph
slots inside its existing **4.00 KB** row bitmap.

## Equivalent operations and responsiveness

- **One-line incoming scroll:** all three avoid drawing the whole viewport when
  old pixels remain useful. Clarice already uses overlapping `CopyBits` without
  `ScrollRect`'s exposed-area erase, then presents completed rows. Covered areas
  retain its tested redraw fallback. No reason to change that working path.
- **Large jump with no overlapping content:** NCSA/BetterTelnet call full-view
  redraw; Flynn's page-scroll branch marks all rows dirty. The shadow comparison
  is disabled for Flynn scrollback. There is no shortcut supplying arbitrary
  dense history pixels without composing them.
- **Heavy output:** BetterTelnet suppresses intermediate drawing and flushes at
  appropriate parser boundaries. Flynn processes 512-byte chunks, may draw at a
  quarter-screen scroll, and bounds additional network draining by a tick
  deadline. Its comment says four ticks but the inspected mono branch uses two.
  These are batching policies, not evidence of faster full-frame composition.
  Clarice already separates two-tick input polling from its 96-tick bulk paint
  cadence and immediate first/pager/final presentation. Copying Flynn's more
  frequent intermediate drawing could delay the final prompt.
- **Selection:** NCSA/BetterTelnet invert affected rectangles; Clarice already
  has tested changed-region highlighting. No change to acceptable input or
  selection is justified by this study.

## Ranked opportunities and decision

Ranking is by suitability for a small follow-up, **not a measured speed ranking**.

| Rank | Idea | Expected benefit / complexity / extra memory | Decision |
|---|---|---|---|
| 1 | Combine dark-theme polarity with glyph stores, analogous to Flynn's AND-NOT path | Could remove the separate inversion pass in dark mode; low-to-medium implementation risk around carry/edge bits; no new buffer required. **No benefit to the measured light-theme 416 ms case.** | Defer until dark inversion is independently shown costly. |
| 2 | Avoid composition of blank glyph scanlines or runs | Content-dependent; checking four glyphs and still writing cleared output may consume the saving. Requires profiling Monaco bitmap sparsity and a matched group-level test; no new allocation needed. | Not clearly promising for dense arbitrary text; no speculative implementation. |
| 3 | Coalesce several row presentations | Targets part of the remaining 61 ms presentation, not the 253 ms glyph cost. A full bitmap violates scope. A small strip competes with glyph storage and adds buffer-management work. | Earlier Clarice strip batching failed to improve complete operations; no new evidence warrants repeating it. |

Attribute-state caching, row pointers, glyph reuse, dirty tracking, bitmap
alignment and deferred output are useful ideas **already substantially present**.
NCSA's direct-window QuickDraw path is simpler, but it visibly clears first.
Our earlier independent QuickDraw-versus-glyph experiment reduced matched SE
dense redraw **805 → 508 ms** before later improvements. That is evidence about
Clarice's own paths, **not a benchmark of NCSA or BetterTelnet**.

Current measured SE breakdown remains approximately **253 ms glyph work**,
**61 ms presentation**, **33 ms row preparation**, **35 ms scrollbar** and
**12 ms model composition**, in a **416 ms** completed redraw. Counters overlap
and are tick-quantized. Removing all presentation cannot remove the dominant
glyph cost. None of these sources demonstrates a cheaper grouped glyph algorithm
than the one already retained in Clarice.

**No production experiment justified; keep 1525.** No rebuild, new host runtime
tests, Snow run, physical-SE benchmark or deployment was performed. Existing
[1525 pixel/performance acceptance](../direct-perf/REPORT.md) remains the evidence;
no third-party speed superiority or new regression pass is claimed. Next priority
remains physical command-loading/nano profiling. If renderer work resumes,
measure within the glyph loop before choosing another algorithm. Existing Git
checkpoint and unpublished work are preserved; no commit, push, tag or release.
