# Phase 1 development test report

8 November 2026. A native 68000 terminal has been built with the existing pinned SkiFree Retro68 image and executed under MultiFinder on the Plus emulator. This is a working development prototype with the limits below; it is not a claim of complete POSIX compatibility or physical-hardware qualification.

## Environment

- Snow 1.5.0-a921b10; Macintosh Plus model; 4,194,304 bytes configured RAM; no overclock or fast-forward.
- User-owned Mac Plus v3 ROM, checksum identifier `4D1F8172`, used locally only.
- Disposable HFS disk copied from the user's recovery media. Although its source filename says “6.0.8”, the running guest reports **System 6.0.7 (`0607`)**. Results must be labelled 6.0.7. MultiFinder is active; Finder was brought to the foreground while Sys6Unix output continued behind it.
- Final MacBinary size: 70,912 bytes; installable disk image: 819,200 bytes. The package includes application source, font source/atlas, notices and build scripts; it excludes ROM/system media.
- 384 KiB application partition. An additional 32 KiB of stack is reserved within that partition before expanding its heap. This fixed the initial stack/heap collision on the monochrome Macintosh baseline. Apple's error reference defines error 28 as that collision: https://developer.apple.com/library/archive/documentation/mac/pdf/Operating_System_Utilities/System_Error.pdf .
- No cache settings or RAM extensions were installed or changed. RAM Doubler, other cache sizes, SE/PB180 and physical Plus testing remain future work.

## Observed checks

- Normal launch from Finder, visible desktop around the terminal, typing and command execution, clean `exit` to Finder, repeated relaunches.
- Foreground/background switching through MultiFinder's top-right application icon, including output behind Finder and subsequent uncovering.
- Original built-ins: `pwd`, `cd`, `ls -la`, `echo`, `cat`, literal `grep -F`, environment assignment/expansion, `>` and `>>`.
- Assembly length/substring/glyph results compared against C/library reference results, including empty strings and odd byte addresses; parser malformed-input checks; command completion check. Guest self-test reports PASS.
- Interactive `ec<Tab>` became `echo `; `cat documents/wi<Tab>` became `cat documents/with\ space.txt `. Executing the completed filename correctly printed the CR/CRLF/LF fixture as three lines.
- Disk history wrapped after 140 commands, still retrieved entry 128, and rejected entry 129. Free application heap was **241,740 bytes before and after** that run. This establishes no accumulation in that exercise, not a universal memory-leak proof.
- Independently extracted HFS bytes matched `alpha beta\nready\n`; overwriting the fork fixture changed only its data fork to `preserved\n`, preserving the resource-fork hash and original `TEXT`/`TEST` type/creator. The final history file had 131,592 bytes and 128 correctly sequenced committed records.
- A running thousand-file listing was interrupted by clicking the visible status bar; it stopped after five entries and accepted the next command.

`guest-report.txt` and `validation.json` contain the extracted guest/host checks. `benchmarks.json` records raw guest tick measurements and their scope.

## Performance

The initial scrolling implementation took 159–162 ticks for a ten-entry listing once the screen filled. Moving existing text with ScrollRect, fixing event-loop accounting, replacing per-file general-purpose number formatting, and batching presentation materially reduced that cost.

Measurements use guest TickCount (60 ticks/second), three successive runs, with fast-forward off. The first-entry measurement in the final implementation includes painting that entry. Total listing ticks start after shell parsing/history and stop before the final idle-loop repaint; they are not full keypress-to-prompt latency. Cache state was not reset between runs. They are emulator results, not physical Plus predictions.

The catalog-only thousand-entry scan took 290–308 ticks (about 4.8–5.1 seconds), including event polling. This is a useful lower-bound comparison, not an `ls` result, because formatting and entry rendering were disabled.

Final thousand-entry listings took **651, 660 and 660 ticks (10.85–11.00 seconds)**, with the first entry painted after **6, 17 and 17 ticks (0.10–0.28 seconds)**. The latter two first-paint measurements also present pending output from the previous benchmark iteration. Bulk display updates are coalesced after 32 ticks of work; event polling remains separate at a two-tick work interval. No extra delay is applied to typing or the first entry.

| Entries | Listing loop, three runs | First row painted |
| --- | --- | --- |
| 10 | 0.25–0.30 s | 0.18–0.23 s |
| 100 | 1.18–1.28 s | 0.20–0.28 s |
| 1,000 | 10.85–11.00 s | 0.10–0.28 s |

These figures exclude shell history/parsing and the final repaint, as explained above.

Bulk output is intentionally coalesced between updates; ordinary input editing and the first listing row do not wait for this cadence. The bounded scrollback retains the latest 128 rows, so a thousand-line listing is not all retained in RAM. Redirect output to an HFS file when the full listing must be retained.

## Limits and remaining qualification

- Full Unix `ls -l` formatting, regex grep, awk, GNU utility ports, vi/nano and a native GCC chain are not implemented. Current commands are original small built-ins. No imported editor is being passed off as an upstream port.
- Terminal escape-sequence support for full-screen editors is a later step. Shell pipelines, globbing, scripts and jobs are unsupported and rejected where recognised.
- Mouse drag/resize and modifier-key history/cancellation were implemented but **not successfully verified through this automation setup**. Drag attempts did not change the window; host-injected Control/Command/Escape/arrow keys did not reliably reach the emulated Plus keyboard. Those paths need manual keyboard/mouse validation. The successful status-bar cancellation does not prove the shortcut paths.
- Clipboard operations, read-only/disk-full failure injection, duplicate volume names, multi-volume traversal, non-ASCII HFS collation and all path round trips need more coverage. Completion is currently case-sensitive even though HFS opening is case-insensitive.
- History has no growing RAM index, but no recent-command cache or RAM fallback on a read-only history volume yet. Failed history writes disable recording. Recent unflushed records may be lost on a crash.
- The self-tests compare assembly output with references; they do not yet provide a complete preserved-register harness or an assembly-versus-C performance tournament.
- Keep the test-only `checks` command on the disposable fixture: it writes named test files and intentionally wraps history. See README before running it.
- The font and runtime notices are included. Version-exact runtime provenance/public-release review remains separate from these local development artifacts.
