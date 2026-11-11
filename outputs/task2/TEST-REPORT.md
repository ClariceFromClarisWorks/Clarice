# Task 2 — Safe independent application updates

10 November 2026. Roadmap 1.2 only. **Implemented; automated Plus guest acceptance completed.**
[43/43 guest checks and three measured loader samples](guest/ACCEPTANCE.md)
supersede the earlier pending/manual-only notes below. No release, push or Task 3 work. [Integrity check](integrity.json) confirms the
290 pre-existing implementation/script/test files outside the four intended edits
are unchanged; all unpublished ncurses/nano implementation files are preserved.

## Changes

- `src/module.inc`: common execution/check-only validator; legacy format 1 and
  requirement-bearing format 2; specific damage/incompatibility diagnostics;
  five loader phase counters; startup installs only absent catalog entries.
- `src/command-install.inc`: bounded stage/validate/flush/rename transaction,
  explicit replacement, bundled repair and recovery; no automatic backup deletion.
- `src/shell.c`, `src/help.inc`: `appinstall` administrative builtin and help;
  `status runtime` phase counters.
- `scripts/pack-command.py`: optional `--runtime MAJOR.MINOR --capabilities MASK`;
  default legacy output unchanged. No terminal rebuild needed to pack an update.
- `tests/commands/update-host.c`, `updates.py`, `update-probe.c`, `update-68k.py`,
  `update-guest.sh`, `update-restart-guest.sh`: focused fault, format, execution
  and prepared guest tests.
- `AGENTS.md`, README, current state, roadmap, portability matrix and command ABI:
  current scope, contract, policy and evidence. Existing editor/runtime sources
  remain unchanged except the shared loader/shell integration described above.

## Behaviour

Startup never overwrites an existing command, whether valid, newer, foreign,
corrupt, incompatible or a directory. It does not checksum existing commands at
startup; diagnostics occur when executed or explicitly checked. This keeps normal
startup to catalog lookups. A missing destination with recovery/staging siblings
is left absent with an error, so startup cannot hide an interrupted replacement.

Examples:

```sh
appinstall check /mnt/Updates/grep.clx
appinstall replace /mnt/Updates/grep.clx grep
appinstall repair grep
appinstall recover grep
```

`replace`/`repair` first require no `.clx-NAME.new` or `.clx-NAME.old` in `/bin`.
They write the candidate to `.new`, close it, validate it without executing, preserve
existing Finder/extended Finder information and flush the volume. The previous
file is renamed to `.old` and flushed, then the candidate takes the command name.
The previous file retains its identity, both forks and metadata. Its backup is
never automatically deleted. Recovery restores `.old`, retaining any current
candidate as `.new`. Existing user configuration and unrelated files are untouched.

Inspect/move/archive old transaction files explicitly before another replacement.
If `.new` survives an interruption before `.old` exists, the original command is
still installed. Check the candidate and move it elsewhere before retrying; no
automatic decision is made about which version the user wants. Repair deliberately
uses the bundled version and may downgrade, but only on explicit request.

Compatibility: runtime **1.0**, capability **native-clx1** (`0x1`). Format 2 declares
required major, minimum minor and capability bits, with a separate header checksum.
Unknown format, wrong major, newer minimum minor, unavailable capability, missing
import, invalid relocation or checksum mismatch prevents entry-point execution.
Format 1 remains supported. This is a pinned native import contract, not a general
POSIX/libc ABI or public SDK. [Exact specification](../COMMAND-ABI.md).

## Tests and evidence

| Environment | Result | Evidence |
| --- | --- | --- |
| Host, actual production loader/installer with fault-injected Macintosh API shims; AddressSanitizer + UndefinedBehaviorSanitizer | 41 scenarios pass; no leaked tracked allocations or open descriptors | [host-tests.txt](host-tests.txt), `tests/commands/update-host.c` |
| Host, actual production validator and actual command files | 21 images pass structural/checksum/import validation, including existing bundled images and newly packed requirements | [host-tests.txt](host-tests.txt) |
| Host packer regression | Legacy core image byte-identical; format-2 payload identical, requirement fields/header hash checked | [host-tests.txt](host-tests.txt), `tests/commands/updates.py` |
| Unicorn 68000 CPU model, independently cross-compiled CLX fixture | Legacy v1 and versioned v2 each execute three times; expected output and exit status | [68000-tests.txt](68000-tests.txt) |
| Existing host builds of source-port utilities | grep match/no-match statuses, sed, awk and wc output smoke checks pass | [utility-smoke.txt](utility-smoke.txt) |
| Pinned Retro68 toolchain | Incremental 68000 terminal compile/link/package succeeds with warnings treated as errors; no ncurses/nano dependency rebuild | [build.log](build.log) |
| Snow Plus / System 6.0.8 / 4 MB / MultiFinder | **43/43 extracted guest-result checks pass; three ncurses loader samples collected** | [Automated acceptance](guest/ACCEPTANCE.md), raw outputs, hashes and screenshots. Reused build on a disposable system-disk copy. |
| Physical Macintosh | **Not run** | No hardware evidence claimed. |

The 41 scenarios include fresh installation, repeated startup, independent version
survival, foreign/corrupt/incompatible preservation, capability/header/payload and
relocation/import rejection, successful replacement/repair/recovery, metadata/fork
retention, partial/failed writes, close and flush failures, cancellation during
copy/validation, failed commit, failed rollback and simulated interruption at each
flush boundary. Recovery names are not overwritten; directory/path violations
are rejected. The fault shim models filesystem operations, not HFS power-loss
consistency or genuine locked/read-only/full media behaviour.

CPU fixture execution uses a test `cli_write` stub and a test relocation harness;
it is **not** a guest test of Macintosh I/O or the full terminal. Host utility smoke
checks use existing host binaries and do not prove the changed guest loader runs
those tools. The later automated guest run also verified ls, df, grep, sed and awk; see the acceptance report.

Reproduce host tests after compiling the two `update-probe.c` objects with the
existing toolchain (`-m68000 -Os -ffreestanding -DPROBE_VERSION=1`, then `=2`):

```sh
python3 tests/commands/updates.py
work/date-test-env/bin/python tests/commands/update-68k.py
```

The second command uses the existing isolated Unicorn installation. See
[build instructions](BUILD-AND-GUEST.md) for exact fixture/build steps.

## Performance and resource cost

The completed guest run measured ncurses loader totals **313 / 279 / 278 ticks**
(5.217 / 4.650 / 4.633 seconds). Validation/checksum accounted for 199 / 198 / 198
ticks, the dominant phase. [Full five-phase table and raw evidence](guest/ACCEPTANCE.md).
These are actual Task 2 normal-speed Plus emulator observations, separate from
historical 277–338 tick measurements. No optimisation was performed. Validation
includes cooperative event handling and is not a pure checksum CPU benchmark.

Added persistent timing state: five 32-bit counters (20 bytes) and one startup
flag. Replacement copies in 4 KiB chunks with a maximum accepted CLX file size of
368,688 bytes. Full validation temporarily allocates the image and metadata within
existing loader limits; no resident command cache or larger partition was added.
Copy/validation permit cancellation during explicit updates. The rename/flush
commit sequence does not yield to UI input; cancellation resumes after consistency
is restored. Volume flush latency remains a guest measurement requirement.

## Remaining limits and decisions

- Requested Plus restart/update/recovery, utility smoke checks and first/repeat
  ncurses loader timings are now verified. Physical hardware and guest power-loss/
  full/read-only-volume faults remain untested. No nano startup acceptance is claimed.
- HFS has no journal here. Flush/rename boundaries preserve recoverable filenames
  under tested API failures, but cannot guarantee arbitrary disk/controller failure.
- Only `/bin` command leaves of 1–20 ASCII letters/digits/underscore/hyphen are
  accepted by the update operation. Source packages are CLX data files; source
  resource forks are not installed. The previous version's resource fork is retained.
- Fixed recovery names intentionally require explicit archival before a second
  update. Concurrent writers/updaters are unsupported. No automatic version
  ordering, signatures, network updater, dependency solver, SDK or worker redesign.
- Product decisions requested in Task 2 are implemented. Review the deliberately
  small contract/name limits and explicit backup-retention workflow before public
  distribution. No additional product decision is required to keep the safe
  default; the requested guest acceptance gate is now complete.

Stop at Task 2. Do not begin invocation/memory redesign or resume editor work.

## Validation-only follow-up — manual acceptance prepared

The existing `Clarice.dsk` and `Task2.dsk` were read and their SHA-256 hashes and
sizes matched `artifacts.json`. The existing fixture scripts and hfsattr/od syntax
were inspected. No rebuild or host regression rerun was performed.

[Exact manual checklist](MANUAL-CHECKLIST.md) covers independently updated grep
surviving restart, deliberate installed corruption surviving another restart,
explicit replacement/recovery/repair, visible Finder metadata fields, and three
loader timing samples. It uses existing fixtures and includes expectations, not
observed guest results. Full extended metadata/resource-fork validation remains
outside what these displayed fields establish.

At that manual-checklist stage, guest acceptance remained pending and no emulator
commands had executed. This historical status is superseded by the automated
validation below. No implementation, Task 3 work, publishing or pushes occurred.

## Automated guest follow-up — completed

The permission state now allowed control, but direct text injection remained lossy.
Existing Snow clipboard typing, a small host mouse-click helper and HFS guest-side
scripts completed the tests without manual testing by the user. No application or
emulator rebuild and no host regression rerun were needed. All existing build/image
hashes still match the recorded Task 2 artifacts.

**43/43 checks pass** against guest-produced outputs and byte comparisons, including
fresh installation, independent-update restart persistence, retained damaged files,
repair, recovery and Finder fields. Three unchanged ncurses demo runs yielded usable
phase timings. [Acceptance report](guest/ACCEPTANCE.md) documents automation attempts,
the duplicate initial script/refusal results, the clean positive confirmation,
limitations, raw files and screenshots. The disposable test emulator is stopped.
Task 2 is complete for the requested Plus guest acceptance scope; stop here.
