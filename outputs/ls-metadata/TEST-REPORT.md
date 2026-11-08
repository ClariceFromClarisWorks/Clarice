# Creation-first long listings

Implemented and tested in Snow's Macintosh Plus, 4 MB, System 6.0.8 under MultiFinder. These are emulator observations, not physical hardware measurements.

Default `ls -l` columns are kind/Locked/Invisible, logical data-fork bytes, logical resource-fork bytes, creation date/time (`YYYY-MM-DD HH:MM`), and name. Directory fork sizes are blank, not fabricated zeros. Virtual entries without catalog metadata leave dates blank. `--full` adds raw Finder flags, N/B bits and type/creator; `ls --help` explains their meanings and the remaining inapplicable fields. Creation rather than modification time is an intentional difference from conventional Unix `ls -l`.

HFS Finder-invisible files and directories are hidden by default and shown by `-a`, together with dot-prefixed names and `.`/`..`. Explicit hidden operands are still listed. Apple's [Finder flag documentation](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-464.html) describes the invisible bit for both files and folders. Dates and flags come from the catalog record already read; no additional per-entry lookup is introduced. Stored wall-clock timestamps depend on the Mac's clock; no timezone conversion is invented.

## Validation

- Pinned Retro68 build and CLX packer validation passed. Native utility image: 10,956 bytes, freed after invocation.
- Portable date formatter: 59,718 comparisons with Python datetime, including every day boundary in the unsigned classic HFS timestamp range, random values, and endpoints.
- Actual linked 68000 formatter executed with Unicorn's M68000 CPU model: the same 59,718 comparisons passed. Four bounded DIVU operations replace expensive general 32-bit division calls. No date cache or heap allocation is used.
- Guest script completed. Extracted output validated default filtering, invisible files AND folders, dot names, explicit hidden operands, blank directory sizes, extended metadata and help text.
- Files deliberately created at 2000-02-29 12:34 but modified at 2025-01-01 00:00 displayed the creation timestamp.
- Interactive long listing paused at the pager. Ctrl+C returned a fresh prompt and `echo $?` reported 130. Rendering and scrollbar composition code were not changed; this task did not repeat the previous pixel-level scrolling test.

## Performance

Same 80-column Monaco 9 window as the preceding external-utilities milestone. Thousand-file full-output benchmark (paging bypassed): 871, 843, 867 ticks at 60 ticks/second; median **14.45 seconds**. First entry: 17, 32, 38 ticks. Prior build without dates: median 13.27 seconds. This is approximately 1.18 seconds additional cost over 1,000 entries and remains an optimization opportunity.

The first general-division implementation measured 1068, 1040, 1063 ticks (median 17.72 seconds); the measured faster 68000 implementation was retained. Benchmark timing excludes PATH/module loading and final prompt redraw. Output streams throughout; interactive use pages instead of forcing all 1,000 entries onto the display.

Evidence: benchmark.png/json, unoptimized-benchmark.json, date-tests.txt, date-68k-tests.txt, guest-tests.txt and extracted visible-*.txt/ls-help.txt. Test sources are in tests/commands/. To rerun machine-code date checks, link created.o at text address 0x10000, extract a flat binary to work/modules/created.raw, and pass the `nm` address of format_created to created-68k.py in an environment with Unicorn installed.
