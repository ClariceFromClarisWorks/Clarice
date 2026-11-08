# Clarice

A native System 6 command prompt for a Macintosh Plus with 4 MB, built using the existing SkiFree Retro68 toolchain. This is a working Phase 1 prototype, not a Unix kernel, bash implementation or collection of completed GNU ports.

## Current build and priorities

Download the application disk or MacBinary package from [GitHub Releases](https://github.com/ClariceFromClarisWorks/Clarice/releases). The initial published build is a development snapshot with the limitations below, not a claim of POSIX conformance. No ROM, operating system or emulator is included.

Verified baseline: Snow emulating a Macintosh Plus with 4 MB, System 6.0.8 and MultiFinder. Physical hardware remains unverified. The application requests 384 KiB; that is its partition, not the entire machine's available RAM. The latest 1,000-entry rendered listing measured **11.70 seconds median**, down from 14.45 seconds. Creation date/time, paging and cancellation remain supported. Light/dark scrolling pixel comparisons and scrollbar staging checks passed. See [measurements and test limits](outputs/ls-opt2/TEST-REPORT.md).

| Area | Implemented | Still to do |
| --- | --- | --- |
| Terminal | Window resizing, solid block cursor, bounded scrollback, history, completion, saved preferences | Installed-font dropdown replacing the Font menu; broader font/geometry coverage |
| Native files | HFS access, `/mnt`, symlinks, editable `/etc/profile`, PATH | More external file utilities and filesystem error tests |
| External programs | Actual loadable `/bin/ls`, `/bin/df`, `/bin/cat` | Move remaining ordinary utility builtins out of the shell |
| Streams and shell | stdin/stdout/stderr, redirection, statuses, finite spool pipelines, tested sh subset | Reusable C stdio runtime, broader POSIX semantics; concurrent pipes are not implemented |
| Text tools | Local literal-only `grep -F` builtin | Faithful upstream grep, sed and awk ports, with source and licences |
| Connections/editors | No implementation yet | Serial client plus host test server, then networking and a faithful editor port |

Next priority is the C runtime needed by upstream text tools: buffered streams, regular expressions, bounded allocation ownership, exit cleanup and cooperative cancellation. Candidate source revisions have been recorded in [the research manifest](outputs/userland/candidates.json); they are **not linked or installed**. Candidate implementations include sbase and One True Awk, not completed GNU ports. Each port must pass upstream behaviour comparisons, repeated-run memory checks and Plus guest tests before it is advertised as working. Serial work follows the base tools. Native compilation on the Mac remains a separate stretch goal.

## Build

Install Docker and Python 3, then fetch the pinned image once:

```sh
docker pull --platform linux/amd64 ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
./scripts/build.sh
```

The project reuses SkiFree’s pinned toolchain; it does not require a SkiFree checkout. Builds are offline, target the original 68000 and produce `outputs/namespace/Clarice.bin` (MacBinary) and `Clarice.dsk` (800 KB floppy image). The application requests a 384 KiB partition under MultiFinder. The terminal uses installed Monaco 9 by default. File > Preferences opens a native dialog for installed font, size, monochrome colours, retained scrollback and rendering cache. Font/View menus and `prefs` also provide controls, including paging and window width. Settings are stored in the 52-byte `Clarice Preferences` file in Clarice Data. See `THIRD_PARTY.md` for the mrsh parser attribution.

## Use

Launch Clarice from Finder to install its data folders automatically, then use the shell. The application can remain outside the System Folder. Move/resize its normal document window. MultiFinder's upper-right application icon switches between Finder and Clarice while commands run.

Commands: `ln -s target link`, `readlink link`, `rm file`, `mkdir directory`, `help`, `pwd`, `cd`, `ls [-laP] [path]`, `cat file ...`, `grep -F literal file`, `echo`, `export NAME=value`, `history [off|on|clear]`, `clear`, `status`, `prefs`, `sh file [args]`, `. file`, `test`, `printf`, `true`, `false`, `exit`.

The scripting adapter implements a tested subset of POSIX sh: lists, `&&`/`||`, quoting and variable expansion, `if`, `for`, `while`/`until`, `case`, functions/return, positional parameters, `set --`, shift, command substitution, arithmetic, here-documents, input/output redirection and finite pipelines. Run `sh path/file.sh` or `./file.sh`; no executable permission bit is required. **This is not yet a complete POSIX sh implementation.** See the detailed limits below. Bash/zsh remain later targets.

`ls -l` displays kind, Locked/Invisible indicators, data-fork bytes, resource-fork bytes, creation date/time and name. `ls --full` adds raw flags and type/creator codes. These come from the same catalog lookup; resource forks are not opened. It does not invent Unix permissions, owners or link counts. The legacy bit sometimes called “bozo” is not presented as a modern permission or protection. No `chmod` permission emulation is provided.

Listings pause after a screen by default, stopping further enumeration until Space (next screen), Return (one more line), or Q (cancel). Clicking the status bar advances a page. `ls -P` disables paging. Redirected/scripted listings never page; View or `prefs paging off|screen|100` changes the interactive default. Creation date/time is implemented; full Unix `ls` option compatibility remains work.

`prefs window` opens the same Preferences dialog. `prefs scrollback 32|64|128` sets retention, and `prefs cache on|off` controls only the terminal rendering cache. No OS disk cache or RAM Doubler settings are changed.

Preferences examples: `prefs font Monaco`, `prefs size 9`, `prefs colours dark`, `prefs colours light`, `prefs columns 80`, `prefs fill` (toggle fill-screen/restore). `prefs fonts` lists installed fonts. Proportional fonts use fixed cells based on their maximum character width, reducing available columns and drawing speed. Monaco is the recommended baseline. Fill-screen retains the menu bar and is reversible without restarting the shell.

Completion currently matches the case of candidate names, although HFS opens are case-insensitive. Tab completes commands and HFS paths; another Tab shows up to 50 matching paths. Arrow keys or Control-P/N recall history; Control-A/E/B/F move the cursor; Control-W/U/K/Y edit; Control-R searches history. Option also acts as Control for keyboards lacking a Control key. Edit > Previous/Next command supplies a menu route on original Plus keyboards. Copy/Paste use Macintosh menus. Pasted line breaks become spaces and never execute automatically. The native vertical scrollbar or Shift-Up/Down scrolls bounded output history. New output keeps the viewed history anchored until those lines expire; typing returns to the live prompt. Clicking the running-status bar also cancels a command. Cancellation is available from File > Cancel command, Command-period, Control-C or Escape where the keyboard/emulator supplies those keys.

`/` is backed by `System Folder:Clarice Data:` on the running System volume. First launch creates missing directories and `/etc/profile`; subsequent launches preserve them. `/mnt/<volume label>` exposes Finder-mounted media. `pwd` and `$PWD` show the full physical shell path; the prompt shows only its final directory name, optionally prefixed by `$HOSTNAME` and a space. At root it shows `/ >`. No disk/network lookup occurs while drawing the prompt. Volume names retain Finder spelling; duplicate labels fail explicitly instead of choosing an arbitrary disk. `%2F` and `%25` encode literal slash and percent in HFS names. Native paths remain available as `native:Disk:Folder:File`.

`ln -s target link`, `readlink link`, `rm file`, and `mkdir directory` are supported. Links are HFS files with Clarice metadata and a bounded target; ordinary Mac applications do not follow them. Relative targets resolve from the link's parent. Resolution follows at most 16 links, supports dangling links, and rejects loops. `rm` removes a link itself and never recursively removes a directory. Hard links are unsupported. `ls -l` identifies links with `l` without opening each target. `test -L` tests the link itself; `test -e/-f/-d` follow it.

The built-in PATH is `/bin:/usr/bin`. `/etc/profile` is sourced once at startup and can replace or extend it, for example `export PATH="$PATH:/mnt/Tools/scripts"` and `export HOSTNAME=macplus`. Current lookup supports `.sh` files or files beginning with `#!/bin/sh` or `#!/usr/bin/sh`. Native CLX1 command images are also supported, as described below. Other interpreters and general Unix executable formats remain unsupported. Built-ins work independently of PATH. A slash in a command uses that path directly; otherwise PATH is searched in order. Empty PATH entries explicitly select the current directory. Tab completion includes PATH scripts. Environment values currently have a 127-byte limit.

History uses at most 128 fixed 1,028-byte records plus an 8-byte header (131,592 bytes on disk). A leading space excludes a command. History retrieval uses a 1 KiB buffer, not a growing RAM list. It flushes on clean exit; power loss can lose recent unflushed commands. Scrollback is a separate fixed 128×132 byte ring (16,896 bytes); retention can be limited to 32, 64 or 128 lines. Reducing retention discards older lines. The Plus default uses native QuickDraw pixel scrolling with the bitmap cache off, following measured comparisons. The optional system-font/bitmap cache allocates another 29,696 bytes and releases them when disabled. Long command lines stop at 1,023 bytes.

## Developer tests

`scrollbench native` compares adaptive burst redraws with a full repaint of the actual visible framebuffer and writes `Native Scroll Report`.

`scrollbench quick` is a shorter diagnostic; only the full run covers all glyph widths/heights.

`scrollbench` compares 90 full-width frames in each of three runs with native QuickDraw, cached C, and cached 68000 assembly rendering. It also compares assembly against C and incremental scrolling against full rendering, and writes `Scroll Report` in the current HFS folder. Use Monaco 9 and 80 columns for repeatable comparisons; the test replaces visible scrollback.

`selftest` checks assembly string/glyph results against C/library references and tests parsing/completion. `benchls path` runs three rendered directory listings and reports guest ticks; `benchscan path` excludes file text output to isolate catalog traversal. These timings are emulator observations, not physical Plus measurements.

`checks` / Tests > Run checks is for the disposable fixture only: it writes `Phase1 Report`, `Documents/result.txt`, and `Documents/fork-test`, and overwrites the bounded history with 140 test commands. Do not run it against personal files. Prepare fixtures with `scripts/fixtures.py` only with the project emulator stopped. `scripts/emulator.py` controls only the copied project Snow instance and disk. Original ROMs and disks are never modified or bundled.

See `outputs/scrolling/TEST-REPORT.md` for scrolling/preferences and `outputs/update1/TEST-REPORT.md` for the earlier scripting update and `outputs/phase1/TEST-REPORT.md` for the historical baseline. Architecture and later GNU tools/editor/compiler work are in `outputs/ARCHITECTURE.md`.

## Scripting limits in this development build

Thirty selected script fixtures match the host reference and the Plus guest transcript. They are regression coverage, not a POSIX conformance certification. The shell uses mrsh's parser with an original Mac execution adapter; it does not provide Unix processes.

Pipelines run sequentially through temporary native HFS files, with cleanup on normal completion/cancellation. Infinite producers, concurrent pipeline interactions, signals/traps, job control, process IDs, descriptors beyond 0–2, general Unix executable formats and complete standard utilities are not implemented. GNU grep/awk/sed and an upstream editor remain separate ports. `grep` still requires `-F` and is not GNU grep.

Other known gaps include shell options (`set -e/-u`), aliases, readonly/export attributes and exact environment inheritance, full IFS edge cases, embedded quoted `$@`, wildcard parent-directory components, locale/collation, all arithmetic assignment forms, all `test`/`printf`/`read` variants, compound-command/function redirection and full subshell positional-parameter isolation. Several unsupported forms report an explicit error; remaining semantic differences mean arbitrary portable scripts cannot yet be assumed to work unchanged. Pathname globbing is limited to the final component and at most 32 total arguments. A standalone quoted `"$@"` preserves separate/empty arguments.

Baseline bounds: 8 KiB source per invocation, conservative 256-token syntax guard, 16 execution levels, 8 nested source invocations, 32 arguments/4 KiB aggregate argument storage, 2,047-byte expansions/substitution output, 16 variables with 31-byte names and 127-byte values, and 16 functions. Parser memory checks may reject scripts below the text cap when insufficient heap remains. The stack reservation is now 64 KiB within the existing 384 KiB application partition. Forced allocation-failure and adversarial parser-depth testing remain incomplete.

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

Standard input, output and error support `<`, `>`, `>>`, `2>`, `2>>`, descriptor duplication such as `2>&1`, and closing with `>&-`/`<&-`. Redirections apply left to right and restore after each command. Finite pipelines remain sequential HFS spools, not concurrent Unix pipes. Pipeline status is the last command's status; a user interrupt stops the complete pipeline with 130. Missing commands return 127 and invalid native images return 126. Utilities report their own success/failure statuses.

`cat [-u] [--] [file ...]` reads stdin when no file is given or an operand is `-`. Terminal stdin uses a fixed 1 KiB canonical line queue: Return submits a line, Ctrl+D ends input on an empty line, and Ctrl+C cancels. On original Plus keyboards Option+D/Option+C are substitutes; the test emulator maps host Control to those guest keys. This is not a raw-terminal/termios implementation or a complete portable C stdio library. `read` remains limited to one variable and 127 characters.

The loader uses a fresh bounded allocation per invocation and frees it on return. CLX1 commands share Clarice's address space and must cooperate with cancellation; they are not protected processes. See `outputs/COMMAND-ABI.md` and `outputs/external-tools/TEST-REPORT.md`. Grep is still the earlier literal-only builtin; a faithful upstream port and awk/sed remain follow-up work.


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
python3 tests/commands/listing-output.py
```

Run the build first for the packer check. The last command checks saved guest evidence; it does not start an emulator. Emulator helpers expect a privately prepared local workspace and legally obtained ROM/System media. Host checks do not substitute for guest testing.

Original project briefs are preserved in `references/`; later decisions in this README and `AGENTS.md` supersede historical proposals. Third-party source and notices retain their own licences; see `THIRD_PARTY.md`. No blanket project licence is granted by making this development repository public.
