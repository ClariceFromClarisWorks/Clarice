# ls formatter and catalog-request follow-up — 18 November 2026

Worktree `Clarice-Ls-Asm`, branch `codex/ls-asm-investigation-20261118`, based on
6a0546a. User-authorised follow-up to the assembly investigation. Main and the
separate CLX memory/policy worktree remain untouched. No push or release.

## Candidate changes

- Keep C and the existing inline 68000 DIVU. Once the remaining fork size fits
  16 bits, use a shorter conversion loop without repeating high-word work.
- Use the already available runtime1.10 `cli_resolve_info` to reuse metadata from
  the same path-resolution operation. Ordinary `/tmp/file` takes two metadata
  requests instead of three; there is no retained pathname/catalog cache.
- For name-only `ls -a`, skip parent metadata entirely. Detailed listings still
  resolve and display parent metadata, using the same-operation interface.
- Preserve virtual `/mnt`, including when its backing folder is absent. On a
  missing-metadata result only, use the old creation-capable resolver to establish
  that exact virtual path; never consume metadata left by a failed call.
- Declare runtime1.10 in the normal core-utility packaging command, because that
  is when the reused service became available. Older cores reject the package
  rather than encountering an undeclared missing import. No runtime ABI change.

Native indexed enumeration remains one PBGetCatInfo request per entry plus EOF.
Explicit volume, parent ID and index restoration, hidden-entry handling, malformed
name rejection, streaming output and cancellation are preserved. No new command,
process backend, cache, asynchronous lifetime, HFS writer or directory array.

## Catalog alternatives considered

The existing [catalog](../catalog-perf/REPORT.md) and
[native HFS](../hfs-cost/REPORT.md) investigations already establish the dominant
File Manager cost and compare synchronous/asynchronous requests. Apple
[FL31](https://leopard-adc.pepas.com/technotes/fl/fl_31.html) and MoreFiles' indexed
walk are the relevant established approach. Reuse this evidence instead of
reimplementing a catalog walker.

| Alternative | Assessment |
| --- | --- |
| Reuse the parameter block | Already implemented; do not claim it again. |
| Remove per-entry stat calls | There are none: the returned catalog record supplies the displayed metadata. |
| Async GetCatInfo | Earlier local HFS calls completed synchronously; no measured overlap or speed gain. |
| System 7 CatSearch/bulk APIs | Not a System 6 baseline replacement; a separate capability-gated backend is outside this bounded change. |
| Stop at initial directory entry count | Can truncate a listing when other applications modify the directory; retain EOF enumeration. |
| Raw catalog access, persistent cache or larger system disk cache | No demonstrated redundant reads; adds invalidation/compatibility obligations or changes global system policy. Not introduced. |
| Reuse metadata already fetched by path resolution | Demonstrated redundant request; existing runtime service, no allocations or persistent state. Implemented in candidate. |
| Avoid unused parent metadata in name-only output | Demonstrated unused work; skip the lookup without changing names or traversal. Implemented in candidate. |

The selected changes save setup work per operand. They do not eliminate the
per-entry HFS bottleneck and should not be presented as a radically faster catalog
algorithm. The formatter change applies to normal detailed sizes, not `-h` or
name-only output.

## Code and memory costs

Matched packages all use the same format2/runtime1.10 header:

| Variant | Package bytes | Loaded image bytes | Relocations | Imports |
| --- | ---: | ---: | ---: | ---: |
| Baseline | 12620 | 8104 | 175 | 37 |
| Formatter only | 12640 | 8124 | 175 | 37 |
| Formatter + catalog setup | 12744 | 8164 | 175 | 38 |

Combined delta: +124 bytes on disk and +60 bytes in the command image, plus the
additional 64-byte import record during loading (124 bytes combined payload
increase at that stage, before allocator overhead). The listing stack-frame
reservations and saved-register totals are unchanged in the compiled objects.
The old normal build used a format1
header; compared with that 12604-byte package, the properly declared candidate
is +140 bytes including its 16-byte larger header. No application partition
increase or new idle allocation/cache. No heap allocation in either changed
algorithm. [Package details](package-costs.json), [native build log](build.log).

## Functional validation

- ASan/UBSan runs the actual resolver and listing loop together: 176 matched
  baseline/candidate cases, identical statuses/names/metadata. Covers absolute,
  relative, root, dot/dotdot, links/cycles, volumes, missing paths, I/O errors and
  virtual `/mnt` with and without a backing folder. Successful ordinary paths
  never increase metadata calls. [Request counts](resolver-host.txt).
- Actual production listing-loop mocks pass explicit volumes, overwritten parent
  IDs, mixed files/directories, hidden entries, escaping, 31-byte/malformed names,
  EOF, errors, cancellation and virtual mounts.
- 4,000 formatter cases match an independent printf oracle across full 32-bit
  sizes, dates, directories, human/full output and cache invalidation.
  [Output](format-host.txt).
- Existing operand tests pass 130 operands, argv preservation, option errors,
  defaults and cancellation; empty-path tests reject without catalog I/O.
  [Operands](operands-host.txt), [empty paths](empty-path-host.txt).
- Pinned Retro68 `-m68000 -Os` compilation passes for all three complete utility
  packages. All imports exist in the runtime export table; package headers declare
  runtime1.10. No full application rebuild was necessary.
- The earlier [CPU test](../ls-asm/REPORT.md) exercised the same specialised
  68000 arithmetic on 75,567 inputs. It is functional emulation, not cycle timing.

## Guest measurement method

Disposable cloned Snow Plus, 4 MiB, System6.0.8 media, Core2026111882/runtime1.10.
Normal emulation speed for measured runs. Utilities installed only beneath
`/tmp/ls-base`, `/tmp/ls-frmt`, `/tmp/ls-next`; standard `/bin` unchanged.
Two HFS volumes mounted; fixtures live on the startup volume beneath Clarice Data.
100/1,000 files named f0000 onward, repeating data lengths0/1/8192/32768 and empty
resource forks. Identical metadata and catalog order for each variant.

The built-in `time` measures loading, dispatch and command execution, including
output work during the command. It **does not include the subsequent final prompt
presentation**. Screen output runs inside a shell script, so they are not claimed
as direct interactive submission-to-final-prompt acceptance. First package
invocations and repeated samples are separated; the sequential first samples
are not equivalent cold-disk tests. Alternating order mitigates session drift.

A long keyboard-entered continuation lost characters; that attempt is excluded.
The earlier valid partial log is retained as [initial evidence](initial-partial.txt),
not mixed into the final scripted comparison. The final run uses an on-disk shell
script to avoid typing loss. Remote services are disabled for measurements.

The first scripted run reached its completion markers, but its offline extraction
failed part-way through the timing file. Those bytes are retained as
[unaccepted partial evidence](unflushed-results-partial.txt), not used for speed
claims. The application was on GlyphFull while logs were on RCGuest; quitting the
application is not a substitute for flushing that explicit volume before stopping
the emulator. The replay starts from the pre-test image and uses a 432-byte,
unbundled native FlushVol probe for the resolved evidence volume, outside all
timed regions. Its status is recorded, then it flushes once more after the status
write. No production durability code or OS cache policy is changed.

Reproduction sources: `tests/ls-catalog/build.sh`, `benchmark.sh`,
`flush.c`, `resolve.py`, `summarize.py`. Build only the utility variants;
install them on offline disposable media, then run the scripts locally. Never
mount or edit an image while Snow has it open. The temporary private media were
used because the cloned emulator's file picker lacked Documents access; no host
permission or security setting was weakened.

## Measured result and recommendation

Retain the bounded formatter and catalog-setup changes in this worktree. The
normal-speed Snow evidence supports a modest gain, especially for small listings;
it does not justify an assembly rewrite or a replacement catalog implementation.

Repeated-run medians, seconds from guest TickCount/60 (three samples each):

| Workload | Baseline | Formatter only | Combined |
| --- | ---: | ---: | ---: |
| 100 files, redirected `ls -la` | 2.40 | 2.33 | 2.30 |
| 1,000 files, redirected `ls -la` | 11.51 | 11.10 | 10.16 |
| 100 files, terminal `ls -la` | 2.35 | 2.33 | 2.21 |
| 1,000 files, terminal `ls -la` | 9.88 | 9.66 | 9.76 |
| 100 files, redirected `ls -a1` | 2.30 | — | 2.21 |

Combined terminal medians improve 6.0% / 1.2% for 100 / 1,000 files. The larger
redirected runs vary substantially: baseline10.21–11.70, formatter9.70–11.46,
combined9.60–11.60. Do not advertise their 11.7% median difference as a reliable
catalog or arithmetic speedup. Combined is also not uniformly faster than the
formatter-only variant on large listings. Request-count reductions are the more
precise evidence for the catalog change; physical matched trials remain required.

First runs within the redirected fixtures, baseline/formatter/combined:
100 files2.46/2.45/2.41;1,000 files10.61/9.70/10.00. Only the100-file runs are
first package invocations after boot; all run sequentially and share OS caches.
No equivalent cold-start comparison is claimed.

**48 timed invocations returned0; every invocation reports owned-peak0 and
remaining0.** These counters exclude the loader's image/metadata and the parent
shell/terminal. Within the redirected phase free heap is353912–353954B; within
the screen phase329628–329652B, and name-only329730–329752B. The level changes
between phases, so this is not an assertion of identical whole-application idle
memory. There is no progressive per-variant loss within these samples; static
command-image/import costs are reported separately above.

All16 in-guest detailed-output comparisons pass. Offline extraction after the
explicit volume flush succeeds: all three final detailed outputs match byte for
byte (49,091 bytes,1,002 rows), with all1,000 distinct names and expected data/
resource sizes. Name-only outputs match102 expected names. Independent real
Ctrl+C interrupted the candidate after f0116, restored the prompt, returned130,
and reported remaining0. [Cancellation evidence](cancel.txt).

[Raw final log](guest-results.txt), [all samples](measurements.json),
[summary](summary.txt), [output validation and first runs](output-validation.json).
The replay records flush:0 and reached ALL-LISTING-TESTS-DONE-FLUSHED. Clarice was
quit and the private Snow workspace closed before extracting the files. The
standard installed utilities, other worktrees and physical machine were untouched.

## Next catalog-request opportunity

`resolve_full` still calls `native_path` and `info` per path component; explicit
`/mnt/volume/...` resolution performs repeated volume-name scans in
`volume_unique` and `cli_native_volume`. The selected patch removes the redundant
final `info` and its volume scan, but does not redesign this shared resolver.
A next focused experiment could count those volume requests for shallow/deep
operands and pass an explicit volume reference within one resolution operation.
Preserve duplicate/missing-volume rejection and do not retain it across commands.
This would affect more than ls and needs its own matched measurement and regression
scope. No speculative shared-runtime cache was added here.

## Acceptance boundary

Physical SE measurements remain required. No physical deployment, System7 or
MultiFinder execution claim, final-prompt timing claim or full idle-memory
acceptance is implied by the focused host/guest tests. Keep the independent
packaging and execution policy from the user's earlier revision.
