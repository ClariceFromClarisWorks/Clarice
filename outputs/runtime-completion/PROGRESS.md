# Terminal/runtime completion progress

## 1. Terminal usability — focused checks pass

Snow Plus/4 MB/System 6.0.8 baseline confirmed Command-C, idle Command-V,
ordered typing during foreground work and launch in front of Finder already work.
No launch or cursor-rendering changes made.

Fixed busy-command paste (atomic insertion through the existing bounded queue),
editor ownership handoff of pending input, and inverse-colour activity overlay.
Paste newlines remain literal until explicit Return. Queue overflow cancels and
blocks submission. Paste into an existing interactive owner is refused rather
than routed into the shell behind it. Serial's existing ownership is preserved.
No new permanent queue, cache or drawing path.

Guest checks: 3 failures before, 0 after, including new newline/no-auto-execution
and overflow checks: usability-before.txt / usability-after.txt. Diagnostic build
free heap 407,662 before / 406,832 after (different probe code; not a production
idle-allocation comparison). Host ASan/UBSan paste, serial queue, selection and
wrapped-input regressions pass. Input drawing/cursor algorithms unchanged.

Next: separate loader event overhead from checksum/validation, then optimise only
measured costs. No push/publication or hardware test.

## 2. Loader — measured targeted improvement

Same Snow Plus/4 MB/6.0.8 disk and 103,300-byte demo image:

| Loader ticks | Baseline first / repeats | Optimized first / repeats |
|---|---|---|
| Total | 342 / 283, 287 | 293 / 230, 229 |
| Read | 34 / 34, 34 | 35 / 35, 34 |
| Validation | 200 / 200, 204 | 175 / 175, 176 |
| Imports | 11 / 10, 10 | 11 / 11, 10 |
| Relocations | 38 / 39, 38 | 9 / 9, 9 |
| Preparation | 59 / 0, 1 | 63 / 0, 0 |
| Events (included above) | 4 / 5, 10 | 20 / 16, 20 |

Exact FNV-1a checksum loop unrolled eight bytes, no table/cache. Even-aligned
68000 word/long accesses replace four-byte assembly in loader metadata accesses.
All format, checksum, import and relocation checks retained. Host loader/install
fault suite: 41 scenarios. Actual 68000 checksum: 2,028 reference cases including
odd addresses, boundaries and register preservation. See checksum.txt and
loader-integrity.txt. Timing source records: loadbefore1–3 / loadafter1–3.txt.
An initial optimized run produced no report and was excluded; the successful
retry explicitly verified the guest prompt before submission.

Repeat loading is about 3.8 seconds, versus 4.7–4.8; first load 4.9 vs 5.7.
This is loader time, NOT nano submission-to-editable-screen acceptance. Checksum
still dominates. No event-poll reduction, integrity bypass or persistent cache.

## 3. Adaptive portable allocations — focused checks pass

Removed shared portable runtime's arbitrary 96 KiB owned-allocation ceiling.
Native allocator checks available application heap, leaving a 32 KiB error/UI/
cleanup margin (separate from the existing 64 KiB stack reserve). No reserved
arena, new persistent buffer or idle allocation. System memory outside Clarice's
application partition is not made available by this change. Allocation can still
fail from fragmentation. Released regex scratch is reclaimed on allocation failure.
Size/accounting overflow is rejected; failed realloc preserves the original.
Invocation entry resets counters; exit/cancellation still releases owned resources.

Shared getline grows according to demand instead of stopping at 16 KiB, with
size overflow checks. Existing awk-specific limits, descriptor/stream counts and
CLX image-validation bounds remain; these require separate consumer evidence.

Host ASan/UBSan: memory pressure/realloc/recovery probe; grep 26 comparisons plus
40 buffer-boundary cases; sed 19 comparisons; pipeline 26 comparisons plus file,
long-line, cancellation and error cases; awk 30 comparisons plus failure cases.
Allocation-failure tests now inject a finite host budget instead of assuming a
production 96 KiB ceiling.

Snow: memory1.txt and memory2.txt both PASS/status 0; peak 346,080 bytes,
remaining=0, identical free heap 402,310 and largest block 398,058. A third
scheduled invocation was stopped before submission completed and is NOT counted.
These two runs used the memory-only revision before subsequent stream fixes.

## 4. Portable file services — partial; failed-save blocker

Fixed update streams enabling both directions, standard binary/update mode order,
seek callbacks for native files and owned descriptors, and preservation of native
I/O errno (e.g. ENOSPC instead of generic EIO). Added fread/fseek to the required-
import lookup for actual portable stream consumers. Existing images remain valid;
new images requiring these symbols fail clearly on older hosts.
No general directory API added: nano tiny's browser is disabled, and no current
consumer needing it was established.

Host stream-services probe passes binary update, SEEK_SET/SEEK_END, read/write
transitions, fdopen ownership, close/reopen, exact contents. Guest evidence follows.

**Stopping condition:** existing nano tiny save is not recoverable on write
failure. work/nano/config/config.h enables NANO_TINY. Upstream files.c:1804 opens
normal saves with O_TRUNC; backup and fsync sections are excluded by NANO_TINY.
The focused host probe uses that same open/fdopen/write/close sequence through the
actual portable runtime. Injected first-write ENOSPC returns status 2 with the
correct error but leaves the original data fork empty (failed-save.json).
This is a host reproduction of the save sequence, not a guest nano acceptance.

Do not change POSIX O_TRUNC globally. Recommended decision: explicit reusable
staged-save/commit/abort service, with sibling file, preserved resource fork and
Finder metadata, recoverable previous version, and error/cancellation integration
at nano's save boundary. Existing command replacement offers reusable patterns,
but is not a general document transaction. HFS rename/flush failure recovery must
be specified and fault-tested before calling editor saves safe. Stop here under
the user's unresolved data-loss rule; no nano source or save policy changed.

### Final normal-build guest evidence

Snow Plus/4 MB/System 6.0.8: stream-guest.txt PASS/status 0, peak 2,128 bytes,
remaining=0. Binary update modes, seek/read/write, fdopen ownership and reopen
produced exactly `abXYef`. After normal Clarice quit and confirmed Snow exit,
MacBinary extraction verified identical resource fork, type/creator, Finder flags,
and creation time (metadata.json). Location/folder bytes were zeroed by the
hfsutils import/export control even without guest execution; nonzero location
preservation is therefore not claimed. Modification time
is expected to change. This is successful data-fork editing, not failed-save safety.

memory-final.txt repeats the demand/exhaustion/realloc/recovery probe against the
final normal application/runtime: PASS/status 0, peak 346,080 bytes, remaining=0.
Final post-command free heap is 400,394 bytes for both stream and memory probes.
Earlier loader baseline free heap was 402,408: about 2 KiB lower in the final build
(including linked stream/seek code); no new persistent arena or queue. Largest
block is not directly comparable across differing probes/FILE-pool preparation.

Final build: build/Clarice.bin and build/SHA256SUMS.txt; ordinary diagnostics-off
68000 build. Existing upstream objects relinked with the new runtime; bundled
resources regenerated. Existing guest /bin commands remain protected by Task 2's
install-if-absent policy. The guest probes explicitly used newly built images;
updated grep/sed/awk/pipeline ports have host coverage, not a claim that every
bundled utility was rerun in Snow. Final loader/install host suite: 41 scenarios.

Changed implementation: src/app.c, activity.inc, editor-screen.inc (usability);
checksum.S, module.inc, shell.c (loader); port-files.inc, ports/runtime.c
(allocation/streams); optional diagnostic switch in CMakeLists.txt. Focused probes
in tests/terminal/usability-guest.inc and tests/ports/{memory-demand,
stream-services,failed-save}.c; existing paste/runtime tests updated. Existing
serial/editor/QOL work preserved. No new directory subsystem or application port.
Physical SE checks remain in outputs/PENDING-HARDWARE-TESTS.md. No push/publication.
