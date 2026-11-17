# Safe HFS replacement (14 November 2026)

Opt-in `cli_replace_begin`, `cli_replace_commit`, `cli_replace_abort` reuse Clarice's
resolver and native HFS helpers. Ordinary open/O_TRUNC semantics are unchanged.
One save is active per synchronous command invocation. Its allocated context holds
three paths and catalog metadata; a 4096-byte stack buffer copies the resource fork.
Idle state is one pointer, not an arena. Abandoned transactions are aborted after
portable streams close and before terminal ownership is returned.

Begin resolves the destination, rejects locked files/directories, checks source
write permission, refuses existing transaction siblings, creates a same-directory
stage, copies the resource fork, and copies standard/extended Finder information
and creation date. The caller writes the stage data fork through existing stdio.
It must check writing and closing before committing. Nano's finish wrapper does so.

Commit checks source file identity, date and fork lengths (best-effort concurrent
change detection, not a concurrency guarantee), flushes the stage, renames the old
file to a recovery sibling, flushes, renames the stage to the destination and flushes.
The previous file is deleted only after the replacement flush succeeds. Failure
between renames attempts rollback. Failed rollback or cleanup reports retained
native paths. Failure flushing deletion of an already-committed recovery entry
reports a warning: the new file was saved, but removal of the recovery entry may
not have persisted. Native rename refuses existing destinations.

`.clarice-XXXXXXXX.new/.old` are deterministic siblings, intentionally outside
/tmp so they share the volume and survive temporary-directory cleanup. Hash/name
collisions stop saving rather than overwriting another file. Siblings from an
interrupted save block retry and require inspection. No startup auto-deletion.

Recovery: inspect the reported `native:` files. If the intended destination is
absent, rename the intact `.old` file back with `mv`, quoting paths with spaces.
If a destination exists, first retain it under a different unused name before
restoring `.old`. Never remove both copies to clear a warning. Finder can perform
the same rename operations. There is no claim of atomic replacement, journalled
recovery, crash consistency or power-loss safety; real hardware remains pending.

Nano 9.2 tiny uses this service only for normal overwrite/new-file saves. The
small idempotent `scripts/patch-nano-save.py` adapts write_file in the prepared
upstream tree; build entry point applies it. Emergency-exclusive-file handling is
unchanged; append/prepend normal saves are explicitly unsupported by this tiny
adapter. The source remains GNU nano, with upstream copyrights and GPL preserved.
New required imports fail clearly on older Clarice builds. This is not a new port
or a full public SDK. A shared declaration header is in src/ports/safe-save.h.

Plus testing exposed missing Control-O/X on the original keyboard. Existing
Option-key normalization now maps Option-O/X to editor save/exit while the editor
owns input. SE Control keys remain unchanged. No cursor/rendering changes.
