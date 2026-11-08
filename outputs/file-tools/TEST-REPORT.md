# File tools integration — Clarice

Test platform: Snow Macintosh Plus, 4 MB, System 6.0.8 and MultiFinder. These are emulator results, not physical hardware measurements. The System API reports 0607 on this established 6.0.8 guest; see the System 6.0.8 verification evidence.

## Implemented

- `df [-h|-k] [--] [path ...]`: mounted HFS allocation capacity, used/free space, percentage and `/mnt` label. Path operands select the backing volume. Default KB means 1024 bytes; `-h` uses binary suffixes. Allocation-block products are widened before multiplication.
- `ls [-lah1P] [--] [path ...]`: current-width row-major columns interactively; single-column script/redirect output. `-1` forces single column. `-a` includes namespace-aware `.` and `..`. Long format retains Mac flags, data/resource fork sizes, type and creator; `-h` formats both forks. Single file operands and multiple paths are supported.
- `status` labels free application heap in KB; `status -h` uses binary suffixes. No installed-physical-RAM claim is inferred from the application memory ceiling.
- Streaming catalog order, one 132-byte row buffer, no directory-sized allocations or sorting pass. Directory entries reuse catalog metadata. First row and page pauses paint immediately; bulk listing paints coalesce at 32 ticks, while event polling remains at two ticks. Ordinary cadence is restored on every listing exit.

## Validation

Host size-formatter tests pass with AddressSanitizer and UndefinedBehaviorSanitizer: 11 decimal/binary boundary cases through 64-bit maximum, including a classic-HFS allocation product above 32 bits. Pinned Retro68 build passes.

The guest integration script and extracted output assertions cover empty-directory dot entries, mounted labels, single-column output, both file forks, df/default/human/path selection, multiple operands, `--`, invalid options (status 2), nonexistent paths (status 1), and exactly 1,000 distinct fixture names plus dot entries. Raw outputs are included. Only one mounted HFS volume was available for this run; multi-volume and removable-media coverage remains future work.

Interactive checks show five short filenames per row at 80 columns and two at 40 columns, a resource fork around 144 KiB for Clarice, and RAM labels in KB. Ctrl+C interrupted a running column listing and returned to the prompt.

## Performance

Three consecutive thousand-file long listings, using guest TickCount at 60 Hz, paging disabled:

| Candidate | Total ticks | First file ticks |
| --- | --- | --- |
| Initial formatter / frequent paints | 2658, 2742, 2706 | 38, 27, 27 |
| Manual row formatting / 32-tick bulk paints | 1027, 1028, 1020 | 19, 33, 25 |
| Final, changed-text choice restored for native drawing | 1027, 1008, 1029 | 14, 28, 34 |

Final totals are 16.8–17.2 seconds, about 62% below the initial candidate median. The final drawing choice produced no material additional timing win in this fixture. This remains slower than the historical 11–12-second renderer, which preceded later scrolling changes. It is not a claim that large long listings are instant. Paging avoids waiting for full enumeration; columns reduce the number of displayed rows for ordinary listings. Repeated synthetic metadata benefits changed-span drawing; diverse real directories still need measurement. Timings exclude command parsing/history and final prompt repaint, and caches were not reset between runs.

Native scrolling pixel comparison passes: 20 output bursts and 20 reverse/jump views, zero mismatches against full redraw. The final narrow-window paged listing also responds to Ctrl+C and returns to the prompt.
