# Clarice

A native System 6 command prompt for a Macintosh Plus with 4 MB, built using the existing SkiFree Retro68 toolchain. This is a working Phase 1 prototype, not a Unix kernel, bash implementation or collection of completed GNU ports.

## Current build and priorities

Download the application disk or MacBinary package from [GitHub Releases](https://github.com/ClariceFromClarisWorks/Clarice/releases). The initial published build is a development snapshot with the limitations below, not a claim of POSIX conformance. No ROM, operating system or emulator is included.

Verified baseline: Snow emulating a Macintosh Plus with 4 MB, System 6.0.8 and MultiFinder. Physical hardware remains unverified. Clarice requests a **768 KiB application partition**, including its 64 KiB stack reservation; command working allocations are demand-driven within that partition. The source-port allocation ceiling is 96 KiB per invocation, separate from loaded code and shared runtime memory.

The four planned utility groups are implemented as loadable `/bin` programs, with the option and platform limitations below. They are not a claim of complete Unix or GNU compatibility.

| Area | Implemented | Next work |
| --- | --- | --- |
| Terminal | Wrapped editing, solid buffered cursor, bounded typeahead/history/scrollback, native Preferences with installed-font dropdown, saved geometry and themes | Further speed and font/geometry coverage |
| Native files | HFS, `/mnt`, symlinks, PATH/profile, fork-preserving file operations and `hfsattr` | Broader filesystem/error coverage and optimisation |
| Source ports | sbase grep/sed and everyday pipeline tools; One True Awk | Compatibility and large-input performance |
| Streams and shell | stdin/stdout/stderr, redirection, exit statuses, finite sequential spool pipelines, bounded sh subset | Broader POSIX runtime coverage; concurrent pipes are not implemented |
| Editor | No working editor in this release | Faithful upstream nano port, starting with tiny configuration |
| Connections | Experimental serial source only, not a verified usable client | Deferred behind nano; Telnet/SSH need separate networking work |

Grep/sed are sbase ports, not GNU grep/sed. Awk preserves One True Awk source. Credits, licences and staged adaptations are in [THIRD_PARTY.md](THIRD_PARTY.md). Current validation: [utilities](outputs/userland-next/TEST-REPORT.md), [interaction](outputs/interaction/TEST-REPORT.md), [preferences](outputs/preferences-text/TEST-REPORT.md). The earlier 1,000-entry `ls -la` result was 11.70 seconds median; it is historical emulator evidence, not a fresh timing for every release. Creation date/time, paging and cancellation are retained. See [listing measurements](outputs/ls-opt2/TEST-REPORT.md).

## Build

Install Docker and Python 3, then fetch the pinned image once:

```sh
docker pull --platform linux/amd64 ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
./scripts/build.sh
```

The project reuses SkiFree’s pinned toolchain; it does not require a SkiFree checkout. Builds are offline, target the original 68000 and produce `outputs/namespace/Clarice.bin` (MacBinary) and `Clarice.dsk` (800 KB floppy image). The current development build requests a 768 KiB partition under MultiFinder (including the awk port). The terminal uses installed Monaco 9 by default. File > Preferences opens a native dialog with installed-font, scrollback, paging and rendering-cache dropdowns, plus font size, monochrome colours, columns and fill-screen/restore. The separate Font menu is removed; `prefs` provides equivalent command-line controls. Settings use the bounded, editable text file `/etc/clarice/clarice.conf`; command history uses `/etc/clarice/clarice.history`. Legacy root-level files migrate automatically, without overwriting an existing destination. A failed migration retains the original. See `THIRD_PARTY.md` for the mrsh parser attribution.

## Use

Launch Clarice from Finder to install its data folders automatically, then use the shell. The application can remain outside the System Folder. Move/resize its normal document window. MultiFinder's upper-right application icon switches between Finder and Clarice while commands run.

External programs under `/bin`: `ls df cat grep sed awk wc head tail cut tr sort uniq tee cp mv touch mkdir rmdir rm ln readlink hfsattr find du file od basename dirname more`. Each provides `--help` with supported options and limits. `benchls` and `benchscan` are additional diagnostics. Shell-state commands such as `cd`, `export`, `history` and `prefs` remain builtins; use `help <builtin>`. `-h` means human-readable sizes for `ls`, `df` and `du`, not generic help.

The scripting adapter implements a tested subset of POSIX sh: lists, `&&`/`||`, quoting and variable expansion, `if`, `for`, `while`/`until`, `case`, functions/return, positional parameters, `set --`, shift, command substitution, arithmetic, here-documents, input/output redirection and finite pipelines. Run `sh path/file.sh` or `./file.sh`; no executable permission bit is required. **This is not yet a complete POSIX sh implementation.** See the detailed limits below. Bash/zsh remain later targets.

`ls -l` displays kind, Locked/Invisible indicators, data-fork bytes, resource-fork bytes, creation date/time and name. `ls --full` adds raw flags and type/creator codes. These come from the same catalog lookup; resource forks are not opened. It does not invent Unix permissions, owners or link counts. The legacy bit sometimes called “bozo” is not presented as a modern permission or protection. No `chmod` permission emulation is provided.

Listings pause after a screen by default, stopping further enumeration until Space (next screen), Return (one more line), or Q (cancel). Clicking the temporary More prompt advances a page. `ls -P` disables paging. Redirected/scripted listings never page; View or `prefs paging off|screen|100` changes the interactive default. Creation date/time is implemented; full Unix `ls` option compatibility remains work.

`prefs window` opens the same Preferences dialog. `prefs scrollback 32|64|128` sets retention, and `prefs cache on|off` controls only the terminal rendering cache. No OS disk cache or RAM Doubler settings are changed.

Preferences examples: `prefs font Monaco`, `prefs size 9`, `prefs colours dark`, `prefs colours light`, `prefs columns 80`, `prefs fill` (toggle fill-screen/restore). `prefs fonts` lists installed fonts. Proportional fonts use fixed cells based on their maximum character width, reducing available columns and drawing speed. Monaco is the recommended baseline. Fill-screen retains the menu bar and is reversible without restarting the shell.

Completion currently matches the case of candidate names, although HFS opens are case-insensitive. Tab completes commands and HFS paths; another Tab shows up to 50 matching paths. Arrow keys or Control-P/N recall history; Control-A/E/B/F move the cursor; Control-W/U/K/Y edit; Control-R searches history. Option also acts as Control for keyboards lacking a Control key. Edit > Previous/Next command supplies a menu route on original Plus keyboards. Copy/Paste use Macintosh menus. Pasted line breaks become spaces and never execute automatically. The native vertical scrollbar or Shift-Up/Down scrolls bounded output history. New output keeps the viewed history anchored until those lines expire; typing returns to the live prompt. Cancellation is available from File > Cancel command, Command-period, Control-C or Escape where the keyboard/emulator supplies those keys.

`/` is backed by `System Folder:Clarice Data:` on the running System volume. First launch creates missing directories and `/etc/profile`; subsequent launches preserve them. `/mnt/<volume label>` exposes Finder-mounted media. `pwd` and `$PWD` show the full physical shell path; the prompt shows only its final directory name, optionally prefixed by `$HOSTNAME` and a space. At root it shows `/ >`. No disk/network lookup occurs while drawing the prompt. Volume names retain Finder spelling; duplicate labels fail explicitly instead of choosing an arbitrary disk. `%2F` and `%25` encode literal slash and percent in HFS names. Native paths remain available as `native:Disk:Folder:File`.

`ln -s target link`, `readlink link`, `rm file`, and `mkdir directory` are supported. Links are HFS files with Clarice metadata and a bounded target; ordinary Mac applications do not follow them. Relative targets resolve from the link's parent. Resolution follows at most 16 links, supports dangling links, and rejects loops. `rm` removes a link itself; `rm -r` recursively removes directories within the documented depth bound. Hard links are unsupported. `ls -l` identifies links with `l` without opening each target. `test -L` tests the link itself; `test -e/-f/-d` follow it.

The built-in PATH is `/bin:/usr/bin`. `/etc/profile` is sourced once at startup and can replace or extend it, for example `export PATH="$PATH:/mnt/Tools/scripts"` and `export HOSTNAME=macplus`. Current lookup supports `.sh` files or files beginning with `#!/bin/sh` or `#!/usr/bin/sh`. Native CLX1 command images are also supported, as described below. Other interpreters and general Unix executable formats remain unsupported. Built-ins work independently of PATH. A slash in a command uses that path directly; otherwise PATH is searched in order. Empty PATH entries explicitly select the current directory. Tab completion includes PATH scripts. Environment values currently have a 127-byte limit.

History uses at most 128 fixed 1,028-byte records plus an 8-byte header (131,592 bytes on disk). A leading space excludes a command. History retrieval uses a 1 KiB buffer, not a growing RAM list. It flushes on clean exit; power loss can lose recent unflushed commands. Scrollback is a separate fixed 128×132 byte ring (16,896 bytes); retention can be limited to 32, 64 or 128 lines. Reducing retention discards older lines. The Plus default uses native QuickDraw pixel scrolling with the bitmap cache off, following measured comparisons. The optional system-font/bitmap cache requests another 29,696 bytes (29,704-byte measured heap delta including allocator overhead) and releases them when disabled. Long command lines stop at 1,023 bytes.

## Developer tests

`scrollbench native` compares adaptive burst redraws with a full repaint of the actual visible framebuffer and writes `/tmp/native-scroll-report.txt`.

`scrollbench quick` is a shorter diagnostic; only the full run covers all glyph widths/heights.

`scrollbench` compares 90 full-width frames in each of three runs with native QuickDraw, cached C, and cached 68000 assembly rendering. It also compares assembly against C and incremental scrolling against full rendering, and writes `/tmp/scroll-report.txt`. Use Monaco 9 and 80 columns for repeatable comparisons; the test replaces visible scrollback.

`selftest` checks assembly string/glyph results against C/library references and tests parsing/completion. `benchls path` runs three rendered directory listings and reports guest ticks; `benchscan path` excludes file text output to isolate catalog traversal. These timings are emulator observations, not physical Plus measurements.

`checks` / Tests > Run checks is for the disposable fixture only: it writes `/tmp/phase1-report.txt`, `Documents/result.txt`, and `Documents/fork-test`, and overwrites the bounded history with 140 test commands. Do not run it against personal files. Prepare fixtures with `scripts/fixtures.py` only with the project emulator stopped. `scripts/emulator.py` controls only the copied project Snow instance and disk. Original ROMs and disks are never modified or bundled.

See `outputs/scrolling/TEST-REPORT.md` for scrolling/preferences and `outputs/update1/TEST-REPORT.md` for the earlier scripting update and `outputs/phase1/TEST-REPORT.md` for the historical baseline. Architecture and later GNU tools/editor/compiler work are in `outputs/ARCHITECTURE.md`.

## Scripting limits in this development build

Thirty selected script fixtures match the host reference and the Plus guest transcript. They are regression coverage, not a POSIX conformance certification. The shell uses mrsh's parser with an original Mac execution adapter; it does not provide Unix processes.

Pipelines run sequentially through temporary native HFS files, with cleanup on normal completion/cancellation. Infinite producers, concurrent pipeline interactions, signals/traps, job control, process IDs, descriptors beyond 0–2, general Unix executable formats and complete standard utilities are not implemented. Full GNU option parity and an upstream editor remain future work. The installed grep/sed are sbase ports; consult their bundled manual pages for upstream options and the port report for limits.

Other known gaps include shell options (`set -e/-u`), aliases, readonly/export attributes and exact environment inheritance, full IFS edge cases, embedded quoted `$@`, wildcard parent-directory components, locale/collation, all arithmetic assignment forms, all `test`/`printf`/`read` variants, compound-command/function redirection and full subshell positional-parameter isolation. Several unsupported forms report an explicit error; remaining semantic differences mean arbitrary portable scripts cannot yet be assumed to work unchanged. Pathname globbing is limited to the final component and at most 32 total arguments. A standalone quoted `"$@"` preserves separate/empty arguments.

Baseline bounds: 8 KiB source per invocation, conservative 256-token syntax guard, 16 execution levels, 8 nested source invocations, 32 arguments/4 KiB aggregate argument storage, 2,047-byte expansions/substitution output, 16 variables with 31-byte names and 127-byte values, and 16 functions. Parser memory checks may reject scripts below the text cap when insufficient heap remains. The stack reservation is now 64 KiB within the 768 KiB application partition. Forced allocation-failure and adversarial parser-depth testing remain incomplete.

Run `scripts/test-shell.sh` for the host adapter comparisons with address/undefined-behaviour sanitizers. The host fixture mocks File Manager operations; it does not validate QuickDraw or HFS. Guest fixtures are in `tests/shell/`; install them only on the disposable disk and run `sh shcases/capture.sh`. The expected byte transcript is `tests/shell/expected.txt`.

Cursor update: solid full-cell inverse block; no blink timer. See outputs/scrolling/TEST-REPORT.md for current verification and remaining scrolling acceptance.

## Clarice identity and Plus keyboard

The application uses original 32×32 and 16×16 monochrome command-window icons, generated by `scripts/make-icon.py`. Its application creator is CLRC; existing shell link metadata remains S6UX for format compatibility. Earlier development data is preserved, not automatically migrated into Clarice Data.

Use Command-period to interrupt on an original Plus keyboard; Command-C remains Copy. The project-local Snow input patch also supports host Ctrl-C by mapping host Control to guest Option on non-ADB models. This uses Clarice's Option-C interrupt handling. It is a host convenience, not an original hardware Control key. An interrupt discards pending input or cancels the active command and returns status 130.

Up/Down navigate command history when the keyboard supplies arrow events; Down past the newest entry restores unfinished input. Shift-Up/Down navigate scrollback. The permanent bottom hint bar has been removed, reclaiming a text row; only a temporary paging prompt remains. The project-local Snow patch delivers Up/Down and host Ctrl-C in the Plus guest. Unpatched Snow drops these keys. Edit > Previous/Next command and File > Cancel command remain available. Left/Right host keys remain disabled on this non-ADB emulator pending separate OS translation work. Menu cancellation shares the interrupt path, including idle input cancellation.

Emulator input patch and guest evidence: `outputs/emulator-input/TEST-REPORT.md`. Rebuild with `scripts/build-emulator-input.sh`; this host emulator build uses pinned Rust/Cargo dependencies and is separate from Clarice’s pinned Retro68 build.

System 6.0.8 / Plus / 4 MB verification: [test report](outputs/system608/TEST-REPORT.md). Full scrolling and memory checks, native/cached pixel comparisons, Up/Down history and command interrupts pass. Shift+Up/Down mapping is corrected and buffered native scrolling avoids erase-before-draw flashing; see the [scrolling correction report](outputs/scroll-fix/TEST-REPORT.md). Scrollbar track clicks pass, and the user confirms manual thumb dragging works. Automated drags do not enter guest tracking. Finder and installation media confirm 6.0.8 despite SysEnvirons reporting 0607. Launch the separate local test workspace with `python3 scripts/emulator.py start608`.

Preferences are verified on the Plus / 4 MB / System 6.0.8: installed font and size, both monochrome themes, rendering cache on/off, and 32/64/128 retained lines survive quitting and reopening Clarice. Open File > Preferences (Command-comma), or run `prefs window`; `prefs` displays current values. Chicago persistence and numeric validation are corrected. [Evidence](outputs/preferences/TEST-REPORT.md).

### Disk space and file listing

`df` reports HFS allocation capacity, used and available space for mounted volumes; `df -h` uses binary units (KiB/MiB). Path operands select their backing volume, for example `df -h /`. Default disk-space units are KB (1024 bytes), not physical device capacity.

Interactive `ls` streams names across the current terminal width in catalog order, using a fixed row buffer. `ls -1` forces one entry per line; scripts and redirected listings use that format automatically. `ls -a` includes `.` and `..`; `ls -lh` shows human-readable data and resource fork sizes alongside Mac flags. Multiple paths and `--` are supported. Paging and interruption remain available.

`status` reports the application's free heap in KB; `status -h` selects binary size suffixes. These figures are application memory, not a measurement of installed physical RAM.


## External commands

`ls`, `df` and `cat` now execute native 68000 CLX1 images from `/bin`, found through `PATH` (default `/bin:/usr/bin`). Their utility implementations are outside the shell executable. The build packages a small multicall image; installation writes it under each command name and also installs `benchls`/`benchscan`. These files can be replaced independently. The installer updates only files bearing its CLRC/CLX1 metadata and preserves other files occupying those paths.

Standard input, output and error support `<`, `>`, `>>`, `2>`, `2>>`, descriptor duplication such as `2>&1`, and closing with `>&-`/`<&-`. Redirections apply left to right and restore after each command. Finite pipelines remain sequential HFS spools, not concurrent Unix pipes. Pipeline status is the last command's status; a user interrupt stops the complete pipeline with 130. Missing commands return 127 and invalid native images return 126. Utilities report their own success/failure statuses. UI/diagnostic builtins such as `prefs`, `status` and `scrollbench` currently write directly to the terminal; redirecting their display does not capture it.

`cat [-u] [--] [file ...]` reads stdin when no file is given or an operand is `-`. Terminal stdin uses a fixed 1 KiB canonical line queue: Return submits a line, Ctrl+D ends input on an empty line, and Ctrl+C cancels. On original Plus keyboards Option+D/Option+C are substitutes; the test emulator maps host Control to those guest keys. This is not a raw-terminal/termios implementation or a complete portable C stdio library. `read` remains limited to one variable and 127 characters.

The loader uses a fresh bounded allocation per invocation and frees it on return. CLX1 commands share Clarice's address space and must cooperate with cancellation; they are not protected processes. See `outputs/COMMAND-ABI.md` and `outputs/external-tools/TEST-REPORT.md`. Grep and sed now use this loader and a bounded source-port runtime. Their owned allocations are capped at 96 KiB, including buffers and an 8 KiB reuse cache, and freed on return. This cap is not reserved memory; actual available application memory can be lower. There are at most 12 streams and 16 KiB per input line. Data forks are read without CR/LF conversion. ASCII/C-locale input is the tested baseline.


### Reading ls -l

The default columns are `kind+LI  data-bytes  resource-bytes  created  name`.

| Field | Meaning |
| --- | --- |
| First character | `d` directory, `l` Clarice symlink, `-` ordinary file |
| Next two positions | `L` locked, `I` Finder-invisible; `-` means the flag is clear |
| Data/resource sizes | Actual logical fork lengths, in bytes unless `-h`; zero means an empty fork |
| Created | Stored HFS creation date/time, `YYYY-MM-DD HH:MM`; not modification time |
| Name | File/directory name; directories normally have a trailing slash |

Directory fork sizes are blank because file fork lengths are not applicable. Virtual entries without a backing catalog record leave unavailable sizes/dates blank. No Unix permissions, owner, group or link-count placeholders are invented.

`ls --full` selects a more detailed long listing: it adds N/B flag bits, raw Finder flags in hexadecimal and stored four-byte type/creator codes. N/B have OS-dependent meanings. `0000` is a real zero flag value. Nonprintable type/creator bytes are rendered as periods; a stored `????` code is literal metadata. Directory type/creator fields use `---- ----` because these file fields are not applicable. `ls --help` explains both layouts.

HFS supports the Finder-invisible bit (`0x4000`) for files and folders. Ordinary directory listings omit those entries and dot-prefixed names. `-a` includes both, plus synthetic `.` and `..`; explicit operands are still shown regardless of hidden status. The hidden check and creation date use the existing catalog record, with no extra file lookup or resource-fork reads.

The displayed creation time is the classic Mac's stored wall-clock value; no timezone conversion or accuracy beyond the saved system clock is implied. The portable formatter covers the complete unsigned classic HFS timestamp range (1904–2040), checked against an independent date implementation across every day boundary. Creation time as the default is an intentional Clarice difference from the usual Unix ls modification-time display.

## Rechecking the build

Host checks (C compiler and Python 3 required):

```sh
./scripts/test-shell.sh
python3 tests/commands/host-streams.py
python3 tests/commands/pack.py
python3 tests/terminal/check.py
./tests/ports/build-host.sh
python3 tests/ports/grep.py
./tests/ports/build-sed-host.sh
python3 tests/ports/sed.py
python3 tests/commands/listing-output.py
```

Run the build first for the packer check. The last command checks saved guest evidence; it does not start an emulator. Emulator helpers expect a privately prepared local workspace and legally obtained ROM/System media. Host checks do not substitute for guest testing.

Original project briefs are preserved in `references/`; later decisions in this README and `AGENTS.md` supersede historical proposals. Third-party source and notices retain their own licences; see `THIRD_PARTY.md`. No blanket project licence is granted by making this development repository public.

A clockwise one-cell activity indicator appears at the top right after five seconds without visible output, advancing every half-second. Output, completion or cancellation removes it. Input waiting and paging suppress it; future interactive programs must explicitly claim screen ownership. It does not enter command output or scrollback. See [validation](outputs/grep/ACTIVITY-TEST.md).


### Editable preferences and temporary files

`/etc/clarice/clarice.conf` accepts `key=value` lines, comments beginning with
`#`, and Mac CR or Unix LF line endings. It is limited to 2 KiB and is read once
at launch. It is never executed as a shell script. Invalid individual values
retain defaults; unknown keys are ignored. Missing installed fonts retain
Monaco. Settings changed through the dialog or `prefs` rewrite known keys;
custom comments and unknown keys are not retained on save.

Keys: `font`, `size` (6–24), `colours` (light/dark), `paging`
(off/screen/100), `scrollback` (32/64/128), `renderer` (native/c/asm),
`width`, `height`, `x`, `y` and `fill` (0/1). Native means cache off;
C and ASM select the two cache implementations. Width/height/position describe
the normal window; fill-screen preserves that restore geometry. Columns are
derived from width and font. The dialog changes window width when its column
value is edited, and otherwise keeps pixel geometry when changing fonts.

Saving first writes a new file and retains the previous committed version as
`clarice.conf.bak`. If the primary is missing after an interrupted replacement,
the backup is read on launch. This is not a guarantee against disk corruption
or power loss before the filesystem has flushed.

Pipeline and substitution scratch files live under `/tmp` and are removed
when their operation finishes or is cancelled. Diagnostic reports also go
there. Clarice does not indiscriminately erase user/script files in `/tmp` on
exit. Crash-orphan cleanup is not yet implemented. `/etc/profile` remains the
separate, editable shell startup script; programs belong in `/bin` or `/usr/bin`.

### Everyday pipeline source ports

The build includes independently loaded `wc`, `head`, `tail`, `cut`, `tr`,
`sort`, `uniq`, `tee`, `basename` and `dirname` from the pinned sbase source snapshot.
Each accepts `--help`; upstream manuals and author/licence notices accompany
its source. These are sbase implementations, not GNU Coreutils replacements
with every GNU option. `uniq` compares adjacent lines, and its `-s` count is
bytes in this upstream version. Character-aware `wc`, `cut` and `tr` process
UTF-8; the Macintosh terminal still renders installed MacRoman fonts.

The shared port runtime limits owned allocations to 96 KiB per invocation,
line-oriented input to 16 KiB per line, and open streams to 12 including
stdin/stdout/stderr. Module images and the shell's existing shared stdio pool
are separate from that allocation cap. Memory and file references are released
on return, error and interruption. Pipes remain sequential `/tmp` spools,
not concurrent Unix processes. `sort` spills bounded 128-line/16-KiB chunks
to owned files under `/tmp` and merges them; `tail -f` yields and is cancellable.
The runtime separately bounds raw descriptors to nine nonstandard descriptors
and tracks up to twenty private temporary names. `tail -m` and `tee -i` fail
explicitly because those behaviours are not implemented.

Reproduce the host comparisons with `tests/ports/build-pipelines-host.sh` and
`python3 tests/ports/pipelines.py`; build the 68000 application with
`scripts/build.sh`. Guest evidence is recorded in `outputs/pipelines/`.

### Interactive command editing

Long commands wrap across terminal rows, including the bottom row. The block
cursor is composed with buffered text, rather than erased before every paint.
Typing during a noninteractive command is retained in a fixed 1024-key queue
and replayed at the next prompt; a full queue beeps. Pager and interactive
program keys retain their existing meaning.

An unescaped backslash immediately before Return continues the command at a
`> ` prompt. Ctrl+C discards the unfinished command; history stores the joined
command. This is explicit escaped-newline continuation, not a claim that every
incomplete shell construct automatically prompts for further input.

The quiet-command activity indicator retains overdue animation steps and
catches up in order when cooperative execution resumes. See
[interaction validation](outputs/interaction/TEST-REPORT.md).

### Native utility and awk limits

`cp` copies both forks and Finder metadata, staging file replacements; `-p`
also preserves file dates/locking, not directory metadata. Recursive copy,
removal and traversal have an eight-level bound. `mv` supports same-directory
rename and same-name directory moves; file fallback copies both forks across
volumes. Arbitrary directory rename-and-move/cross-volume moves are unsupported.
`touch` leaves new type/creator unset and preserves existing metadata; `-c`
is supported, but Unix access times and timestamp-setting options are not.
`hfsattr` edits four-byte type/creator, Locked and Invisible; `chmod` is absent
and offers an hfsattr hint when unresolved through PATH. These are different
metadata models, not interchangeable permission commands.

`find` supports name/type/maxdepth/print predicates combined with AND; no exec,
OR or bracket-pattern syntax. The default depth limit reports incomplete
traversal. Use an explicit `/mnt/<volume>` path to traverse a mounted volume;
virtual mount-root enumeration is not implemented by find. `du` totals allocated
fork storage, excluding directory catalog overhead. `file` reports HFS metadata
and Clarice links, not libmagic content detection. `od` supports byte/16-bit
integer and character formats, not floating-point or 32/64-bit formats. `more`
supports forward screen/line paging, Q and Ctrl+C; no search/backward file scan.

One True Awk supports its upstream pattern/action, arrays, functions and
software-floating-point evaluation within a 96 KiB owned-allocation cap,
16 KiB text buffers, parser stack 128 and evaluation depth 64. `system()` and
command pipes inside awk are explicitly unsupported. Shell pipelines around
awk use the normal sequential spools. Integer formatting has been corrected
and checked in the Plus guest; historical failure reports are retained.

### Next milestone

Prioritise a faithful upstream **nano** port before further serial setup.
Evaluate upstream tiny configuration, retain source/authorship/licence notices,
and port the real upstream ncurses dependency and the Macintosh screen/input and safe HFS file services it needs.
A host build or compile probe is not a working Macintosh editor. No nano binary
is included in this release. Serial, Telnet/SSH and a native compiler remain
later work; continue measuring baseline performance throughout.
