# Filesystem confidence follow-up — 18 November 2026

The merged development runtime was tested in disposable Snow Plus / 68000 /
4 MiB / System 6.0.8. Two additional data-safety defects were reproduced and
fixed. Candidate Core **2026111842**, runtime **1.10**, with file utilities
**13,348 bytes**. Filesystem fixes are committed on `codex/coherent-deployment`; existing nano,
serial, Kermit and release work remains uncommitted and preserved. No push, release or physical-Mac installation.

## Defects and corrections

| Finding | Reproduction before fix | Correction |
|---|---|---|
| FS-004 destructive dot operands | `rm -R tree/.` and `rm -R tree/child/..` deleted sentinel contents and returned 0 in Snow. | Reject final `.` / `..` components before resolution/traversal, including trailing slashes. Applies to rmdir as well. |
| FS-005 empty pathname resolves to cwd | `rm -R ''` deleted the test current directory and returned 0 in Snow. | Shared pathname resolver rejects empty input before catalog access. File tools and other callers receive a real invalid-name error. Explicit `.` still resolves normally. |

Both new host regressions fail against the original implementation and pass
against the correction. Native macOS BSD rm was independently checked with the
same dot/dotdot/empty operands: nonzero status and sentinel preservation.
The fix restores familiar Unix behavior without introducing a new abstraction,
cache or OS service. Original HFS operations and upstream applications remain.

## Evidence

- Final standard guest suite: **71/71 passed**, including empty operands across
  rm/cp/mv/touch/mkdir/readlink/cat; dot and trailing-slash refusal; normal copy,
  overwrite, multiple operands, case aliases, paths/spaces/dashes/name lengths,
  recursive operations, symlinks/dangling links/cycles, locks, file-vs-directory
  errors, cross-volume forks, cwd rename and repeated cleanup. `guest.json` records
  commands, expected/actual statuses and output. The first build's **37/41** and
  destructive empty-operand reproduction are retained separately as negative evidence.
- **116 ASan/UBSan copy/replacement cases** run production fork-copy functions
  with a deterministic native-boundary fixture: failures at every normal native
  operation, short writes, cancellation during data/resource copies, source
  integrity, complete destination/recovery data and closed file references.
  These are host boundary tests, not simulated physical media failures.
- Existing depth-24 traversal / 25 allocation-boundary / cancellation and cleanup
  tests pass. Existing cwd/session/locked-source and volume/rename negative-control
  regressions pass. Resolver tests retain same-operation metadata reuse.
- Focused 68000 Core and file-utility compilation/linking pass. Source whitespace
  checks pass. Compilation is separate from guest execution.
- **15 completed large-file/full-volume checks** pass across `stress-prefill.json`
  (five setup/large-file checks) and `stress-resume.json` (ten completion/recovery
  checks). The first volume-fill exceeded its 300-second harness timeout and left
  a caller-owned partial filler. Reconnection and retry confirmed native disk-full
  errors; cp and mv retained the existing target, mv retained its source, and
  copying worked after freeing space. The test timeout is now 600 seconds.
- **Three real guest interrupted-copy cycles** pass: Telnet IAC IP interrupts
  a 2 MiB replacement, status is 130, the old target survives, and owned survivors
  are zero. Native heap/largest snapshots are recorded in `cancel.json`.
- After successful explicit native `FlushVol` for both fixture volumes and the
  default volume, application quit, and owned-emulator termination, independent
  MacBinary exports pass: data/resource fork hashes and type/creator match for
  overwrite, cross-volume move, disk-full retention and cancelled-copy retention.
  Fixture timestamps also match, but both fixture dates are zero; nonzero dates
  are covered by the host boundary test, not claimed as a new guest date test.
  Both **2,097,163-byte** local/cross-volume copies match the source byte-for-byte.
  No temporary siblings remained. `forks/comparison.json` contains exact hashes.
  The exported native flush fixture was verified against its known CLX hash.
- The final guest was explicitly flushed and quit because the UI harness did not
  reliably select Finder's Shut Down menu. No live disk-image inspection was used.
  Earlier control/setup retries were discarded; only the final staged candidate
  contributes passing acceptance evidence.

## Costs and practical limits

File image **13,268 → 13,348 bytes (+80)**. Core MacBinary remains **279,552 bytes**
at the package's padded granularity; this is not a claim of identical machine
code size. No new allocations, resident state, runtime imports, catalog I/O,
startup work or caches. One empty-input branch per resolution and a bounded
backward operand scan per removal. Existing strlen work is reused.

All five final repeated cp/rm snapshots held native free heap **332,788 bytes**,
largest block **331,450 bytes**, and invocation-owned survivors **0**. This is
settled guest memory evidence, not a hardware peak-memory or latency benchmark.
The emulator shared host resources with other tasks; host durations are diagnostic
and are not physical 8 MHz SE measurements.

A separate `wc -c` scan of a 2 MiB file was stopped after prolonged execution;
no successful large-wc result is claimed. Its upstream byte/rune scanning path
needs a separately measured throughput follow-up. Transfer verification instead
uses catalog lengths and independent offline byte hashes. The owned client was
closed and the supplementary session recovered for subsequent tests.

No sudden-power-loss, damaged-media, physical SE, System 7 or MultiFinder
acceptance is claimed. Existing documented limitations remain: cross-volume
folder moves / simultaneous parent-and-leaf folder moves, directory metadata
preservation, and cwd changes made by another Macintosh application. HFS is not
a journaled filesystem; backups remain appropriate for valuable originals.

## Reproduction

From the development checkout:

```
python3 tests/filesystem-confidence/remove-operands.py
python3 tests/filesystem-confidence/empty-path.py
python3 tests/filesystem-confidence/copy-faults.py
python3 tests/utility-correctness/file-depth.py
python3 tests/filesystem-fixes/host.py
python3 tests/namespace/resolve-info.py
python3 tests/namespace/volume-host.py
python3 tests/namespace/rename-host.py
```

Guest scripts `guest.py`, `stress.py` and `cancel.py` require the specifically staged
**disposable** FS Audit A/B volumes and loopback port 23283. They are destructive
test workflows, not commands to run against personal disks. `offline.py` refuses
to read a running guest's disks; run only after native volume flush and application quit (or Finder Shut Down),
then termination of the recorded owned PID. Integration build/staging artifacts are retained in
`Clarice-Integration-Filesystem/work/filesystem-confidence`.

Recommended next step: a physical SE smoke test of this candidate before relying
on it for the only copy of important files. No architecture or authentication
policy changes require review in these fixes. Concurrent opt-in physical-network
diagnostics and their current-state report were preserved; they were not part of
this frozen guest candidate. The tested source and binary hashes are recorded.

## Commit integration

Only the safety guards, build identifier, new tests and this evidence are committed.
The empty-path regression is self-contained and supports both the committed
resolver and the newer metadata-aware resolver still in development. Focused
operand/copy-fault regressions are checked against both forms during integration.
The broader Snow evidence above belongs to the recorded combined development
candidate (runtime1.10), not a claim that unrelated work has been committed.
