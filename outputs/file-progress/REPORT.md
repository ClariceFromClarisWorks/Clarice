# Default file-operation progress — 18 November 2026

Follow-up: the resource-fork validation gap below is **resolved** using a valid
Rez-generated resource file and native System 6 Resource Manager reads.
[Results and explanation](resource/REPORT.md). Production binaries are unchanged.

User authorised development, testing and staging after agreeing default interactive
progress for cp/mv/rm and reuse of Clarice's existing progress system. Isolated
branch `codex/file-progress-20261118`, based on accepted ls commit 4cc7eb6. No Core,
installed utility collection, physical machine, publication or release changes.

## Design and implementation

Reuses `src/commands/progress.h` (already used by FTP/wget) and runtime
`app_progress`. The common bar painter now also accepts an item-count message;
no second renderer, window, background worker, allocation or preliminary directory
scan. The existing native HFS implementation and mutation ordering are retained.

Interactive cp/mv/rm enable progress by default, with a 30-tick/half-second initial
delay and at most one ordinary update per 60 ticks. A final/error row is exempt
from throttling. `--no-progress` suppresses it; scripts and redirected stdin/stdout
suppress it. Stderr is checked through the existing `cli_isatty(2)` runtime API.
The ordinary shell activity indicator is separate and may still appear.

Copy and copy/delete moves use the already fetched sum of data/resource logical
lengths, count only successfully written chunks, and display the current file.
Byte percentage is capped at 99 until file completion; the final command row
reports completed items with a filled bar only on success. No transfer-rate
estimate is shown: current-file bytes with a whole-command clock would mislead.
Counters include completed files and directories, not failed/attempted items.
Native same-volume moves keep their direct rename/move path. Slow recursive
traversal/removal uses the existing indeterminate bar and item counts, with no
invented whole-tree percentage. Nonprintable filename bytes are sanitized before
progress is passed to a remote terminal. Narrow displays omit the filename prefix
rather than wrap the row.

Errors finish the current row before diagnostic text. Cancellation finishes with
“stopped”, returns 130 and retains existing cleanup/recovery behavior. Removal is
still direct HFS deletion: it is not Trash or undo, and cancellation cannot restore
items already deleted. Directory cross-volume mv remains unsupported as before.

The shared helper is established in-tree code, so no new third-party implementation
or speculative progress framework was required. Extra UI work is bounded formatting
and a clock check; the copied bytes, fork buffers, native queries, file writes and
safe replacement/locking decisions are unchanged.

## Validation

- UI/helper ASan/UBSan: delayed display, throttle, current/total 32-bit fork sizes,
  incomplete 99% cap, success/error rows, columns 1..132, sanitized names, counts,
  script/file/device suppression and explicit opt-out: [UI](ui.txt).
- Actual argument parser: default/quiet flags, `--`, invalid options/status/stderr,
  help/stdout and no leakage of the new option to other commands: [options](options.txt).
- 116 real fork-copy/replacement fault cases: fail each native boundary, short
  writes, every cancellation point, both-fork source/destination integrity,
  recovery and closed refs. Real progress counters match 12654 bytes across both
  forks; no premature 100% row: [faults](copy-faults.txt).
- Existing deep traversal,25 allocation failure boundaries, dot/dotdot removal
  rejection, locked sources/destinations and native move errors pass:
  [depth](depth.txt), [remove](remove.txt), [filesystem](filesystem.txt).
- Existing network progress regression passes unchanged, including integer sizes,
  overflow boundaries, widths 1..200 and redirected suppression: [shared UI](shared-ui.txt).
- Native 68000 compilation and packing pass with warnings as errors. An initial
  guest candidate incorrectly imported unexported error_ref and was rejected
  before execution. Replaced with supported cli_isatty; the focused build now
  verifies every package import against the existing runtime export table.
  Failed candidate results are excluded from acceptance. [Build](build.txt).

## Limits and staging boundary

System 6/stockSE physical measurements and System 7/MultiFinder acceptance remain
separate from host/Snow evidence. No new native-process backend is included.
The existing cli_isatty reports virtual stderr devices as terminals; stdout virtual
devices are explicitly excluded, and --no-progress remains the unconditional
opt-out. Correcting that shared runtime behavior is outside this utility patch.

## Snow results and measured costs

Normal-speed Snow 1.5.0, Macintosh Plus, 4 MB, System 6.0.8, existing Core
2026111882/runtime 1.10. Eight 320 KiB two-fork copies completed successfully.
First baseline/candidate: 6.75/6.00 seconds. Three repeated replacement medians:
6.86/7.16 seconds (+0.30 seconds, 4.4%). These include shell timing and loading;
progress is suppressed by script scope. Larger package loading contributes:
repeated loader medians 58/72 ticks. No speed improvement is claimed.
[Results](guest-results.txt), [measurements](measurements.json).

Interactive copy, cross-volume move and recursive copy/removal displayed the
shared bar and completed counts (11 items for the ten-file directory). Ctrl+C
stopped the large copy, returned 130, and left no fp-cancel destination or copy
sibling in the test directory. Explicit --no-progress displayed no bar. Runtime
counters for matched replacement copies were 402 ticks quiet versus 382 ticks
visible (6.70 versus 6.37 seconds), including module loading/cleanup. One pair is
noise-limited, not proof of zero overhead. The initial visible create was 375 ticks.
The existing `time`/`env` shell wrappers increment script_depth and thus suppress
these bars too. UI timing logs: [on](ui-on-repeat.txt), [off](ui-off.txt).

Post-command heap in the eight-copy run spans 20 bytes; largest block settles at
355824 bytes for both variants. Runtime-owned remaining allocations are zero.
These counters do not measure stack high-water or native allocations directly.
Static build inspection: loaded image 8136 → 11540 bytes (+3404); package
13364 → 18916 (+5552); transient loader metadata 5180 → 7328 (+2148).
Extra BSS is 144 bytes, included in image size. No progress heap allocation or
resident Core storage. Existing copy-fork stack frame stays 4112 bytes; progress
show/painter frames add 160/264 bytes plus saved registers while active. Exact
whole-command peak stack remains unmeasured. [Package sizes](packages.json).

Guest status checks pass for rename, cross-volume move, recursive copy/removal,
locked-file errors, invalid options and quiet mode. Data-fork SHA-256, logical fork
lengths, type/creator and timestamps match after cross-volume move. **Full resource
fork equality did not pass**: bytes 48–125 differ in both unchanged baseline copy
and candidate move, containing temporary filename/native bookkeeping. All other
resource bytes match. The imported source remains unchanged. The synthetic fork
is arbitrary descending bytes, not a valid Resource Manager structure, so this
run cannot establish preservation of valid resource files. This is recorded,
not normalized into a passing full-fork claim: [fork evidence](fork-validation.json).
The valid-resource follow-up now passes and establishes these differences as
system-reserved bookkeeping, with all resource/application data preserved. No
new data-safety implementation was needed.

Source, focused tests and evidence are staged on the isolated branch. Candidate
cp/mv/rm packages and SHA-256 manifest are in `work/file-progress/stage`, requiring
runtime 1.10. The progress change was subsequently committed and integrated into the development
branch as a9fe185 at the user's request. This resource-validation follow-up is
staged separately. No push, release or hardware deployment. Stock SE and
System 7/MultiFinder acceptance remain the next validation steps.
