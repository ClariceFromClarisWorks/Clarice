# Clarice CLX1 command interface — current implementation

Reconciled 10 November 2026 against `src/module.inc`, `src/ports/runtime.c` and
`scripts/pack-command.py`. This documents today's private interface, not the
future stable SDK or two-backend contract. See [architecture](ARCHITECTURE.md).

## Image and invocation

Baseline 68000; pinned Retro68 C convention, 32-bit C integers/pointers. The
packer consumes big-endian m68k ELF relocatables and emits CLX1. Header fields:

| Offset | Size | Meaning |
| --- | --- | --- |
| 0 | 4 | `CLX1` magic |
| 4 | 4 | Format version: legacy 1, requirement-bearing 2 |
| 8 | 4 | Loaded image bytes, including code/data/zeroed BSS |
| 12 | 4 | Entry offset (`int clarice_main(int argc, char **argv)`) |
| 16 | 4 | Relocation count |
| 20 | 4 | Import count |
| 24 | 4 | Checksum of image and metadata |
| 28 | 4 | Reserved, must be zero |

Integers are big-endian. Twelve-byte relocation records and fixed 64-byte import
names follow the image. Loader limits: 262144 image bytes, 8192 relocations,
128 imports. It checks size, entry alignment, checksum, names and nonoverlapping
relocation offsets, resolves its explicit export table and releases metadata
before entry. Assembly checksum yields between 1 KiB chunks. The image is freshly
allocated for every invocation and freed afterward; there is no resident code cache.
Exit status is masked to 8 bits; invalid images return 126, missing commands 127.

### Minimal independent-update contract (Task 2)

Format 1 remains accepted without changing bundled bytes. Format 2 retains the
32-byte prefix and appends these fields before the image (48-byte total header):

| Offset | Size | Meaning |
| --- | --- | --- |
| 32 | 4 | Required runtime major: must equal the host's major |
| 36 | 4 | Minimum runtime minor: must not exceed the host's minor |
| 40 | 4 | Required capability bits: unknown/unavailable bits reject the image |
| 44 | 4 | FNV-1a of header bytes 0–43 |

Runtime is **1.0**. Capability bit 0 (`0x1`, **native-clx1**) denotes the baseline
68000/pinned Retro68 calling convention, relocation model, synchronous invocation
and explicit native import table described here. Bits 1–31 are unassigned and
rejected. It does not promise POSIX, complete stdio, isolated processes, or general
curses/networking semantics. Individual imports must still resolve. New services
may be added compatibly within major 1; breaking calling conventions, layouts or
existing semantics require a new major/capability contract, never silent reuse.
Experimental editor interfaces remain experimental; this is not the public SDK.

The payload checksum is unchanged: FNV-1a over image, relocations and import names,
starting after the complete header. FNV detects accidental corruption, not malicious
modification or publisher identity. Runtime/header/checksum/import/relocation checks
finish before any entry-point execution. `appinstall check FILE` uses the same
validator without executing the command or preparing stdio.

Independent pack example (after compiling a baseline ELF object):

```sh
python3 scripts/pack-command.py command.o command.clx --runtime 1.0 --capabilities 0x1
```

Without `--runtime`, the packer emits byte-identical legacy format 1. Older Clarice
builds reject format 2; they do not acquire update safety from a new command file.
Install the Task 2 terminal first. No application-version ordering is needed:
startup preserves *every* existing entry, and replacement is always explicit.
Application versions can be reported by the application itself. Some native imports
still expose host structs, globals and newlib FILE/ctype state; this contract is
limited to the pinned build environment, not a general Linux/Unix binary ABI.

## Current runtime

Ordinary tools, including mkdir/rm/ln/readlink, are loadable actual implementations;
small shell-state/UI operations remain builtins. Some tools share multicall images.
Streams 0–2 and raw descriptors 3–11 are distinct bounded mechanisms. `cli_read`
returns bytes, zero EOF or -1; `cli_write` handles stdout/stderr. Shell duplicates
share native file offsets. Pipelines are sequential HFS spools, not live pipes.

The port shim owns allocations, exit handling, streams/descriptors and temporary
names. Ceiling: 96 KiB, including at most 8 KiB of allocation reuse; not reserved
or guaranteed memory. There are twelve streams with 512-byte buffers, a 16 KiB
line bound, nine nonstandard descriptors and twenty tracked private temp names.
Image, loader workspace, stack, host FILE pool and screen ownership are additional
costs. Cleanup runs on port return/exit/cooperative cancellation; unmanaged host
state and arbitrary third-party allocations are not automatically isolated.

Raw descriptor `lseek` exists. Experimental fdopen and exclusive/update opens
have been added locally. They are **not complete stdio seeking/update support**:
funopen has no seek callback and port_fopen's read/write selection remains limited.
Files are data-fork bytes without newline conversion. printf z/t length adaptation
and direct newlib buffer access tie this interface to the pinned toolchain.

`app_activity_interactive` suppresses the overlay. Experimental screen ownership,
partial termios/select/ioctl and local signal/environment services support the
real ncurses probe. These are not asynchronous OS signals or general socket
readiness. Service truth and gaps are listed in [PORTABILITY.md](PORTABILITY.md).

## Installation, replacement and recovery

Startup installs an embedded command only if its catalog entry is absent. It does
not open, checksum, rewrite or relabel existing entries, even foreign, damaged,
incompatible or directory entries. Validation/diagnosis is on execution or explicit
`appinstall check`; this avoids rereading the entire userland on every launch.

`appinstall replace FILE NAME` installs a CLX data fork in `/bin/NAME` after copying
to `.clx-NAME.new`, closing, full validation, and flushing the target volume.
NAME is limited to 1–20 ASCII letters/digits/underscore/hyphen. The original is
renamed to `.clx-NAME.old` and flushed before committing the new name. Existing
Finder info (including extended Finder info) is copied to the candidate. The
previous file, including both forks and metadata, remains intact under `.old`.
New entries use type CLX1 / creator CLRC. Source resource forks are not installed;
CLX images are self-contained data-fork programs. No configurations are touched.

`appinstall repair NAME` explicitly installs the bundled version using that same
transaction; it can intentionally downgrade. `appinstall recover NAME` restores
`.old` even if it was originally damaged; it does not execute or bless that file.
If a current candidate exists, recovery retains it as `.new` before restoring `.old`.

Neither transaction name is overwritten or automatically deleted. A pre-existing
`.old` or `.new` blocks another update; inspect and explicitly move/archive it first.
Startup also refuses to fill an absent destination when either recovery name exists.
A failed commit attempts to restore the old name. If restoration also fails, the
old file remains at the reported `.old` path. A failed final flush reports failure
with both versions retained. Staging files created by the current failed attempt
are removed where possible; pre-existing files are never cleanup targets.

HFS rename plus FlushVol is not a journal or a whole-volume power-loss guarantee.
Host fault tests cover failure boundaries; normal Plus/System 6.0.8 workflows now pass guest acceptance; arbitrary power-loss
and physical durability remain unverified. Locked/read-only/full media errors propagate without clearing
locks. Commands execute cooperatively; concurrent writers to these names are not
supported. See [Task 2 evidence and guest procedure](task2/TEST-REPORT.md).

## Loader diagnostics

`status runtime` retains total `load` ticks and adds `read`, `validate`, `imports`,
`relocations`, and `prepare` ticks for the last successfully loaded invocation.
These count TickCount deltas (60 Hz), including cooperative yields in their phase.
Read includes open/header/payload reads; validation includes header/payload hashes;
relocations includes record validation and patching; preparation includes stdio,
allocations, metadata disposal and close. A few boundary/bookkeeping ticks may
fall outside phase totals. Failure resets counters; `appinstall check` does not
replace execution statistics. No optimisation or startup speed claim accompanies
these counters. First rendering/input readiness must still be timed separately.
