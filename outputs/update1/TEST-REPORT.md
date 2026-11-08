# Sys6Unix terminal and scripting update

8 November 2026. Development build; Macintosh Plus emulator, 4 MiB, MultiFinder.

Installed-font rendering, saved preferences and paged native-metadata listings are implemented and exercised in the guest. A bounded POSIX-language scripting adapter now runs the selected fixtures. **The requested full POSIX sh compatibility is not complete.** The missing semantics are listed below and in the project README; this build must not be described as a conforming POSIX shell.

## Terminal and native files

- Monaco 9 is the default installed font. The normal window has 80 columns and 21 output rows on the Plus. Font/View menus and the `prefs` command provide font selection, size, reverse colours, paging and window sizing. Installed Geneva was selected, saved and visibly restored after restarting the emulator; the dark theme survived too. Monaco and the light theme were then restored for baseline use.
- Preferences occupy a versioned 44-byte native HFS file, `Sys6 Preferences`, separate from executable resources and history. Final extracted bytes identify Monaco 9, light colours and screen paging. Invalid/truncated formats fall back to defaults; disk-full/corruption injection has not been exercised in the guest.
- Proportional fonts remain usable in fixed cells, but have fewer columns and a slower per-character drawing path. The measured baseline is Monaco.
- `prefs fill` expanded the document window while retaining the menu bar; a second invocation restored its previous position and dimensions. Native mouse dragging/grow-box operation and direct Font/View menu selection remain manual-validation gaps in this automation setup; command equivalents were exercised.
- `ls -l` uses one catalog record per entry to print kind, Locked/Invisible/Name-locked/Bundle indicators, all raw Finder flag bits in hexadecimal, data/resource fork lengths, type, creator and name. No resource-fork open is added. The guest report includes the invisible Desktop file and its resource-fork length. This is native metadata, not fictitious Unix ownership/permissions. Legacy flags are not promoted as security protections; `chmod` is not redefined.
- Default `ls` stopped after entries 0–19. Space advanced to 20–39; Return advanced exactly one line; Q returned to the prompt. Enumeration pauses with the display. Scripted or redirected output does not page, and `ls -P` explicitly disables paging. The View menu/`prefs paging` supports screen, 100-line and off modes. Shift-arrow scrollback while paused is implemented but not verified through this emulator's injected modifier keys.
- Switching to Finder while a listing was paused left the terminal clipped behind it and the command intact. Returning to the terminal and advancing/cancelling worked. See `finder-while-paging.png`.

## Performance and memory

The renderer uses native QuickDraw text. Changed-character spans avoid redrawing unchanged columns; proportional fonts retain a fixed-cell path. Bulk output paints are coalesced at 64 ticks, while prompt editing, first-entry display and page pauses bypass that interval. This batching is not the input latency.

| 1,000-entry rendered listing | Three runs, guest ticks | Approximate seconds |
| --- | --- | --- |
| Historical bitmap font, fewer metadata columns | 651, 660, 660 | 10.85–11.00 |
| Initial native font, complete changed rows | 915, 928, 927 | 15.25–15.47 |
| 64-tick bulk batching, complete changed rows | 819, 835, 837 | 13.65–13.95 |
| Final changed-character drawing, Monaco 9 | **713, 713, 719** | **11.88–11.98** |

The final richer listing remains about one second slower than the earlier sparse listing. It does not require an interactive user to wait for the entire directory: screen paging is now the default. Final first-entry times were **5, 10 and 11 ticks** (0.08–0.18 seconds). These are not full-page latency measurements. The synthetic fixture repeats metadata and has similarly named files, so the changed-character optimization benefits from common columns; mixed real directories still need hardware measurement. Benchmark ticks exclude history/parsing and the final prompt repaint. Raw observations are in `benchmarks.json` and `benchmark-1000.png`.

The application still requests a **384 KiB partition**. Its stack reservation increased from 32 to 64 KiB for nested parsing/execution. Launch-to-ready in the extracted run was 52 guest ticks. Free application heap before and after 140 repeated commands was **163,178 bytes in both cases**. This is an application-heap observation, not total free physical RAM or a complete peak-memory/stack-canary measurement. History remains capped at 131,592 disk bytes; scrollback is a fixed 128 × 132 bytes.

## Scripting evidence and limits

The MIT-licensed mrsh parser/AST is pinned to commit `4c81598721bc5eeb28f9faa818b3102d0471b7f6`, with copyright and licence retained in `src/vendor/mrsh`. Local parser fixes cover left-associative AND/OR lists, unary arithmetic precedence, ternary whitespace and compiler portability. The upstream Unix process/runtime layer was not imported. `src/sh.inc` is the Macintosh adapter.

`tests/shell` contains 30 selected fixtures covering quoting/empty arguments, variables and default/trim expansions, conditional lists, if/elif, for/while/until, break/continue, functions/return, positional parameters, set/shift, case, environment isolation in a subshell, command substitution, finite pipelines, arithmetic, printf, here-documents, comments and file redirection. All 30 host comparisons passed with address/undefined-behaviour sanitizers. The final Plus run produced the same **742-byte** expected transcript, including exit statuses. No host File Manager mock result is substituted for the guest evidence.

Scripts run with `sh file [args]`, `. file`, or an explicit `.sh` path without a Unix executable bit. Pipeline stages run sequentially using temporary HFS data files, so RAM does not grow with pipe volume. The final offline catalog contained no remaining `Sys6 temp` files. Finder briefly displayed a cached temp-file icon; the actual disk catalog was checked after clean exit.

This is still a subset. In particular, concurrent/infinite pipelines, signals/traps, jobs/process identities, descriptor duplication/closing, general external executable loading, aliases, shell options, readonly/export attributes and exact environment inheritance, complete IFS/globbing/positional-parameter semantics, all arithmetic assignments, compound-command redirection, and complete standard utilities remain unfinished. A standalone quoted `"$@"` is supported; embedded quoted forms are explicitly rejected. Some other semantic differences remain, so arbitrary POSIX scripts cannot be assumed to run unchanged. GNU utility/editor ports and bash/zsh extensions remain separate work.

The baseline has explicit source, argument, variable and recursion limits, documented in README. Temporary files are cleaned on normal completion/cancellation; a crash may leave them behind. Conservative parser working-memory checks are present, but forced allocation failures and adversarial parser nesting are not fully validated. Do not present the selected regression suite as conformance certification.

## Filesystem integrity and environment

`validation.json` checks the exact script transcript, guest regression statuses, stable repeated-command heap, history ring sequences/cap, preference bytes and data-fork outputs. Data-fork replacement preserved the fixture's resource fork byte-for-byte and retained its TEXT/TEST type/creator. The resource-fork SHA-256 remains `9a0910838a52e9d22de0ca64fcdbad0d78516525652d0f1df867ca0f420ad89f`.

Tests reused the pinned SkiFree Retro68 Docker image, with offline builds targeting 68000. Snow was the project's copied 1.5.0-a921b10 installation, configured for a Macintosh Plus with 4 MiB and MultiFinder. **The available guest reports System 6.0.7, not the required 6.0.8.** System 6.0.8, single Finder, SE, later machines and physical hardware remain unverified in this update. No ROM, original system image, SkiFree source or system cache/RAM Doubler setting was changed. No ROM or system disk is redistributed.
