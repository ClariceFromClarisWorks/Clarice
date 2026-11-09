# Utility validation progress — 9 November 2026

Plus, 4 MiB, System 6.0.8 / MultiFinder, project-local Snow. Not real hardware.

The first combined run completed pipeline, awk-format and help checks, but
file/inspection tests cascaded from a mkdir -p failure. Keep guest-first-run.txt
as evidence of that failure, not a passing transcript. Intermediate
guest-files.txt also failed because an earlier touch error created the missing
parent as a regular file. Both defects are now fixed: mkdir creates components
in order; touch never creates from a partially resolved failed pathname.

The clean rerun in guest-final.txt verifies mkdir -p, untyped new touch files,
metadata preservation, hfsattr, both-fork copy and replacement, rename and
same-name directory moves, symlinks/readlink, locked-file refusal, recursive
copy/removal, find/du/file/od/basename/dirname and redirected more. Negative
cases deliberately return errors for nonempty rmdir, self/descendant copy,
missing parent, and chmod (127 plus hfsattr hint). Missing-parent touch now
leaves the parent absent.

A guest copy interruption returned 130. Extracted destination data and resource
forks remain byte-identical to the original fixture after copy, rename, move
and interrupted replacement. The evidence does not pinpoint which copy phase
was interrupted. No staging file remained in the destination directory.

The first run's remaining sections verify disk-spilling sort, numeric/unique
sort, tail, tee pipelines, repeated port allocation release (remaining=0), all
external --help exits, builtin help, and corrected awk integer formatting.

Final release gates now pass on the Plus guest: `more` Q returns 0, `more`
Ctrl+C returns 130, and Ctrl+C interrupts sorting the 600,000-byte / 20,000-line
fixture with status 130. The workload exceeds the 128-line chunk bound; the
command was allowed to run before interruption. An offline HFS directory
inspection after clean exit finds no `clarice-sort-` temporary files. Exact
statuses and the complete temporary-directory listing accompany this report.
No claim is made about cancellation at every possible instruction or disk error.

Release documentation now describes the 768 KiB partition, source-port limits,
actual utility options, upstream credits, and the next priority: nano before
serial work. Existing host fault-injection tests cover allocation/output failure
and temporary cleanup; guest checks cover the native HFS and input paths.
