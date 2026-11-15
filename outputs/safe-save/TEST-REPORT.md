# Safe HFS replacement — completion report

14 November 2026. Host and Snow Plus, 4 MB, System 6.0.8. No physical acceptance,
push or publication. Existing runtime, QOL, Serial-3 and upstream editor work retained.

## Result

Reusable explicit staged replacement is implemented and used by GNU nano 9.2 tiny.
Existing-file and new-file editing/saving executed successfully in Snow. Original
contents are not truncated before successful writing/closing. Resource fork,
Finder information and creation date are copied to a same-volume sibling. Native
rename/flush commits with rollback; retained recovery siblings are never silently
overwritten. Abandoned saves are cleaned on invocation exit. See IMPLEMENTATION.md
for API, recovery instructions and limitations. Ordinary O_TRUNC is unchanged.

## Evidence actually obtained

- Host ASan/UBSan: actual safe-save.inc with HFS model; 27 single-fault boundaries,
  27 simulated interruption boundaries, short writes, disk-full injection,
  432 paired fault combinations including rollback failure and subsequent recovery,
  new-file failures, stale-file collision, cancellation, locked/read-only rejection,
  both-fork/metadata checks and 20 repeated saves. Original or complete replacement
  remains recoverable. No retained context allocations or normally closed handles.
  This models native failures; it is not a power-cut or real-disk durability test.
- Existing loader/installer host suite: 41 scenarios passed (loader-host.txt).
- Snow actual nano: edited /tmp/save-note by inserting `edited `, saved via
  Option-O/Return, exited Option-X, status 0. Created /tmp/save-new, saved
  `new document\n`, exited status 0. Screenshots show the real upstream editor.
- Snow repeated service probe: two invocations of 20 successful replacements each,
  exact content verification, abort after writing incomplete data, and abandoned
  pending transaction cleaned at invocation return. Both status 0 and remaining=0.
- Snow locked original refused (probe status 11 = begin rejected); original then
  unlocked. Reopened nano, made unsaved edits, cancelled with Command-period:
  status 130; saved contents unchanged. Normal shell restored.
- Extracted only after normal Clarice quit and confirmed Snow termination:
  existing data exactly `edited original data\r\n` (nano appended its normal final
  LF to the CR-containing fixture); new file exactly `new document\n`. Resource
  bytes, type/creator, Finder flags and creation date identical to the imported
  baseline. No .clarice-* transaction siblings left. validation.json: 13 checks pass.
  hfsutils zeros exported Finder location/folder fields, so nonzero placement is
  not verified in the guest. The host model checks standard/extended metadata
  copying, not actual Finder placement.

## Memory / timing

Both normal nano exits: free heap 398,354, largest block 394,044; portable owned
peak 69,032 / 69,014 and remaining 0. Both repeated-save probes: free heap 398,322,
largest 394,044; owned peak 2,128 and remaining 0. Equal within each pair; native
save context is outside portable owned counters, with cleanup evidenced by stable
heap and successful next begin. Cancellation: remaining 0, heap 398,318 (4 bytes
below probe result); this single sample is not a claim of long-term cancellation
leak freedom. There is no new permanent arena; idle transaction state is one pointer.

Nano image 197,896 bytes. Loader 575 ticks first, 461 and 457 subsequent (about
9.6 / 7.7 / 7.6 seconds); invocation elapsed times include human/automation waits.
No three-second startup or physical-performance acceptance claimed. Two 20-save
probe invocations took 1,348 / 1,365 ticks including load (~1.1 seconds/save for
these small files). Resource-fork copying cost scales with its size.

## Build / source changes

Normal 68000 application: build/Clarice.bin. Separate experimental editor:
build/nano.clx. Checksums and licence/installation notes accompany them. Nano is
not silently added to existing installations or advertised as a completed port.

- src/safe-save.inc; src/ports/safe-save.h: reusable host transaction and contract.
- src/shell.c, src/module.inc: inclusion, required imports, invocation cleanup.
- scripts/patch-nano-save.py, scripts/link-nano-probe.sh,
  src/ports/nano/{nano-compat.h,platform.c}: narrow upstream save adapter.
- src/app.c: editor-owned Option-O/X fallback for Plus keyboard. No cursor change.
- tests/ports/safe-save-host.c, safe-save-guest.c: focused probes.

The final normal build also corrects diagnostic recovery-path reporting on a
failed rollback flush (report the restored destination) or failed final commit
flush (report the installed candidate). Host fault tests reran after that change;
Snow success-path evidence predates only that diagnostic wording adjustment.

## Remaining limitations

No atomicity, journal or power-loss safety claim. Concurrent external edits are
not supported; identity/date/length recheck is best effort. Recovery after an
interrupted rename may need manual inspection/rename of the reported siblings.
Real media failure, power loss and physical SE acceptance remain pending.

Nano is still tiny/C-locale/byte oriented. Its existing `%zu` line-count display
shows `zu`; append/prepend normal saves are rejected by this overwrite adapter.
These are documented limitations, not extra features implemented in this task.
No additional milestone started.
