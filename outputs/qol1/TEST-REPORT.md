# QOL-1 — mounted HFS volume resolution

11 November 2026. Scope: directory/catalog volume selection only; no push or release.

## Root cause and reproduction

`PBGetCatInfoSync` accepts `ioVRefNum` as an input. A full HFS pathname can locate
an item on another disk without rewriting that field. Clarice's `info()` supplied
zero, then `directory()` and `ls` reused the zero alongside the returned directory
ID. Indexed catalog enumeration consequently selected the default volume. Root
ID 2 exists on every HFS volume; non-root IDs can identify unrelated directories
or fail. Shell quoting and textual cwd were correct in this reproduction.

The unchanged Task 2 application reproduced the bug on a disposable Plus boot
disk and a distinct `Test 2` floppy: both quoted and escaped `cd` yielded
`/mnt/Test 2`; direct `cat sentinel.txt` returned `SECOND-VOLUME`. Both relative
and absolute `ls -la` instead listed `System Folder`, `Clarice`, and the other
boot-root entries. Listing the floppy's `sub` returned only dot entries instead
of `second.txt`. Raw pre-fix outputs are in [before](before/).
The separately reported absolute-path -43 was not reproduced on this particular
catalog; wrong-volume IDs explain why results can depend on the disks involved.

## Focused change

- `src/namespace.inc`: resolve a unique mounted label to a real HFS volume ref;
  reject missing/duplicate labels and propagate enumeration errors. Share this
  lookup with existing native-file operations. No cached refs survive unmounts.
- `src/shell.c`: bind the explicit ref before `PBGetCatInfoSync`. Existing callers
  can safely pair the returned directory ID with that volume, including `ls`
  and directory completion. Reject invalid/relative native paths at this boundary.
- `src/port-files.inc`: remove its duplicate first-match volume lookup; retain its
  existing interface and callers.
- `tests/namespace/volume-host.py`, `volumes-guest.sh`, `volumes-validate.py`:
  focused regression coverage and checks of actual extracted guest output.

Existing external utility binaries import `info` from Clarice and need no update.
Only the terminal was rebuilt using the existing pinned Retro68 container.
All other pre-existing source/script/test bytes match the pre-QOL-1 manifest,
including Task 2 and unpublished ncurses/nano work.

## Validation

Host: new helper regression passes under ASan/UBSan and rejects a mutation restoring
the original zero-volume behavior. Covers spaces/case, indexed enumeration,
missing/ambiguous labels, invalid native paths and volume-enumeration I/O errors.
Existing shell tests: 31/31 plus 3/3 cancellation checks pass. Logs are alongside
this report. Build and `git diff --check` pass.

Guest: Snow v1.5.0-a921b10, Macintosh Plus, 4 MiB, System 6.0.8 with
MultiFinder, normal Video speed and no overclock. Used a copy of the existing
`work/system608/system.hda`, the already-built Task 2 app for reproduction, and a
new disposable 800 KiB `Test 2` fixture. The user's running Snow instance and its
Task 2 disk were not modified. Automated input used the existing clipboard/click
helpers; completion and application exit were visually confirmed before offline
extraction. An initial capture attempt stopped the emulator before its queued
exit completed; those missing outputs were discarded and the reproduction rerun
with confirmed exit. No unexecuted checks are counted.

[Extracted regression transcript](after/regression.txt) and
[validator result](guest-validation.txt) confirm:

- Quoted/escaped volume names, absolute/relative root and nested listings, `cd`,
  `pwd`, parent traversal and reads select the intended catalog.
- `mkdir`, redirection, `cp`, `mv`, `touch`, `rm`, `rmdir`, cross-volume copies and
  symlinks work on disposable files. Same-named boot/floppy guard files retain
  distinct contents. Offline extraction confirms boot guard `BOOT-GUARD` and
  cross-volume copied content `SECOND` after application exit.
- Missing volumes/directories fail; failed `cd` retains cwd. Redirection failures
  return status 2, utility/cd failures 1. No fallback writes/deletes hit the boot
  guard. The raw floppy loader retains guest writes in memory rather than writing
  them back to the source fixture, so floppy mutations were verified in-session.
- Existing `tests/namespace/paths.sh` and `links.sh` execute and match expected
  transcripts (with the additional mounted `Test 2` entry allowed).

Performance: the existing `benchscan` reports 4/4/4 ticks for the four-entry
floppy root, and 331/333/333 ticks (5.52–5.55 s) for 1,000 boot-volume entries.
These are catalog-only guest measurements, not rendered-listing or real-hardware
timings. No matched pre-fix timing sample was taken, so no numerical speed delta
is claimed. The fix adds a mounted-volume scan per `info()` lookup; it does not
add a scan per entry to `ls`, heap allocations, resident caches or rendering work.
Performance with many mounted volumes remains unmeasured.

## Boundaries

No physical-hardware validation. Duplicate-volume and injected I/O-failure cases
are host mock coverage, not claims of guest fault injection. This is not a general
filesystem redesign or hot-unplug transaction guarantee. Full-path HFS file calls
continue using their native pathname semantics; safe cross-volume mutation tests
are required separately from a successful textual resolution.

Task 2 acceptance was not resumed; latest scope explicitly stops after QOL-1.

## Reproduction assets and next step

`Clarice.bin` is the fixed test build, not a published release. `Test 2.dsk` is the
original disposable fixture containing `sentinel.txt` and `sub/second.txt`.
`before/script.sh` records the pre-fix commands; the guest regression source lives
in `tests/namespace/volumes-guest.sh`. To repeat, use a fresh disposable boot copy,
install the chosen application with HFS metadata preserved, create `/tmp/qol`,
and seed `/mnt/Sys6Unix/qol-safe/guard` with `BOOT-GUARD`. Run the regression with
stdout/stderr redirected to `/tmp/qol/regression.txt`; run existing namespace
paths/links fixtures into the corresponding report files. Exit Clarice and confirm
Finder is visible before stopping Snow and extracting results with hfsutils.
Do not host-mount an image still open in an emulator.

No product decision is needed for this correction. Recommended next step is owner
review of QOL-1; further Task 2 or roadmap work requires a new instruction.
