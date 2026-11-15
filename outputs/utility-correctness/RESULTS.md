# Utility correctness — focused batch, 16 November 2026

Host and Snow only; physical SE untouched. Existing checkpoints and unrelated
editor work preserved. No publication or deployment.

## Findings and changes

- Upstream sbase sed searched beyond a requested single occurrence and repeatedly
  rescanned source suffixes and accumulated replacement text. The staged port
  now tracks replacement length, ends a positive-occurrence substitution when
  fulfilled, and polls cancellation during replacement. Vendor source unchanged.
- find/du traversal now allocates one active DFS frame per level; no recursion,
  permanent cache or arbitrary depth-8 limit. Explicit maxdepth and native path
  limits remain. cp/rm received the corresponding validated work below.
- Common native errors have shared descriptions alongside their numeric codes.
  Application build/guest acceptance passed in the follow-up below.

## Actual validation

Host: existing 19 sed comparisons and write/cancel cases; additional 35 boundary,
large-record and empty-match comparisons against pristine upstream; low-memory
and cancellation cleanup. ASan/UBSan traversal depth 24, maxdepth 12, both-fork
space accounting, 26 allocation-failure positions, cancellation and missing path.
31 shell comparisons plus cancellation passed after native-error formatting.

Snow Plus/4 MB/System **6.0.7**, existing normal app 2026111538, external candidates:

| Workload | Result | Guest ticks including loader | Portable-owned peak |
|---|---|---:|---:|
| 64 KiB grep | exact bytes, status 0 | 312 | 208,692 B |
| 64 KiB single substitution | exact bytes, status 0 | 476 | 212,789 B |
| 32 KiB global substitution | exact bytes, status 0 | 5,200 | 180,588 B |
| 64 KiB global substitution | explicit allocation failure, status 1 | 10,108 | 246,124 B |
| subsequent small sed | exact bytes, status 0 | 155 | 12,000 B |
| find, 24 nested directories + file | 26 correct paths, status 0 | 1,666 | not instrumented |
| du of same tree | expected 65 allocation units, status 0 | 1,755 | not instrumented |

All portable allocation survivors are zero; native traversal allocations are
verified by host failure injection and stable guest free heap, not the portable
peak counter. Do not interpret accelerated ticks as physical-SE performance.
Global substitution remains expensive; no broad regex rewrite attempted.

[Captured counters](fixed.txt), [traversal/recovery counters](traverse.txt),
[byte hashes](guest-integrity.json), [host comparisons](sed-host.json).
Screenshots: fixed-unmount2.png and traversal-guest.png.

## Test infrastructure

Use fixed uncapped speed with dynamic fast-forward disabled. Toggling FFWD off/on
produces approximately 15×; captured runs show 15.1–15.4×. Avoid the old key helper
and Snow clipboard injection for long batches at this speed; prepared /etc/profile
scripts execute the fixture without input delivery ambiguity.

Explicit native FlushVol plus PBUnmountVol of the disposable Audit volume made
exports persist and permitted exact offline comparison. Earlier host-only exits
lost catalog changes despite a name-based FlushVol success. Root cause is not yet
proven; no production storage fix claimed. The test-only helper is never pointed
at the boot/user volume. Earlier incomplete exports are not acceptance evidence.

Native errors and pinwheel are accepted below; comprehensive utility coverage and
POSIX compatibility remain incremental work.

## cp/rm and stream follow-up

Normal cp/rm now use heap-backed explicit DFS frames with fixed C stack use.
Existing protected-path checks, symlink handling, both-fork copy, staged per-file
replacement and failure returns remain. Native 255-byte paths and signed catalog
indices still bound operations. Directory operations are not transaction-wide:
failed cp/rm can leave partial trees and return failure, as before.

Portable stdio now uses stable individually allocated cookies (the existing BSD
funopen interface requires stable addresses); the descriptor vector grows and
reuses unused numbers. No native references are opened before ownership storage
is available. Failed fdopen retains the caller's descriptor; successful fdopen
owns it. Cleanup closes streams before leftover descriptors and frees nodes/buffers.
The normal three standard streams require no new node allocations.

Validation:

- `sh tests/utility-correctness/run-host.sh`: ASan/UBSan deep copy/removal,
  allocation boundaries, cancellation and failures; 40 concurrent streams + 40
  descriptors, repeated runs, descriptor ownership, native handle and owned-memory
  recovery. Separate 16-stream/descriptor mode also passes.
- Existing host awk 30 cases and sed 19 cases pass, including write failures;
  elastic sed 35 comparisons and cancellation also passed after the runtime edit.
- Guest resources.sh: 24 nested directories copied and removed; OOM at directory
  frame 12 and cancellation both return correctly, free every frame and recover.
- resources-recovery.sh: injected disk-full write (-34) and cancellation preserve
  the existing destination's complete data/resource forks and Finder metadata.
  No temporary siblings remain. Combined open attempt fails at descriptor 11
  after 16 streams with errno 24 (native EMFILE), status 2 and zero survivors.
- 16 streams then 16 descriptors pass three times, including fdopen ownership.
  Guest peak 13,816 bytes; free heap stays exactly 397,948 bytes over these runs;
  total work 406/404/404 guest ticks (45/42/42 loader ticks), not hardware timings.
- stream-recovery.sh: injected allocator failure status 2 and cancellation 130,
  zero live native allocations and portable survivors, followed by successful reuse.
  Normal rebuilt upstream awk writes 16 simultaneously open files twice, status 0;
  offline comparison verifies every file byte-for-byte. Peaks 33,625 bytes;
  repeated free heap 398,808/398,810 bytes, no accumulating loss.

Native shared newlib retains FILE records for reuse. First larger-stream stress
used about 2.10 KiB beyond the old prewarmed pool; first observed largest free block
changed 388,476→364,500 bytes. All subsequent matched repeats remain stable.
Do not claim all native libc memory is released, or that fragmentation is absent.
No additional idle cache/arena is introduced in Clarice. Native FCB limits remain;
20 tracked temporary names and existing path limits are outside this focused fix.

`python3 tests/utility-correctness/validate-resources.py` verifies exported results.
Evidence: resources.txt, recovery.txt, stream-recovery.txt, corresponding stderr,
resources-validation.json and screenshots. Exact guest scripts/fault helpers are
under tests/utility-correctness; fault helpers are excluded from normal builds.
Host log: resources-host.txt.

Normal local packages in build/: cp.clx and rm.clx are the same 12,840-byte multicall
image, awk.clx is 188,312 bytes. Inputs use existing pinned Retro68/pack-command
scripts; no executable format/runtime ABI change. No rebuilt nano or blanket
bundled-command update, physical testing, push or publication.

## Build 2026111539: errors, inverse activity and shell compatibility

Normal application built with existing Retro68 inputs; activity diagnostics and
nano profiling OFF. MacBinary is 946,176 bytes. No new permanent bitmap/cache;
shell substitution tracking adds two scalars (8 bytes on this target). Native
error strings live in application code; no per-command error-table allocation.

Native errors: [mac-errors.txt](mac-errors.txt) verifies -43, -34, -108 and an unknown
-32000; [actual-error.txt](actual-error.txt) verifies a real missing-file cat error.
Descriptions follow [Inside Macintosh File Manager result codes](https://dev.os9.ca/techpubs/mac/Files/Files-301A.html).
Unknown codes stay numeric; these are not a replacement for upstream errno.

Activity: destination port colours could remap already-final monochrome pixels.
The three save/draw/restore CopyBits operations now temporarily use neutral
black/white colours and restore the original port state. Existing buffer retained.
The diagnostic guest fixture tests four frames × two themes × two port polarities,
inverse background, exact underlying pixels after cancellation and colour-state
restoration. All pass; heap 422,922 bytes before and after. See activity.txt,
activity-guest.png, activity-clock.txt and tests/terminal/activity-guest.inc.
Diagnostic instrumentation is excluded from the normal package.

Shell: focused comparisons found IFS empty-field/joining bugs, assignment-only
substitution-status loss, omitted sh -c arguments, read raw/escape/trim gaps and
prefix assignments restoring unrelated builtin effects. Fixed these in the existing
adapter. Two parser-context fixes prevent an empty assignment eating the following
command and a later `$#` consuming the rest of a quoted line as a comment.
Original mrsh attribution retained. Studied [dash expansion code](https://github.com/herbertx/dash/blob/master/src/expand.c)
for field-splitting semantics; Clarice implementation is independent, not copied.

Validation commands:

```
scripts/test-shell.sh
python3 tests/shell/posix-focused.py
python3 tests/shell/read-overflow.py
```

Existing 31 comparisons and 3 cancellation cases pass with ASan/UBSan. The 39
focused dash comparisons pass; their generated eight guest batches also match
dash byte-for-byte, including exit-status output. Evidence: posix-focused.json,
posix-guest.json, posix-0.out through posix-7.out, posix-guest.png and shell-host.txt.
The generator creates scripts and expected results in work/utility-correctness;
execute each with `sh /mnt/Audit/posix-N.sh > /mnt/Audit/posix-N.out`, then native
flush/unmount Audit before offline exports. All eight batch statuses are zero.
Guest normal build reports 1539, Plus/4 MB/System 6.0.7, 15.6× uncapped FFWD.
No physical-SE or System 6.0.8 result is inferred.

Current-runtime utility regressions: 26 pipeline comparisons, help/cancellation/
write-error/long-line/invalid-option cases; 5 extra file-operand comparisons;
45 tail/sort comparisons, four 360 KB spilling sorts, three spill cancellations,
output failure, same-file sort, tee append and cancellable tail-follow pass.
See pipelines-host.txt. Earlier cp/rm and portable resource fault evidence remains
valid; unrelated full suites and prior successful upstream tests were not repeated.

Remaining: bounded shell variables/source/arguments and one-variable read;
embedded quoted $@ and broader grammar gaps; comprehensive utility compatibility
coverage. No claim of complete POSIX conformance, hardware acceptance or measured
shell speedup. No SE contact, executable cache, push, release or silent bundled
command replacement.
