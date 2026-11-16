# 0.5.0 bounded performance audit — 16 November 2026

Snow-only development acceptance, Core 2026111550 unchanged. No physical-SE,
System 6.0.8 or release acceptance. Retain the focused utility changes and proceed
to consolidated hardware acceptance rather than another optimisation campaign.

## Method and coverage

Snow v1.5.0-a921b10, Plus/68000, 4 MB, reported System 6.0.7, maximum FastForward
(no CPU overclock). Measured Macintosh guest ticks, not host elapsed time.
`clock.c` gives identical 66/132/264 ticks and checksums at normal/max pacing for
100k/200k/400k CPU iterations. Snow compact-bus source advances video/VBL from CPU
cycles; FastForward changes host pacing. TickCount is coarse (~60 Hz): repeat
nontrivial complete operations, avoid attributing one-tick differences. Event
service and HFS costs remain included. Accelerated asynchronous network timing
is not reliable evidence of physical throughput.

Inputs: 504 B/8 lines, 8064 B/128 lines, 64512 B/1024 lines; 65536 B binary;
directories of 10/100 files. Three executions per case. Small per-case scripts
avoid holding a large benchmark AST throughout each command. The rejected initial
monolithic script held about 320 KiB and induced allocation failures with only
78 KiB available; it is not the performance baseline. Corrected runs leave about
391 KiB available and all measured cases succeed.

Baseline.txt and after.txt cover cat, grep, sed, awk, sort, cut, tr, wc, cmp,
cksum, cp, ls, find, du, df, dd; then head, tail, uniq, tee, od, file, more, mv,
rm. Small.txt exercises mkdir, touch, ln/readlink, hfsattr, basename/dirname,
free, printenv, mktemp, sleep 0, rm/rmdir. Shell workload combines functions,
quoting, command substitution, variables and test; pipeline is cat|grep|wc with
redirection. Tiny commands receive grouped workflow coverage, not meaningless
exhaustive microbenchmarks. appinstall/Core setup retain the accepted distribution
and installer evidence. serial, kermit, wget and update retain prior functional
evidence: no fresh asynchronous transfer benchmark is claimed. Diagnostic
benchscan/benchls and optional nano are not new optimisation targets.

`time` measures the outer command; runtime ticks describe only the most recent
CLX module. They are stale for builtin-only scripts and cover only the last stage
of a pipeline. Loader stage/event counters overlap: do not sum them as disjoint
phases. Post-load subtraction includes processing, runtime, filesystem and event
work, not pure CPU. First invocation may verify a newly changed executable; it is
not a cold disk-cache measurement. No renderer or loader policy changed.

## Retained improvements

| Workload | Before guest ticks | Final guest ticks | Reduction |
|---|---:|---:|---:|
| wc, 64512 B text | 2983 | 763 | 74.4% |
| tr a-z A-Z, same text | 4434 | 2687 | 39.4% |
| find, 100 files, alternating old/new | 2098 | 159 | 92.4% |
| du, same directory | 2095 | 146 | 93.0% |

Warm representative medians; raw repeats and loader breakdowns are preserved.
Final wc runs 854/763/763 (load109/43/45); tr2770/2687/2686
(load127/51/47). Final paired find221/159/156, old2116/2098/2097;
du189/146/146, old2095/2099/2095. Changed-image verification explains much of the
first-run premium. Candidate small/ordinary wc86→69 and429→151; tr93→79 and606→387.
Final production layout slightly improves rather than reverses candidate results.

1. **Shared upstream UTF helpers:** ASCII bytes need no general rune decoder,
   encoder or Unicode whitespace binary search. Isolated generated-source patches
   in prepare-utf.py preserve the pinned sbase originals/notices. General Unicode
   and malformed input paths remain unchanged; exact ASCII whitespace includes
   upstream's 0x1c–0x1f. No new cache, table or assembly.
2. **Native find/du traversal:** reuse indexed catalog metadata for child entries;
   keep explicit native volume/directory IDs and call the same PBGetCatInfoSync
   operation already used by ls. Previously each entry re-resolved both parent
   and child paths. First metadata-reuse experiment cut ~53%; eliminating the
   repeated parent lookup yields the final result above. No new CLX ABI/import,
   filesystem subsystem, retained cache or recursive implementation. Each PB is
   freshly populated because HFS overwrites result fields. Zero-volume selectors
   are rejected rather than falling back to the boot volume.

## Memory, size and correctness

No permanent allocation or idle Core change. wc peak4208 B, tr3668 B unchanged;
command-tracked remainder zero. Two final five-repeat batches show free heap
388686/388688 B, largest block387750 B: no monotonic decline. Catalog traversal
removes a 512-byte native path per frame, adding only volume/ID fields (about
504 B less per active depth); root resolution uses a transient 512-byte stack
buffer, no recursive stack growth. Native traversal allocations are not covered
by the upstream-owned peak counter; host fault tests explicitly count every frame.

CLX bytes: wc15428→15496 (+68); tr32332→32456 (+124);
shared find/du/file image9780→10152 (+372 per installed copy). Other pipeline
production images remain identical. Core was not rebuilt.

Passed:
- ASan/UBSan differential UTF helper checks, all rune whitespace values, ASCII,
  malformed/truncated/binary input and output return conventions.
- Existing 26 upstream pipeline comparisons; 5 help,5 cancellation,5 write-error,
  3 long-line,4 invalid-option plus5 file-operand/output cases. Allocation budgets
  0/512/1600 fail cleanly; larger budgets and subsequent runs recover, no survivors.
- Native traversal mocks: both volumes, raw versus encoded names, nested dirs,
  links, fork sizes, overwritten PB fields, every allocation failure point,
  cancellation, catalog errors and zero/missing-volume rejection. Existing QOL-1
  namespace regression passes, including rejection of historical zero-vRef fallback.
- Final Snow old/new find/du byte-identical output: nested tree, space/slash/percent
  names, links, relative paths and copied boot-volume tree. Invalid volume status1
  with error-35. Both forks, type/creator, flags and creation date fixture preserved.
- Final Snow wc/tr UTF/binary output equals unchanged upstream host reference.
  Streaming tr interruption status130, remainder0; repeated production runs succeed.
- Final native traversal loop returned to prompt via Command-period. Automated
  Control-C reached this guest as plain c; a reliable follow-up status/heap export
  was not obtained. Do not claim that final keyboard-Ctrl+C/heap check passed.
  Host cancellation/cleanup passes; physical keyboard acceptance remains required.

An incomplete final acceptance export was discarded and rerun separately; the
replacement report ends ACCEPT_DONE with all expected statuses. The initial
baseline's post-report unmount returned -47 because a report was still open;
all command cases completed, but that footer is not a successful unmount. Export
only after reports close and the disposable volume is safely flushed/unmounted.

## Remaining costs and next step

Baseline warm total/load ticks for large input: cat57/28, cp169/40,
cksum174/28, cut341/42, cmp541/28, awk832/82, grep1128/55,
sort2870/57, sed2923/61. ls100files99/46; df40/39;
dd64KiB105/40. cat/cp/cksum are already comparatively economical. sed regex work
and sort's bounded disk merge are now prominent; do not mislabel disk spooling as
CPU or replace faithful upstream algorithms without separate evidence. Shell's
10 command-substitution iterations take ~11.9 guest seconds; spool/dispatch work
merits later focused study, not an unmeasured execution-model rewrite here.
Small utilities are often dominated by ~28–61 load ticks; no cache introduced.

No material regression observed in measured affected workloads. Exhaustive utility
performance, native wall-time breakdowns, idle-system measurement and real-network
throughput are not claimed. The accepted low-memory/runtime architecture remains.
Use the consolidated section in PENDING-HARDWARE-TESTS.md to validate matched old/
new binaries, actual cancellation, both forks, repeated heap recovery, ordinary
pipelines and visible terminal responsiveness on stock SE/System6.0.8 before release.
