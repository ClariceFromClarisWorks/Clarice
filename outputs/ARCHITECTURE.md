# Clarice architecture and feasibility review

8 November 2026. Historical Phase 0 review with current decisions below.

Version correction: the 6.0.8 installation media and Finder identify the System file as 6.0.8 even though SysEnvirons returns 0607. Prior 6.0.7 labels based solely on that API are misleading. A fresh Plus/4 MB/MultiFinder test copy and evidence are recorded in outputs/system608/TEST-REPORT.md.

Current product name: **Clarice**. The shell root `/` is backed by **Clarice Data** inside the running System Folder. First-run installation creates missing folders and `/etc/profile`, preserving existing content. `/mnt/<Finder volume label>` exposes mounted volumes; `/dev` is reserved but has no device endpoints yet. Bounded symbolic links, script lookup through PATH, and a basename-only `hostname directory >` prompt are the current implementation work. Eject and formatting remain deferred. The hostname is an optional profile value; no MacTCP/Mooftcp hostname discovery is claimed. Older sections and reports retain their historical names and are superseded by this decision.


Emulator input update: the local Snow patch supplies Up/Down keypad packets, forwards macOS modifier changes, and aliases host Control to guest Option on non-ADB models. Command-period remains the original-keyboard interrupt and Command-C remains Copy. Evidence and limitations: outputs/emulator-input/TEST-REPORT.md.

Scrolling update (8 November 2026): native pixel reuse remains the Plus default after comparisons with an optional RAM bitmap/system-font cache. Refresh batching is reduced from 64 ticks to 3; this is a scheduling threshold, not a promised frame rate. Native scrollbars expose a fixed 16,896-byte ring with 32/64/128 retained-line settings. File > Preferences and equivalent commands persist installed font, size, monochrome theme, retention and bitmap-cache choice; the cache releases its 29,696-byte allocation when disabled. These preferences never change the OS disk cache. Richer process enumeration/accounting is deferred to supported System 7 Process Manager APIs. Details and measured outcomes belong in outputs/scrolling/TEST-REPORT.md.

Implementation update: the font/preferences, paged native-metadata listings and initial POSIX-language scripting update is described in [the update report](update1/TEST-REPORT.md). Full POSIX sh compatibility remains unachieved. A Phase 1 development prototype now runs in the Plus/4 MB/MultiFinder emulator. See [the current test report](phase1/TEST-REPORT.md) for achieved behaviour, timings and open validation gaps. The historical Phase 0 statements below describe the earlier review, not current execution status. The available test guest reports System 6.0.7 despite its recovery-image filename.

Latest scope decision (8 November 2026): **POSIX sh is the first scripting compatibility target; bash/zsh extensions or ports are later considerations.** The current prototype's limited command parser does not yet meet that target. Acceptance must cover quoting and expansion, positional parameters, conditionals, loops, functions, redirections, pipelines, command substitution, script invocation and exit status, with explicit evidence for unsupported runtime semantics. Familiar interactive editing does not establish language compatibility. Keep Mac file flags separate from POSIX permission semantics. The user also superseded the custom bundled font with an installed system monospaced default and persistent selection of installed fonts. These decisions override the historical staging and font proposals below.

Build one native Macintosh application with an assembly-first runtime, a C-compatible command interface and a bounded terminal model. Use built-in commands for the first proof, then load one native command module at a time. Keep files on HFS and networking behind the installed MacTCP interface. Reuse the existing SkiFree Retro68 toolchain.

## Product and compatibility contract

Sys6Unix is a native Macintosh command shell and terminal application with Unix-like behaviour, not a Unix/Linux kernel or a complete POSIX environment. Built-in commands may use C and measured 68000 assembly implementations; names alone do not imply upstream or full option compatibility. The current grep is a local subset, not GNU grep. POSIX sh language compatibility remains a tested development target, not an achieved compliance claim.

Faithful upstream utility ports (including grep and a small editor) are a later goal: preserve upstream source provenance, licences, authorship, reproducible builds and a documented adaptation patch set. Cross-compiling C ports using the existing toolchain is distinct from the later stretch goal of compiling C on the Macintosh itself. C source alone does not guarantee portability: required runtime APIs, memory, terminal features and cooperative scheduling must be supported. Native command loading and networking/SSH remain unimplemented; do not advertise them as current capabilities.

The cursor is a full-cell inverse block. Blinking must affect only that cell, with no prompt or text redraw; scrolling and idle text must avoid visible erase/redraw flicker.

## Current requirements

The baseline is a **Macintosh Plus with 4 MB physical RAM, System 6.0.8 and MultiFinder**. The SE is an additional 68000 compatibility target. Four megabytes is the whole machine's budget, shared with System, Finder, disk cache, drivers and other applications; it is not a proposed shell allocation.

The terminal is a normal movable, resizable Macintosh window. Finder, the menu bar, desktop and other applications remain accessible. Include a modern monospaced font, interactive editing, disk-backed history, command/path completion and bash/zsh-inspired shell behaviour. Single-Finder operation remains a fallback. Neither xterm's full escape-sequence set nor bash/zsh language compatibility is implied by the interface.

The assembly-first supplement supersedes the original preference for a mainly C implementation. Low-level subsystems may be wholly assembly when that serves measured speed and memory goals. C is the interoperability interface and reference-test language. Correctness, filesystem integrity and ABI fidelity remain mandatory.

Optional enhancements should activate automatically where CPU, OS capabilities and memory permit. The complete basic shell must work on the Plus/SE. Keep a user override for baseline mode. Do not install system extensions or change system cache settings automatically.

## Reused toolchain and verified result

The existing SkiFree image is installed locally and was successfully reused, without network access:

`ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c`

Run `./scripts/build-probe.sh` from the project root. It builds a deliberately small C/68000 assembly probe and packages a Macintosh application, MacBinary file and disk image. Compiler identification is GNU 16.1.0, using multiversal interfaces. Both languages receive `-m68000`; the probe uses single-segment linking. C compile checks establish 32-bit `int` and pointers for this toolchain. The assembly object's disassembly contains a stack argument load, increment and return.

Evidence is in `outputs/toolchain/`: build log, image identity, assembly disassembly, checksums and packages. The MacBinary package is 3,840 bytes; the generated disk image is 819,200 bytes. These are packaging sizes, **not resident-memory measurements**. The probe has not executed in a Macintosh guest or on hardware and is not a terminal implementation.

The SkiFree Snow harness is a useful existing automation pattern, but its documented configuration is an SE with 2 MiB RAM, an SE ROM, and a special startup application replacing Finder for that test. Reusing it unchanged would not validate the new Plus/MultiFinder requirement. The installed Snow bundle inspected here contains an SE ROM. A Plus configuration, a legitimately available Plus ROM and a copied MultiFinder system disk must be established before claiming baseline execution. Original ROMs, disks and SkiFree sources have not been modified.

The local Retro68 source inspected is recorded separately from the container image. Its commit is not asserted to be the image's source revision. Research checkout revisions are in `outputs/research-sources.json`.

## Architecture and alternatives

| Approach | Benefit | Cost and decision |
| --- | --- | --- |
| Built-in commands in one application | Smallest initial launch/dispatch path; one runtime; simple profiling | Code residency grows with commands. Use for Phase 1, with the future command context interface already defined. |
| One in-process native module at a time | Shared services; release command code/data after use; no process duplication | Requires relocation/startup/cleanup work and trusts command code. Recommended Phase 2 direction. |
| Reproduce the MPW tool ABI | Potential access to historical tools | Multiple historical ABI assumptions, A5 worlds, callbacks and exit handling. Investigate as an optional adapter; do not make it the core contract. |
| Adopt MacRelix wholesale | Existing process, VFS and executable machinery | Substantially broader semantics and dependencies than this brief needs; AGPL implications. Study specific designs, benchmark before any adoption. |
| Separate Macintosh application per command | OS-owned application lifetime | Launch overhead and partitions; poor sequential command/pipeline fit, particularly without MultiFinder. Reject for ordinary utilities. |
| Mac06 or MacMINIX as the base | Existing Unix-style functionality | Mac06 source is not publicly offered; MacMINIX brings a kernel/filesystem model outside the requested design. Reject as the foundation. |

This recommendation is about scope and avoided work, not a measured speed victory over existing environments. A native module is not a protected process: memory corruption can damage the shell or system. Cleanup can be guaranteed for normal return, controlled cancellation and runtime errors, not arbitrary bad assembly, stack corruption or direct `ExitToShell` calls.

## Source review and reusable components

**Retro68.** Inspected `libretro/syscalls.c`, `Samples/MPWTool/main.c` and its CMake configuration. The MPW example uses a special argument/callback structure, startup relocation and controlled exit. Its 68k build explicitly requires a single segment because Retro68 multi-segment trap patches conflict with MPW. This proves a relevant build route exists; it does not prove Sys6Unix can load arbitrary Retro68 executables. Default `_read_r`/`_write_r` call the File Manager without fully translating its errors; default `_exit` exits to Finder. They cannot simply be inherited as the shell runtime. Reuse the compiler, assembler, linker and resource packaging; replace or audit runtime hooks. The inspected syscall file carries GPLv3-or-later with the GCC Runtime Library Exception; sample code has its own notice. Check every linked component, including newlib and interface definitions, before distribution. [Retro68 source](https://github.com/autc04/Retro68)

**MPW.** The public `ksherlock/mpw` project is a host-side emulator, not a native Plus runtime. Its `toolbox/loader.cpp` handles CODE resources, jump tables, A5 and relocations. Use it as a format/behaviour reference and possible independent oracle. Its README describes GPLv2 licensing for the combined emulator and public-domain allocator provenance. Apple tools and libraries are separate assets, not automatically covered by that licence. [MPW emulator](https://github.com/ksherlock/mpw)

**MacRelix.** Inspected `lamp/Genie/Genie/code/{prepare_executable.cc,resource_handle.hh,loaded_handle.hh}`, `Kernel/execve.cc`, and process/vfork machinery. The 68k resource entry path accounts for address masking; executable lifetime and process machinery are distinct. This is valuable evidence against treating a resource pointer as an adequate complete loader. Most original repository code is AGPLv3-or-later, with listed Boost and other exceptions. Do not copy snippets as though the entire tree were permissive. Its resource ownership and context handling deserve comparison experiments; no footprint or System 6 compatibility was established here. [Repository and licence inventory](https://github.com/jjuran/metamage_1)

**Mac06.** The author's FAQ describes native HFS and MacTCP use, a separate kernel process and application-launch workarounds. It explicitly withholds source except by negotiated contract. Published tests concern later OS versions, so the project is not evidence of a verified 4 MB Plus/System 6 solution. Study the path/metadata lessons without adopting its process structure or claiming reusable source. [Author FAQ](https://www.dsitri.de/projects/mac06/FAQ.html)

**MacMINIX.** Inspected its archived command sources and Macintosh terminal shell launcher. `do_shell.c` depends on fork, signals, ioctl and exec; importing it would introduce the very process machinery excluded here. `cat.c` is a plausible small port exercise but has legacy C syntax, static state and exit handling to adapt. `grep.c` carries a public-domain notice but depends on a regexp implementation and getopt with separate provenance. The archive's root licence is BSD-style with three conditions; the archive maintainer disclaims completeness/authenticity. Audit selected files and dependencies individually. [MacMINIX archive](https://github.com/macminix/MacMinix)

**MoofTCP.** MIT-licensed source provides MacTCP-compatible parameter blocks, TCP status, ICMP echo and resolver facilities. Inspected `mac/include/mt_mactcp.h` and `mac/glue/mt_pb_{ip,tcp,udp}.c`. A concrete limitation: UDPStatus currently succeeds without filling a status layout. ICMP request submission is distinct from reply completion. Therefore successful initiation is not successful ping, and global connection enumeration must not be promised. Use its public interfaces as a client; do not embed its TCP/IP core. [MoofTCP source](https://github.com/matthewmrichter/mooftcp)

No upstream runtime code has been imported into the probe. Research clones are references, not shipping dependencies. GNU utility versions should be selected per utility after dependency and size tests; being old is not sufficient evidence of a small or suitable port.

## Platform constraints and consequences

| Constraint and evidence | Design consequence |
| --- | --- |
| Apple documents relocatable handles, locking and heap fragmentation. [Memory Manager](https://dev.os9.ca/techpubs/mac/Memory/Memory-11.html) | Never retain a dereferenced movable handle across memory-moving calls unless locked. Keep executing module storage locked; use compact arenas without replacing the OS heap. |
| Macintosh event handling cooperatively gives time to other applications. [Event Manager](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-38.html) | Yield during long commands as well as while the prompt is idle. Window visibility alone does not make Finder responsive. |
| HFS has separate fork/file operations and hierarchical identifiers. [File Manager](https://dev.os9.ca/techpubs/mac/Files/Files-72.html) | Use volume reference plus directory ID internally; distinguish data streams from whole Macintosh files. Audit individual routine availability against System 6. |
| Apple documents 31-character file/folder names, 27-character volume names and reserved colon. [Apple naming note](https://til-2001.mirror.kb1max.com/techinfo.nsf/artnum/n25010/index.html) | Reject overlong components; preserve spelling and defer equivalence to HFS. Do not silently truncate or replace names. |
| The PowerBook 180's supported systems start at 7.1 in Apple's specification. [Apple specifications](https://support.apple.com/en-mt/112147) | It is a secondary System 7/68030 target, not a System 6 target. |
| The selected Retro68 build uses 32-bit C ints and pointers; the classic Toolbox has separately defined types/calling conventions. Local build and headers. | Use explicit-width ABI fields; never infer Toolbox layouts from host C structs or assume Pascal and C calls are interchangeable. |

System 6 builds must avoid accidental System 7 dependencies such as assuming newer File Manager calls or Apple Events are present. Presence of a declaration in the cross-compiler's headers is not evidence of runtime availability. The baseline has no protected address-space boundary; 68000 word/long accesses need appropriate alignment. Review generated instructions and test on a true 68000 configuration, not merely a 68030 emulator running old system software.

## Terminal and interactive shell

Start with one normal document-style window, native title bar, drag/resize/close controls, menu handling, selection and clipboard operations. Open smaller than the desktop; choose rows/columns from the actual content rectangle and font metrics. A readable 80-column window may occupy almost the entire 512-pixel width, so do not force 80 columns at the expense of legibility or visible desktop. Resizing updates the grid and reported dimensions. Multiple tabs and windows are later options, not necessary for the requested terminal behaviour.

Keep text cells and a bounded scrollback ring, separate from command history. Draw dirty runs using monochrome QuickDraw within the window's clipping region; compare `ScrollRect` plus exposed-area redraw with a bitmap-backed path. Redraw correctly after another window covers and uncovers the terminal. Never accelerate by writing over the desktop or other applications. Deactivate the caret in the background and avoid idle busy loops. Assembly glyph composition can target an offscreen bitmap; clipped QuickDraw presentation retains Window Manager correctness.

**Font:** evaluate JetBrains Mono as a modern source, rasterised at build time into fixed-width monochrome strikes and packaged with the app. Its SIL OFL permits bundling and derivative formats subject to the font licence and naming terms. Carry the licence and use a distinct derivative name where required. No runtime TrueType engine or system font installation is required by this design. Compare approximately 6×10 and 7×12 cells at native 1-bit scale; accept only after inspecting `0/O`, `1/l/I`, punctuation, braces and extended Macintosh characters. Package suitable FONT/NFNT/FOND resources or an application-owned glyph atlas; verify the chosen resource path on System 6. JetBrains Mono is a candidate, not an already validated bitmap font. [Font licence](https://github.com/JetBrains/JetBrainsMono/blob/master/OFL.txt)

Phase 1 interaction includes insert/delete, cursor motion, start/end-of-line, word erase, kill/yank, previous/next history, bounded reverse search, and command/path Tab completion. The Plus keyboard does not require dedicated arrow keys: provide Control-P/N/B/F/A/E equivalents as well as keys present on other keyboards. Command-period cancels; Control-C can also cancel in the input path. Native Command-C/V remain copy/paste. Do not execute pasted multiple lines without an explicit submission action in the initial shell.

Completion uses the same quoting/token rules as execution. Complete command names from built-ins and configured command directories, paths from HFS, append a slash for directories, and escape spaces and shell metacharacters. First Tab inserts the longest common prefix; repeated Tab lists or cycles bounded results. Enumerate large directories incrementally and allow cancellation. Do not load a whole directory into RAM merely to complete a filename.

Define single/double quotes, backslash escaping, `$NAME`, `${NAME}`, `$?`, `~`, `cd`, `pwd`, `export`, and basic `>`/`>>` redirection. Preserve empty quoted arguments and report unterminated quoting without executing. Basic globbing, pipes, scripts and conditionals follow in Phase 3; unsupported syntax must be rejected explicitly. Full bash/zsh job control, arrays, command substitution and programmable completion are not assumed. Porting bash/zsh wholesale is an experiment, not a prerequisite for their familiar interactive behaviour.

**History:** proposed default 256 KiB disk cap, 4 KiB recent-entry cache, at most 2 KiB pending writes and 1 KiB retrieval scratch, with a 1 KiB maximum command line initially. Store completed commands with lengths so a bounded backward reader can find previous entries without loading an index for the entire file. Rotate a small fixed number of segments under the total disk cap; never compact the whole history on each command. Flush on a small idle batch, orderly exit or explicit request, not each keystroke. Power loss can lose the unflushed tail; recovery ignores incomplete records. On full/read-only media, report the problem and continue with a bounded in-memory ring. Reverse search reads chunks and yields; its latency grows with disk history, but memory does not. Provide history off/clear controls and an option to omit commands prefixed by a space. Persistent history and visual scrollback have independent caps.

## Native files and internal storage

Native Macintosh filesystem access is paramount. The user's latest direction permits a private internal store for application-owned assets if measurements justify it; the original prohibition on a mandatory separate user filesystem still applies. Users' files remain accessible directly on HFS. An indexed internal store is packaging, not a substitute for that access.

Compare three implementations with identical commands/fonts: separate HFS files; Resource Manager resources in the application or a companion file; and a read-only indexed bundle with contiguous records and offset/length entries. A bundle might reduce catalog lookups and open/close overhead. It might also increase read amplification, update cost and resident indexes. Do not assume compressed storage is faster on an 8 MHz 68000. Start uncompressed and measure compression separately only if storage transfer is the bottleneck.

The recommended initial candidate is a hybrid: built-in core commands, bundled font and small immutable assets, ordinary HFS files for user data, scripts and bounded history, and loadable external commands later. Keep bundle indexes bounded, validate all offsets/lengths and version the format. Load only the needed asset, not the entire bundle. Avoid writing history or preferences into the executable's resource fork. Compare cold/warm loading, repeated invocation, random command access, code residency, fragmentation and bundle replacement. Select the winner after measurements; no mini filesystem implementation is justified yet.

Represent a working directory by volume reference and directory ID. Define `/Volumes/<name>/...` as the initial mounted-volume namespace; relative paths use the shell's own working directory. Handle duplicate volume names explicitly rather than selecting one silently. Reserve an explicit native-path form rather than guessing whether a colon means HFS syntax. Use a reversible escape convention for literal slash, percent and special dot-only HFS names; test round trips before freezing it. For the first implementation, unrepresentable names must produce an explicit error rather than point to another file.

Byte-stream `open/read/write` addresses the data fork. Do not automatically convert newline bytes based on Finder type. Terminal display can interpret CR and LF; text utilities need documented Macintosh CR/LF handling without changing binary files. `cat` and redirection remain byte-preserving. Whole-file `cp` must eventually preserve both forks and Finder metadata; a data-only copy must be explicitly selected. Redirection to an existing file truncates only its data fork and preserves its resource fork and metadata. Newly created text output gets documented type/creator values. Test disk-full partial writes and report them; never report a failed copy as success.

Do not promise POSIX atomic replace semantics using delete-then-rename. Cross-volume moves require both-fork copying and validation before source removal. HFS case behaviour, Unix permissions, links and unlink-while-open semantics must be documented as limited or unsupported, not emulated with misleading success codes. Resource edits require a backup of the original, validated staged changes and a recoverable commit process; “atomic” must be proven for the actual operation and filesystem.

## Command ABI and lifecycle

Phase 1 commands use a shared context and ordinary call/return. Proposed entry contract: a versioned, fixed-width context carrying argc/argv, descriptor mappings, environment, working directory, error state, cancellation flag, allocation ownership and a service table. Assembly and C utilities call the same services. Keep an ABI specification for field offsets, alignment, argument passing, result values, saved registers and callback conventions; test it with independently compiled callers/callees.

Phase 2 should first try a single native code resource with a bounded manifest, zero-initialised per-invocation data and explicit runtime imports. Compare position-independent assembly using context-relative state against compiler-generated code with validated relocations. Arbitrary C globals, function pointers and static initialisers make “just copy the bytes and call them” invalid. Do not assume an APPL CODE resource is an independently loadable module. Use the established Retro68 single-segment route as an experiment, not a final ABI commitment.

Before entry, validate version, CPU minimum, resource lengths, relocation bounds/alignment, required services and memory limits. Lock code for execution, establish data context and preserve shell/Toolbox state. Resolve imports once. On controlled exit, flush owned streams, close owned files, restore the graphics port/resource-file context and globals, free command allocations, unlock/unload code and return status. Command `exit()` returns to the shell boundary, not Finder. Reset static state between invocations. Reject background callbacks that outlive an unload; later networking modules must cancel and drain outstanding operations before release.

A controlled longjmp-based exit may be suitable for C commands if all cleanup is centrally tracked; it is not an exception mechanism for arbitrary machine faults or C++ destructors. Initially reject modules requiring unsupported process services. Keep code caching off until repeated-load measurements justify a small purgeable cache.

Sequential pipelines should spool to bounded buffers that spill to HFS temporary files, keeping only one command resident. They cannot reproduce concurrent streaming semantics: an infinite producer piped into `head` would not terminate as in Unix. Document this and reject unsupported workloads where identifiable. Cooperative streaming is a later, separately measured design.

## Minimal compatibility surface

| Stage | Interfaces | Required semantics |
| --- | --- | --- |
| Phase 1 | open, close, read, write, lseek; errno; descriptors 0/1/2 | Access flags, EOF, partial reads/writes, errors, data-fork offsets; bounded descriptor table. |
| Phase 1 | stat/fstat subset, opendir/readdir/closedir, getcwd/chdir | File kind/size and available dates; HFS directory enumeration; no fabricated Unix ownership. |
| Phase 1 | malloc/calloc/realloc/free, getenv/setenv, string/memory helpers, exit boundary | Allocation failure and overflow; per-command ownership; bounded environment. |
| Phase 2 | stdio subset, dup/dup2 if selected ports need them, time conversion, rename/remove/mkdir/rmdir | Audit newlib hooks and reentrancy; shared open-file offsets where required; accurate failure semantics. |
| Phase 3 | pipe-like shell plumbing, scripts and resource/metadata utilities | Explicit sequential-pipeline limits; both-fork safety. |
| Phase 4 | selected socket/connect/listen/accept/send/recv/close and resolver calls | Map to public MacTCP operations; cancellation, callback lifetime and yielding. |
| Unsupported initially | fork, full exec/process replacement, signals, mmap, pthreads, symlinks, Unix permission enforcement | Fail explicitly; no claim of POSIX compliance. |

Keep a per-function implemented/partial/unsupported table as code lands. Optional interfaces should be driven by actual utility dependencies, not a speculative broad libc rewrite. Selective newlib reuse may save effort, but measure its linked code, per-stream buffers and retained allocations. Write assembly entry points directly where practical; do not add C wrappers solely to connect two assembly components.

## Memory budget and measurement

The original 80–168 KiB target is an aspiration, not a measured total. Start development with a 384 KiB preferred MultiFinder partition and test a 256 KiB minimum only after measured peaks fit. These are proposed experiment settings, not established requirements. Four megabytes installed does not guarantee a contiguous partition of that size while other applications run.

| Component | Initial planning range in KiB |
| --- | ---: |
| Resident shell/runtime code and static data | 64–128 |
| Stack and guarded working margin | 16–32 |
| Terminal cells, capped scrollback, glyphs and drawing scratch | 16–40 |
| History cache/pending writes/retrieval scratch | 7 |
| Parser, completion, descriptors, environment and path state | 8–16 |
| One small command's code/data and I/O buffers | 16–48 |
| Window/resource bookkeeping and fragmentation margin | 24–48 |
| **Planning envelope** | **151–319** |

These are allocation allowances to test, not sums inferred from binary size. Avoid double-counting built-in commands as both core and dynamically loaded code. System heap growth, Font Manager caches, disk cache and network-driver memory must be reported separately. Do not advertise a minimum partition until representative commands, resizing, history search and failure paths have run within it.

Record heap free space and largest contiguous block before startup, at first prompt, after each command, after cleanup and after hundreds of repetitions. Add tracked allocation high-water marks and stack canaries; handle locked/purgeable code explicitly. Measure system-heap changes with and without networking/fonts and compare the whole-machine state under MultiFinder. Report partition reservation separately from touched/resident allocations. Test forced allocation failures rather than relying only on generous partitions.

## RAM Doubler, OS caching and capability scaling

RAM Doubler is not a stock Plus/SE baseline option: archived requirements list 68030/68040/PowerPC-class hardware. Its exact supported OS and hardware depend on the release. Obtain the intended release's original requirements before testing it on the PowerBook; do not infer all versions from one product description. More effective memory is not inherently faster execution. [Archived RAM Doubler requirements](https://hub.catalogit.app/australian-computer-museum-society-%28acms%29/folder/entry/connectix-ram-doubler)

Test the System 6 RAM/disk cache using the settings actually exposed by the installed control panel. Candidate experiments are the smallest supported setting, 32, 64, 128 and 256 KiB where supported, with the same app partition, disk image and storage hardware. Restart between configurations as required and record the effective setting. Compare first-run loads after restart, repeated command loads, directory completion, history reverse search, sequential reads/writes and scripted mixed use. A larger cache can help metadata/repeated reads but displaces memory available to Finder and applications. Excessive application buffering can duplicate OS caching. Select the best balanced setting from results; do not prescribe a universal cache size yet.

On the PowerBook 180, separately test ordinary physical-memory operation and an explicitly compatible RAM Doubler release, under identical light and memory-pressure workloads. Record median and tail response times, task-switch latency, disk activity and memory availability. Do not install RAM Doubler, enable virtual memory or change cache controls automatically. Its page-management/compression behaviour is OS software configuration, not something CPU dispatch inside Sys6Unix can turn on safely.

Use a small startup capability record. On older systems use documented available system/CPU queries, with trap-availability checks before newer queries such as Gestalt. Detect OS facilities, processor class, installed networking and available memory independently. Dispatch once per subsystem, avoiding per-byte feature branches.

| Baseline on Plus/SE | Automatically eligible enhancement |
| --- | --- |
| 68000 parsing, scanning, memory movement and glyph routines | Separately compiled 68020/68030 implementations only after CPU checks and a measured advantage. |
| Small fixed working buffers, bounded history and scrollback | Larger bounded buffers/caches when available memory and benchmark policy justify them. |
| Native monochrome clipped window | Colour treatment where Color QuickDraw exists; core layout remains functional without it. |
| Core offline shell | Networking commands when public driver/resolver capabilities are available, regardless of CPU generation. |
| Supported filesystem API subset | Newer OS operations through separate adapters when detected, without weakening baseline semantics. |

A 68030 does not itself imply a particular OS API, network driver or surplus RAM. Keep unsupported opcodes out of startup and baseline paths, including libraries. Provide `auto`, `baseline` and diagnostic capability reporting; a forced fast mode must still reject incompatible hardware. A 68030 module experiment must also validate code-cache coherence before calling newly loaded/generated code. Optional modules can be loaded on demand so the Plus does not retain their code.

## Stretch goal: a native GCC build chain

Provide a GCC-based development toolchain that runs inside Sys6Unix on the Macintosh itself. The existing modern Retro68 cross-build environment remains the bootstrap and development route; it does not by itself satisfy this goal.

Success means editing a small C program on native HFS, invoking `gcc` from the terminal, preprocessing, compiling, assembling and linking it, then executing the resulting Sys6Unix command. Supply compatible headers, runtime libraries and startup code. Add native Macintosh application/resource packaging and a small `make` workflow as subsequent milestones. Compiler self-hosting and C++ support are separate later experiments, not requirements for the first working chain.

Investigate historically appropriate GCC versions and matching assembler/linker formats, along with any existing Macintosh ports. Audit their licences, target ABI, object format, runtime dependencies and required source changes before choosing a version. The compiler must emit code compatible with the selected command loader; producing a generic 68k Unix executable is insufficient.

For the 4 MB Plus/SE, test small translation units, sequential compiler stages, HFS temporary files and bounded driver state. Measure each stage's peak memory, largest allocation, compile time, temporary disk use and MultiFinder responsiveness. File-backed intermediate results do not guarantee that an individual compiler or linker stage will fit. Replace process-spawning assumptions with the shell's supported sequential execution model where feasible, and account for globals and allocations across invocations.

Offer larger compiler versions, optimisation workloads or language support on later machines only when their actual CPU, OS and memory requirements are met. Keep the core shell independent of all compiler components. If a useful native GCC cannot fit the Plus, document that measured limit and retain cross-compilation for it while offering the native chain on qualified later hardware. Do not promise a current GCC release or a self-hosting build within 4 MB without evidence.

This belongs after the core shell, HFS compatibility and command loading are proven. First feasibility gate: one end-to-end C program built and run on the target, with recorded memory and timing results.

## Benchmarks and validation gates

Use identical fixtures for assembly and reference implementations: empty/short/large inputs, odd addresses, boundary lengths, overlap for memmove, malformed tokens, quoted paths, CR/LF/binary data, and allocation failures. Compare exact output and exit status before timing. Test preserved registers and stack balance independently. A faster incorrect result is a failed test.

Collect startup-to-prompt, key-to-visible-character latency, completion latency for 10/100/1000-entry directories, history search latency at each disk cap, command dispatch/load/cleanup, directory traversal, sequential I/O, rendering/scrolling, pipe spool overhead and peak memory. For the terminal, include cover/uncover, resizing, selection, background output and switching to Finder while commands run. Use guest TickCount batches long enough to exceed coarse timer resolution; keep logging outside the measured block. Report repetitions, median, range/tails, timer source, bytes processed, code size and peak allocation. Treat single-operation sub-tick timings as unresolved, not zero-cost.

Separate emulator correctness/relative comparisons from physical Plus results. Record CPU, RAM, ROM, System/MultiFinder versions, driver versions, disk/cache settings, emulator timing configuration and source/build identities. Re-run on the physical Plus with 4 MB, then SE and PowerBook; do not scale an SE result by clock speed and call it a Plus result. Track regression history in machine-readable records. Provisional acceptance: bounded history memory, no accumulation after repeated commands, correct clipping, responsive Finder, and filesystem fixtures unchanged except for intended writes. Quantitative latency gates should be set after the first honest baseline.

## Concrete Phase 1 plan and risks

1. **Establish the Plus test fixture.** Prepare a disposable copy of System 6.0.8 with MultiFinder and 4 MB configuration. Launch normally from Finder, retain its desktop, and verify clean exit. Reuse SkiFree automation ideas but remove its Finder-replacement assumptions. Gate: observed launch/quit and foreground/background switching on Plus configuration.
2. **Build the window and font proof.** Render fixed-width text, resize, obscure/uncover and select/copy/paste. Package bitmap font candidates and inspect at native resolution. Gate: readable glyphs, correct clipping and measured font/window memory. No full-screen rendering shortcut.
3. **Implement bounded input and history.** Establish shared parser/completion rules, editing shortcuts, capped disk history and bounded completion scans. Gate: quoted filenames round-trip; long history and large directories do not grow memory without limit; reverse search remains cancellable.
4. **Implement native HFS descriptors and error mapping.** Test named volumes, nested directories, read-only/full disks, partial I/O and both-fork fixtures. Gate: no metadata/resource-fork loss from data-fork redirection; errors distinguish EOF from failure.
5. **Demonstrate commands.** Add `cd`, `pwd`, `ls`, `cat` and a clearly documented initial `grep` subset, variables/status and redirection. If initial grep is literal-only, label it explicitly and reject unsupported regex syntax/options; full regex support follows the chosen library audit. Gate: repeatable command transcripts and exact file outputs, cancellation/yielding and successful Finder return.
6. **Measure and choose.** Compare assembly/reference scanning and terminal paths, internal asset packaging alternatives and OS cache configurations. Establish actual memory/latency baselines and revise partition sizes. Gate: report correctness, speed, size and memory together.

Phase 2 then ports an existing small utility with a documented patch set and proves a module containing globals, static initialisers, function pointers and runtime calls can load/unload repeatedly. Keep Phase 1 built-ins if module experiments fail; do not hold the first terminal hostage to an unproven loader.

| Main risk | Resolving experiment |
| --- | --- |
| Incompatible startup/A5/relocations | Cross-language ABI harness; load at different addresses; callback and register tests; reject malformed modules. |
| CPU-bound utilities starve MultiFinder | Long scanning/regex fixtures with explicit bounded polling; measure Finder switching and cancellation latency. I/O hooks alone do not make pure computation cooperative. |
| Heap fragmentation and stale state | Hundreds of load/run/unload cycles and forced failures; compare largest blocks, ownership counts and static-state reset. |
| Tiny-font illegibility | Native 1-bit specimen and real-screen review before locking the font/grid size. |
| History or completion causes hidden RAM/disk costs | Maximum-length records, interrupted writes, full disks, large directories and capped-cache assertions. |
| HFS path/metadata mismatch | Reversible naming fixtures, collision tests and both-fork hashes before/after every operation. |
| Internal bundle slower than loose HFS files | Same asset contents/layout variants, cold/warm loads, memory and update-cost measurements. |
| Networking lifetime bugs | Public API probes, delayed callbacks, cancellation and unload tests; optional client module only. |
| Faster CPU path breaks baseline | Separate 68000 and accelerated builds, opcode inspection and forced-baseline runs on every supported machine. |

Phase 0 has established the reusable build path and a source-grounded architecture. Runtime sizes, font quality, Plus guest execution, loading performance, cache recommendations and hardware benchmarks remain experiments, not achieved results.

## November 2026 scrolling correction

Native output now reuses the existing 4 KB row buffer for changed spans and copies overlapping scroll areas without first erasing them. Aligned, fully visible monochrome rows use the existing reference-tested 68000 copy routine; clipped or misaligned destinations use QuickDraw. No scrollback or heap allocation increase is introduced. Native remains the measured default: 526 median guest ticks for 90 frames versus cached assembly 567 and cached C 693. The prior native median was 507; the 3.7% cost is accepted to eliminate erase-before-draw flashing. Native/cached burst and reverse-view pixel comparisons pass, including native inverse colours. Shift-arrow normalization is Plus-specific. Live scrollbar tracking is implemented and manual thumb dragging is confirmed by the user; automated drag verification remains limited by input delivery. See outputs/scroll-fix/TEST-REPORT.md.

Preferences verification: the native dialog and equivalent commands share installed-font lookup and strict numeric validation. Font ID zero (Chicago) loads correctly; a missing saved font retains the default while other valid settings load. The compact 48-byte S6P3 format is unchanged. Plus/6.0.8 restart and cache-release results are recorded in outputs/preferences/TEST-REPORT.md.

## November 2026 file tools

`df` uses mounted HFS volume allocation counts, with widened products for allocation-block sizes. It reports allocation space rather than complete physical media capacity. `ls` streams catalog order through one 132-byte row buffer, uses the current window width, and retains one-name-per-line output for scripts/redirection. Dot entries are synthetic namespace-aware entries selected by `-a`. Mac flags and both fork sizes come from the catalog record already fetched. The shared integer-only size formatter supports bytes through EiB without floating point.

Long listings avoid general printf parsing per row. Listing-only bulk paints are coalesced at 32 ticks; event polling remains at two ticks, with first-entry/first-row and pager presentation immediate. The ordinary painting cadence is restored on success, error, or cancellation. No OS disk-cache changes are made. Status reports free application heap, explicitly distinguished from physical RAM.

## External utility direction (latest user requirement)

Ordinary utilities must become actual independently loadable programs under `/bin` or `/usr/bin`, using PATH. Shell-state builtins such as `cd` stay in the shell. Wrapper files that merely dispatch built-in implementations do not meet this requirement. A versioned, bounded native module interface must expose stdin/stdout/stderr, file access, allocation, cooperative cancellation and exit statuses. Programs must release resources on normal exit, errors and cancellation.

Grep is to be a faithful, updateable upstream source port with original authorship/licences and a separate adaptation patch. Investigate a current maintained implementation before choosing a baseline version. Awk and sed follow. Preserve the distinction between sequential spool pipelines currently implemented and concurrent Unix pipes; neither the CLI nor its compatibility layer may advertise unsupported process semantics. Test redirection, descriptor duplication, EOF, pipeline status and interrupt handling at the interface boundary.

## Listing and scrollbar follow-up

Printable terminal output now copies bounded runs, with an executable byte-at-a-time reference retained in tests. A complete long-listing row is emitted in one call. The 32-tick bulk paint threshold and two-tick event polling are unchanged. Final thousand-entry timing is recorded in outputs/ls-fast.

Scrollbar bounds are excluded from background erasure. Control Manager setters and complete control drawing run against a fixed 1,368-byte monochrome scratch bitmap with word-aligned bounds, followed by one clipped CopyBits transfer. No on-screen HideControl/erase phase is used. A staging readback test checks that text rendering and control composition leave existing scrollbar screen pixels untouched until presentation.

## Window placement and persistence

The initial outer window frame is inset two pixels from the left edge and two pixels below the menu bar. User moves and resizes persist content origin and dimensions in the 52-byte S6P4 preference file; S6P2/S6P3 preferences remain readable. Restored frame positions are constrained to the current screen. Text left inset is fixed at four pixels regardless of font.

Cursor update: the block cursor is solid. The periodic blink timer is removed; cursor visibility still follows foreground focus, command execution and repainting.

## Native /bin utility milestone

The ls/df/cat implementations and listing benchmarks compile separately into a 68000 CLX1 multicall image. The application resource is installer payload only: invocation resolves PATH and loads the corresponding native HFS /bin file. The module uses explicit shared services for filesystem resolution, rendering, standard streams and cancellation; its actual listing/copy/volume algorithms are not shell dispatch wrappers. Its allocation is freed after each call.

Three standard descriptors now have independent bindings; redirections and duplication are applied in order, and bindings are restored after every simple command. File references shared by duplication share their offset. Closed output fails. Standard error is separate from pipelines. Terminal input is canonical and bounded to a 1 KiB queue. Sequential HFS spool pipelines retain bounded RAM, EOF, cleanup and final-stage statuses, but remain nonconcurrent. Full libc stdio, raw terminals, process isolation and signal delivery are not implemented.

## Creation-first long listing

Latest preference: creation time is paramount. Default ls -l now shows kind/Locked/Invisible, data/resource sizes, creation date/time and name. Raw hex, N/B bits and type/creator move to --full. Inapplicable directory fork sizes and absent virtual timestamps are blank. The catalog record supplies all fields; no additional per-file I/O is introduced. -a includes both dot names and Finder-invisible files/folders; explicit operands remain visible. Date formatting covers classic unsigned 1904–2040 timestamps and uses no locale allocations.

Date formatting uses four bounded 68000 DIVU operations, with the quotient proven to fit 16 bits; a portable C branch and Python datetime are executable references. The actual compiled 68000 routine is checked over every day boundary plus seeded random timestamps. No timestamp cache or heap allocation is used.

## 9 November listing and native-rendering optimization

Long-listing plain fork lengths now use bounded 68000 DIVU decimal conversion, verified against independent integer output across small and random 32-bit values. Human-readable sizes retain the general formatter. Unrequested type/creator formatting is skipped. Native row rendering only rounds changed spans for alignment when the window origin permits the direct-copy path; clipped/misaligned QuickDraw transfers use the actual changed span. Row-port font setup occurs once per paint, and character preparation covers only that span. Offscreen row composition, scrollbar staging, first-entry presentation, paging and two-tick event polling are retained. No extra heap or scrollback allocation is introduced. Timings and pixel verification belong in outputs/ls-opt2.

## Queued user requirements, 9 November

Lock in the tested performance work before advancing. Next UI change: remove the separate Font menu; Preferences should offer all installed fonts in a dropdown, retaining Monaco as the baseline. Use a System 6-compatible user item with PopUpMenuSelect rather than the System 7 popup control. Fill-screen/restore already exists and should gain an obvious reversible shortcut, keeping MultiFinder/menu access. These UI changes are queued, not yet implemented.

After the base Unix utilities, implement a separate serial terminal client over the installed Mac serial driver, with an emulator-compatible test server on the host. Serial is not Telnet; a TCP-backed emulator serial cable must be distinguished from Telnet negotiation and from SSH. Scope includes serial parameters, terminal emulation, echo/newlines, break, logging, disconnect, and explicitly verified flow control/file transfer. No server or serial client has been created yet.

Font/encoding note: ASCII is the safe initial interchange set. Apple documents that older ROM bitmap Monaco/Chicago/Geneva/New York do not even cover the full Mac Roman range (only through 0xD8). Future remote-terminal support needs encoding translation, explicit missing-glyph handling, VT/ANSI state and line-drawing glyphs; selecting a font alone cannot deliver Unicode/UTF-8 or SSH terminal compatibility. Reference: https://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/ROMAN.TXT

## Initial publication scope

The initial GitHub snapshot publishes the tested external ls/df/cat build and its evidence. Grep remains a literal-only builtin; sed and awk are not installed. Source candidates recorded in outputs/userland/candidates.json remain research, outside the compiled source tree. A reusable buffered C runtime with bounded ownership and cancellation is required before faithful upstream utility ports can be delivered. Finite sequential HFS spool pipelines must not be described as concurrent Unix pipes or complete POSIX compliance. The README contains the current implementation/roadmap matrix and supersedes historical capability descriptions above.

## Source-port runtime and activity indicator

The grep/sed guest tests and activity checks are recorded in outputs/grep. Grep and sed are independently loaded CLX1 programs built from pinned sbase source with musl regex. The adapter owns each invocation's allocations, limits their total to 96 KiB including tracking headers and stream buffers, and releases them on normal exit, errors and cancellation. This ceiling is not a reservation; available contiguous application memory can impose a lower practical limit. Images, loader metadata and newlib's fixed stream-record pool are separate costs. Loader metadata is allocated separately and released before execution. The Plus application partition remains 384 KiB.

Short-lived allocation reuse is capped at 8 KiB within the same 96 KiB ownership ceiling; cached blocks are discarded at invocation exit or under allocation pressure. Each stream has a 512-byte buffer; there are at most twelve open streams including standard streams. Input lines are limited to 16 KiB, and allocation failure is an error. Native files use their data forks without CR/LF conversion. stdout is line-buffered for terminal output and buffered for redirected output; stderr is unbuffered. File descriptors, EOF, buffering and exit cleanup are real adapter services, not calls back into the earlier grep builtin. Finite pipelines remain sequential HFS spools.

A passive activity overlay occupies the top-right text cell, never the scrollback model or output stream. It appears after 300 ticks without terminal output and cycles clockwise through vertical, slash, horizontal and backslash at 30-tick intervals. Output, return or cancellation removes it and restores underlying pixels. Canonical input, paging and interactive screen ownership suppress it. Future full-screen tool adapters must explicitly claim ownership with app_activity_interactive; mere long-running loops do not identify an editor.

Larger tools may justify a separate MultiFinder worker application with its own partition. Current /bin programs execute within Clarice's partition. System 6 temporary-memory handles cannot be treated like System 7 handles across event processing; do not adopt that as an untested workaround.

## Awk port and initial serial milestone, 9 November 2026

The pinned One True Awk source is a separately loaded /bin image, retaining its own parser, evaluator and regex engine. The shared ownership ceiling remains96 KiB, with dynamic small initial text/regex buffers, a16 KiB text-buffer limit, parser stack128 entries and evaluation depth64. Mathematical routines use software floating point linked into the disposable command image. Clarice's SIZE partition is being tested at768 KiB on the4 MB Plus; the extra image and working memory cannot be promised inside the earlier384 KiB partition. system() and command pipes inside awk fail explicitly; ordinary shell pipelines around awk use the existing sequential HFS spool. Host comparisons are necessary but guest validation remains the release gate.

Initial serial access uses the installed System6 Serial Driver, modem port A,8N1 at selected standard rates, with a fixed1 KiB receive buffer and bounded transfers. It must restore driver buffers and close references on success, error, timeout and cancellation. No replacement TCP stack or direct SCC programming is needed. A separate /bin/serial program may use an explicit bounded keyboard queue for a simple text session; this is not yet VT/ANSI emulation, Telnet, SSH, file transfer or a general /dev device layer.

The host test uses Snow's local PTY serial bridge, whose peer is a bounded Python responder. It exposes no network listener and executes no host shell commands. A fixed ping/pong probe verifies guest-to-host and host-to-guest bytes; a short text session checks interactive input and clean disconnect. Physical cabling, handshaking and real hardware remain unverified. Printer/AppleTalk ownership is outside this initial modem-only scope.

## Preferences and utility priority, 9 November 2026

Preferences now use a bounded, non-executable text configuration at
`/etc/clarice/clarice.conf`; history retains its capped binary ring at
`/etc/clarice/clarice.history`. Existing root-level Clarice Preferences/History
are migrated without replacing an existing destination. The native dialog
exposes every `prefs` setting, including installed fonts via a popup list,
colours, size, columns, paging, retention, renderer/cache and reversible fill.
The top-level Font menu is removed. Guest verification is recorded separately
in outputs/preferences-text; implementation is not itself execution evidence.

After preferences, the agreed order is:
1. Everyday pipelines: wc, head, tail, cut, tr, sort, uniq, tee.
2. File operations: cp, mv, touch, rmdir; move mkdir, rm, ln and readlink to /bin.
3. Inspection: find, du, file, od, basename, dirname.
4. Consistent --help, help for builtins, and standalone more.
5. Serial client, then a faithful small vi or nano port; Telnet/SSH separately.

Use sbase where suitable, preserving authors, per-file notices, licence and
pinned provenance. Get correct bounded implementations working first; measure
and optimise each subsequently. Sort must spill to disk when necessary, and
tail -f must remain cancellable. Ownership/process tools require meaningful
Macintosh behaviour rather than invented Unix semantics. Previously started
serial work remains unfinished and is deferred behind these utility groups.

Private temporary files and diagnostics belong under /tmp, never loose in the
Clarice root. Pipeline/substitution files are deleted on completion and normal
cancellation. Do not wipe arbitrary /tmp contents on exit: user scripts may
have deliberately saved files there. Crash-orphan recovery remains future work.
Settings/history remain in /etc/clarice; future persistent logs should use
/var/log/clarice. Known old root-level diagnostic reports migrate to /tmp with
legacy-prefixed names, preserving content and refusing destination collisions.

## Everyday pipeline ports (9 November 2026)

`wc`, `head`, `cut`, `tr` and `uniq` are separate CLX1 programs installed in
`/bin` and resolved by PATH. Their pinned sbase source files remain pristine;
`prepare-pipelines.py` adds explicit help and a cooperative poll in `tr`.
The common runtime redirects `putchar` as well as formatted output to the
active command's stdout. This is required for newlines/separators to follow
redirections and pipeline spools correctly. The shell exports `memcmp`,
`strtoul` and `strtoll` for these ports. No CPU beyond the 68000 is required.

Retain 96 KiB invocation allocation and 16 KiB line bounds. `wc` and `tr`
stream input without retaining complete lines. Unicode classification is
upstream UTF-8; font encoding/serial terminal emulation is a separate concern.
Do not claim GNU option parity or concurrent pipelines. Disk-spilling `sort`,
`tail` with cooperative follow, and `tee` remain the next work.


## Current utility completion work (9 November 2026)

The remaining pipeline tools use the same pinned sbase source: tee/tail receive
owned descriptor I/O and cooperative sleep; sort retains upstream option/key
comparison code with a bounded external merge driver. Private sort runs live in
/tmp and are owned through completion/error/cancellation. Tail -m and tee -i are
explicitly unsupported, not silently approximated.

File operations and catalog inspection use original native HFS implementations
inside independently loadable files/inspect command images. cp stages both
forks before replacing an existing file. Metadata editing is hfsattr; chmod has
no meaningful permission model here and remains absent, with a command-not-found
hint pointing to hfsattr. touch does not infer type or creator for new files.
Ordinary mkdir/rm/ln/readlink implementations move out of the shell, leaving only
shared platform/namespace primitives in the host application. Native od/more
and upstream basename/dirname complete the planned inspection/pager names.
Help must describe actual supported options and limits, not claim full Unix or
GNU parity. Publication waits for guest validation of all four requested groups.

## Interactive editing update (9 November 2026)

The command editor wraps by terminal columns and keeps its cursor in view when
an editable line occupies more rows than the window. A cursor marker in the
bounded screen model makes the inverse block part of each offscreen row/frame;
typing no longer removes a visible cursor before painting its replacement.
Both native and optional cached renderers consume the same compositor.

Busy-command typeahead uses 1024 key slots and fixed modifier storage (3 KiB),
replayed by the outer event loop without recursive command execution. A full
queue beeps and refuses further keys. Pager, serial and canonical-input owners
retain their keys; cancellation still takes effect immediately. Explicit
backslash-Return continuation uses a separate bounded 1 KiB assembly buffer
and `> ` prompt. The escaped newline is removed before parsing, including
within double quotes; single-quoted backslashes remain literal. Ctrl+C clears
the unfinished command. This does not claim automatic continuation for every
incomplete shell grammar construct.

The activity clock retains overdue half-second deadlines, presenting one next
clockwise frame per cooperative visit rather than selecting a frame directly
from elapsed time. Painting commits a deadline; inaccessible/dirty views do
not consume frames. Completion and interactive screen ownership suppress it.
No interrupt-time drawing or extra animation allocation is introduced.

## Priority update, 9 November 2026

After the utility release, prioritise a faithful upstream nano port (evaluate
tiny configuration) ahead of further serial client/host-server work. Preserve
nano source, licences and authorship; implement only the required native
screen/input and file/runtime adapters, and validate editing/saving/cancellation
and bounded memory on Plus/4 MB. A source assessment or host build is not a
completed Macintosh port. Serial work stays deferred.

Nano dependency decision: use upstream ncurses, with native Macintosh platform
services beneath it. Do not replace curses with a nano-specific imitation.
