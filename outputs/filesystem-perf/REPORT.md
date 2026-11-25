# Batch 3C — HFS traversal performance

Build 2026111968, product0ab807e, based on accepted1967/0f82946. Both focused changes retained.
Snow Plus, 68000, 4 MiB, System 6.0.8; maximum acceleration, no hardware access.

## Changes and measurements

cp/rm retain the explicit volume and directory ID already obtained for each active
directory, rather than resolving its native pathname for every indexed entry.
The parameter block is reset each call because HFS overwrites returned fields.
This follows the existing find/du traversal and Apple's documented
[directory enumeration](https://leopard-adc.pepas.com/documentation/mac/Files/Files-94.html)
and [PBGetCatInfo contract](https://leopard-adc.pepas.com/documentation/mac/Files/Files-238.html).
Per-child resolution, symlink guards, fork copying and metadata handling remain.
find/du process nondirectory entries directly, preserving their cancellation poll
and output checks without allocating a traversal frame that is immediately freed.

Three interleaved normal-build pairs per fixture, order reversed in the middle
pair. Same source/destination names, 10/100/1,000 files across 2/10/10 directories;
1 KiB binary data files and one two-fork resource-bearing file per directory.
Output redirected to /dev/null and copy/removal progress disabled. Median command
execution ticks, excluding separately recorded CLX loading:

| Files | find before → after | du | cp -Rp | rm -R |
|---|---:|---:|---:|---:|
| 10 | 29 → 23 | 28 → 24 | 845 → 810 | 259 → 239 |
| 100 | 174 → 131 | 179 → 129 | 8,040 → 7,744 | 2,427 → 2,234 |
| 1,000 | 1,117 → 683 | 1,087 → 636 | 96,783 → 93,231 | 26,755 → 23,901 |

Large-fixture reductions: **38.9%, 41.5%, 3.7%, 10.7%** respectively. Including
CLX loading, medians are 1,171→737, 1,135→682, 96,846→93,296 and 26,828→23,980
ticks. Raw outer `time`, loader and native-heap observations are in pairs.json;
summary.json separates loading/execution. No median regression in these fixtures.

Scoped native enumeration catalog calls for cp/rm fall 30→15, 242→121 and
2,042→1,021. Corresponding volume scans fall 30/242/2,042→0. These counters exclude
per-child resolver/copy internals; a volume scan can issue several HGetVInfo calls.
find/du retain 15/121/1,021 enumeration calls but eliminate 10/100/1,000 leaf
allocations. Large traversal frames: 1,011→11, requested payload 547,962→5,962
bytes; peak simultaneously live frame payload 1,626→1,084 bytes, final live=0.

## Responsiveness, memory and correctness

- Poll counts are unchanged: large find 2,021, du 2,022, cp 3,203, rm 1,023.
  Repeated final find/du maximum intervals between polls were 9–10 guest ticks,
  matching baseline 9–10. Earlier cp/rm profiles showed 71→72 and 43→41 ticks:
  native copy/catalog operations still bound worst-case service latency.
- All 12 focused remote cancellations returned130, then executed subsequent
  commands. Corrected host-to-returned-prompt measurements: baseline instrumented
  7–27 ms, candidate instrumented 7–73 ms, normal candidate 5–14 ms. These are
  accelerated host/network observations, not SE latency guarantees. The initial
  timing read matched echoed redirection; cancellation.json supersedes those
  discarded timings. Local Ctrl+C and Command-period also returned130 with a
  restored prompt (cancel.png).
- 55 guest acceptance records cover matching find/du options and output, root
  files, missing paths, encoded names, file/dangling links, recursive copy/removal,
  both forks and Finder metadata, cancellation and partial-tree cleanup. Three
  batches of ten find/du pairs ended at exactly heap320,722/largest320,554,
  remaining=0. The 72 normal benchmark runs showed no progressive native loss.
  Direct traversal allocations are measured separately from runtime-owned counts.
- Host ASan/UBSan enumerator bounds and overwritten-result tests, 25 deep-frame
  allocation boundaries, catalog faults/retry, cancellation and output failure
  pass. Prior symlink/path-boundary checks and 232 native copy fault cases pass;
  data/resource forks, metadata and source-on-failure safeguards are unchanged.
- Core MacBinary remains438,656 bytes. Shared files CLX20,792→21,084 (+292),
  inspect13,928→14,284 (+356); BSS unchanged. CopyFrame1,544→1,038 bytes;
  RemoveFrame1,028→1,034; measured two-level peaks3,088→2,076 and2,056→2,068.
  No persistent allocation, cache, runtime/ABI change or startup work.

## Measurement limits and distribution

Guest ticks include emulated OS/event/I/O time, not pure CPU time. CPU calibration
at normal pacing gave85/166/316 ticks for100k/200k/400k iterations, versus
85/166/319–320 at maximum pacing. This supports comparative timing, not physical
SE equivalence. Initial separately run profiles were sensitive to catalog/cache
state (including a slower candidate copy); retention relies on the three matched
normal pairs. after.json also predates removal of the now-unused CopyFrame path;
final.json/final-check.json record the final smaller frame. A fresh n10 copy/tree
comparison in final-check.json confirms the matched two-level footprint.

Normal full build, loader import validation and sealed release checks pass.
Offline HDA Core/installer/helper forks and Finder metadata, Release and49 tools,
plus all51 USTAR members match sealed inputs. HTTP/AFP publication verification
passed for all51 manifest entries/36 payloads, source, seal and offline packages
(verification.json). Previous1967 recovery/distribution and unrelated
work are preserved. Physical acceptance pending; no GitHub push. Stop after3C.
