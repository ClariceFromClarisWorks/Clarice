# Scrolling, bounded history and Preferences

**Version correction (8 November 2026):** subsequent installation-media and Finder verification identifies System 6.0.8 despite the 0607 environment-query result. See [the 6.0.8 verification](../system608/TEST-REPORT.md). Raw historical results below are preserved.

8 November 2026. Development update; emulator evidence, not real-hardware results.

## Delivered behaviour

- A native vertical scrollbar, with line/page movement and classic thumb tracking, for the terminal's output history. Command history remains a separate capped HFS file.
- Fixed 128 × 132-byte output storage: 16,896 bytes. Preferences choose 32, 64 or 128 retained lines without growing this allocation. Reducing retention discards older lines; increasing it does not resurrect discarded text. New output keeps an older viewed position anchored until its lines expire. Typing returns to the live prompt.
- File > Preferences opens a native modal dialog. `prefs window` opens the same dialog. Installed font name, size, black/white theme, retention and optional bitmap cache have matching command settings. OK persists them; Cancel leaves them unchanged. Existing Font/View menus remain.
- Preferences use a 48-byte, versioned native HFS file; the earlier 44-byte file is accepted. No OS cache, RAM Doubler or other system settings are changed.
- Native QuickDraw pixel scrolling is the Plus default. It reuses pixels in either direction and redraws changed text. Refresh scheduling is reduced from 64 ticks to 3 ticks between paints, with command/event yielding retained. This threshold is not a guarantee of 20 fps.
- Optional system-font bitmap rendering uses an additional 29,696-byte allocation (29,704 bytes including observed heap overhead). Disabling it releases the allocation. Allocation failure falls back to native rendering. Font glyphs are rasterised by the installed OS font system; no replacement custom typeface is used.

## Why this scrolling approach

Apple's TextEdit/QuickDraw guidance describes moving the current view, clamping the scroll position and updating exposed areas. This informed the scrollbar and native pixel-reuse path; TeachText source itself was not inspected or copied. [Apple, Scrolling Through a Document](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-312.html)

SkiFree's local implementation/report was inspected read-only for RAM composition, bounded dirty transfers and clipped fallbacks. The optional renderer applies these principles with original code. A direct framebuffer transfer is allowed only for a foreground, aligned monochrome rectangle completely contained in both the window visibility and clip regions, within the Plus display bounds. Other cases use QuickDraw clipping. Cursor hiding is scoped to the direct transfer. Original SkiFree sources, ROMs and system disks were not modified.

The first cached implementation was slower than native scrolling, so it was not selected as the default. Trimming blank glyph work and using 68000 row transfers improved it. Assembly remains compared with the executable C implementation; native scrolling is retained where measured faster. Initial and intermediate reports are included to make that decision reviewable.

## Test environment

Pinned offline Retro68 image reused from SkiFree; C and assembly target `-m68000`, with warnings treated as errors. Application partition remains 384 KiB, including the existing 64 KiB stack reservation.

Snow 1.5.0-a921b10, Macintosh Plus ROM, 4,194,304 bytes physical RAM, MultiFinder, no overclock or fast-forward. The available disposable guest identifies as **System 6.0.7**. System 6.0.8 remains the requested target and is not claimed tested by this run. No physical Plus, SE, single-Finder or System 7 test is implied.

## Automated guest checks

`scrollbench` executes inside the 68000 guest:

- Packed glyph compositor compared byte-for-byte against C for widths 1–8 and heights 1–32, including high-bit character indices and guard space.
- Assembly row copies checked for widths 1–64, five rows and untouched guard bytes.
- Ring wrap after 300 lines at each 32/64/128 limit; anchored views, expiry and non-resurrection after increasing retention.
- Four rendering-cache allocation/release cycles checked against free heap.
- Twenty incremental forward/backward cached views compared against freshly rendered pixels.
- Three runs of 90 varied, full-width line updates for each native/C/assembly renderer. Timings include output insertion, presentation and cooperative event handling; disk report writing occurs afterwards.

Quick diagnostic reports are explicitly marked by their 30-frame length and do not replace the full compositor test.

## Remaining scope

System 7 process monitoring, multiple terminal sessions, GNU utility/editor ports and full POSIX sh conformance remain separate work. Existing scripting remains the documented bounded subset. The optional bitmap cache is not evidence of a universal speed improvement across fonts, window positions or later machines.

## Measured renderer comparison

Monaco 9, 80 columns × 21 output rows; 90 forced full-width updates per run, 60 guest ticks per second:

| Renderer | Run ticks | Approximate updates/second | Largest measured frame |
| --- | --- | --- | --- |
| Native QuickDraw | 505 / 504 / 505 | 10.7 | 5 ticks (83 ms) |
| Cached C reference | 682 / 682 / 683 | 7.9 | 7 ticks (117 ms) |
| Cached 68000 assembly | 568 / 568 / 568 | 9.5 | 6 ticks (100 ms) |

The assembly cache improves on its C reference but loses to native rendering on this Plus workload. Native is therefore the default and saves the cache allocation. These are forced-update throughput measurements, not a claim that every command runs at that rate. The full run reported zero failures and a 29,704-byte heap difference for the cache; four release cycles returned to the same heap size. See `full-scroll-report.txt` and `full-benchmark.png`.

Frequent unconditional pixel scrolling initially regressed the 1,000-file listing to 1959 / 1902 / 1898 ticks; that candidate was rejected. A native changed-span variant passed its framebuffer comparison but regressed further to 2178 / 2186 / 2192 ticks and was also rejected. The next candidate applies sparse character updates and narrow transfers inside the bitmap renderer instead. The forced single-line renderer comparison above is not an adaptive multi-line burst benchmark.

## Cursor correction and scope clarification

The blinking underscore was replaced with a full-cell inverse block. Previously each blink marked text dirty and forced the prompt row through erase/redraw, explaining the reported dollar-sign flicker. Blinking now only inverts the cursor rectangle; both renderers keep the cursor out of their text/cache state. A content redraw removes the overlay before drawing and restores it afterwards. Unchanged prompt rows are no longer forced to redraw.

The updated 68000 build launched in the Plus/4 MB MultiFinder guest. Native light and cached dark/light block cursors were observed, and `pwd` returned the startup directory. The sparse cached renderer's actual framebuffer comparison passed 19 burst sizes with zero pixel mismatches. The full compositor/timing suite has not yet been rerun for this candidate. Static screenshots and pixel equality do not establish absence of transient scrolling flicker: that broader visual acceptance remains open, especially the native erase/draw path. Cache and theme were restored to off/light after checks.

The architecture now explicitly distinguishes built-in implementations from faithful upstream ports, POSIX shell language goals from platform compliance, and cross-built C utilities from the future native C toolchain. No GNU grep, awk, nano, SSH or external command loader is claimed implemented.
