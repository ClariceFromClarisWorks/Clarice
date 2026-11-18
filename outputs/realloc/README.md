# Native realloc investigation — 18 November 2026

Worktree: Clarice-Busybox-Additions; branch codex/busybox-additions-20261118.
User-authorized follow-up to f1d9e3b. User subsequently authorized committing and pushing this separate branch. No Main merge, physical SE work or release.

## Failure established

The instrumented copy-only runtime reproduces the 256 KiB changed-text failure:

| At the failed allocation | Bytes |
|---|---:|
| Existing realloc payload | 43500 |
| Requested payload | 47844 |
| Growth delta | 4344 |
| Live invocation-owned allocation | 244088 |
| Earlier peak owned allocation | 290944 |
| Native free heap | 91884 |
| Largest contiguous free block | 39580 |
| PurgeSpace potential free / contiguous | 91884 / 39588 |

The replacement needs its full payload **plus the allocation header**, while the
old block is still live. The free total covers the request and unchanged 32768-byte
recovery margin, but the largest block cannot fit it. This establishes a
contiguous-space failure, rather than an arbitrary 256 KiB limit or a refusal to
use elastic allocation. Native allocation had already attempted reclamation.
The failure-point snapshot uses existing `cli_memory`; no successful allocation
is burdened with a full memory snapshot.

The 43500→47844 sequence matches upstream diff's candidate-list growth:
3625→3987 records of three 32-bit integers, growing by 10%. Diff had already
performed 62 realloc copies, moving 761832 bytes. The 256 KiB input has 8192
32-byte lines, repeated in groups of 256; there are two input files, and the
changed version alters one line per group. This is not a universal file-size
threshold. Repeated lines also affect the matching workload.

Baseline: 83.07 host seconds, 7815 guest module ticks (130.25 seconds), status 2.
After cleanup owned allocations are zero; an 8 KiB comparison succeeds.
[Raw baseline](baseline.json).

## Small native alternative

Apple's documented `SetPtrSize` changes the size of an owned nonrelocatable
block without changing its address. Growing may fail when another nonrelocatable
block prevents expansion; copy fallback remains necessary. References:
[Inside Macintosh II, Memory Manager](https://puyoman.net/documentation/Inside_Macintosh.pdf),
[Inside Macintosh: Memory](https://www.vintageapple.org/inside_r/pdf/Memory_1992.pdf),
and the pinned Retro68 Multiversal Memory Manager declarations.

The candidate retains upstream diff unchanged and opts its invocation runtime
into `PORT_NATIVE_RESIZE`. The small classic Mac binding uses SetPtrSize and its documented D0 result,
and reuses `heap-admission.h` with the same 32 KiB recovery margin,
applied to the growth delta. No partition, Core, ABI version, OS state, resident
arena or permission policy change is required. The native binding is linked into
these experimental modules; it relies on this runtime's standalone allocations
being `cli_alloc`/NewPtr blocks. Small page slots never go to SetPtrSize.

- Equal size or a fitting small slot retains its address/capacity.
- Standalone shrink tries to return space natively; if it fails, retains the
  existing capacity and accounting without allocating a second block.
- Standalone growth tries native resizing, releases invocation spare blocks and
  retries if useful, then uses the original allocate/copy/free fallback.
- Failure preserves the original pointer, contents and ownership. Successful
  resize updates owned/peak accounting. Overflow and NULL/zero semantics remain.
- `PORT_ALLOC_TRACE` records failures and aggregate copying counters. It uses
  stack formatting/direct stderr writes and is excluded from ordinary builds.

The comparison pair uses the same upstream object, input, Core 2026111882,
768 KiB requested partition, System 6.0.8, Plus 4 MB Snow fixture and recovery
margin. The final package-cost figures are recorded in packages.json. No resident cost; no extra data copy on successful resize.
[Exact test packages](packages.json), [68000 build](build.txt), and
[native trap disassembly](native-resize-disassembly.txt).

## Validation

[Host ASan/UBSan](host.txt): actual runtime allocation/realloc implementation with
mocked native boundary, testing in-place growth/shrink, failed shrink, copy
fallback, failed growth preserving contents, NULL/zero/overflow, page slots,
10000 mixed operations, owned/native accounting and complete cleanup.

Host mocks cannot establish native Memory Manager behavior; Snow validation is
recorded separately. All private test packages use the actual Clarice updater.
Physical SE performance and System 7/MultiFinder remain unmeasured.

## Reproduction

`sh tests/realloc/build.sh` builds difftrace, diffgrow and alloctest against the
already prepared BusyBox additions objects. `python3 tests/realloc/serve.py`
appends them to the retained private fixture repository; it does not start HTTP.
Serve its directory on 8099, then run the existing Clarice updater locally.
Tests use authenticated Telnet port 23245 and the existing private password file.
Start only the dedicated AdditionsSnow instance with
`SNOW_TELNET_FORWARD=23245,10.0.0.2:2323` and its `test.snoww`. Forwarding is a
process environment setting, not a saved workspace setting. Run baseline.py,
allow listener rearm, run probe.py, then compare.py. Fixtures are the prior `/tmp/ba-*` inputs.

The initial host connection refusal was caused by omitting Snow's forwarding
environment on restart; it is not a guest Telnet defect. The Snow file picker
also stalled while reopening; the dedicated instance was restarted through its
CLI with the exact workspace. Other agents' instances were not used.

## Native error-return defect caught before acceptance

The first candidate used the pinned header's void SetPtrSize wrapper followed by
MemError. The guest mixed-allocation test crashed with an address error; its
transcript is retained in initial-native-crash.txt. A test-only GetPtrSize check
then stopped before writing outside a block: iteration 361 requested 1463 bytes
and the allocation header recorded that size, but the actual native block was
only 49 bytes including its 28-byte runtime header. This failed run is retained
in initial-capacity-mismatch.txt. It was never promoted to a default package.

Capturing the documented trap result established the mismatch directly:
**D0 = -108, MemErr = 0, old native size = 436, requested = 912, actual = 436**.
Thus the global error check was falsely treating a rejected native growth as
success on this exact binding/System 6 path. The final binding captures the
_SetPtrSize result in D0 as a signed OSErr immediately, using documented trap
register conventions. No OS patch or manipulation of the heap is involved.
The native-size diagnostic is test-only and not required on each production call.

With the corrected error handling, all 2000 mixed native operations passed
GetPtrSize capacity and payload checks: 1564 in-place resizes avoided 793865 bytes
of copying; 406 fallback copies moved 181160 bytes. The test also checks explicit
oversized failure preservation, NULL, zero, overflow and small-page behavior.
[Corrected native transcript](capacity-probe.txt).

## Final Snow comparison

The corrected opt-in candidate still fails the repeated-line 256 KiB case with
status 2, but reaches candidate-list growth 93168→102480 bytes (baseline
43500→47844). At this point live owned memory is 293756 bytes, native free
41762, largest contiguous 33860; growth delta 9312 plus the unchanged 32768
recovery margin exceeds the free total. This is now a working-memory/headroom
constraint as well as fallback contiguity, not just temporary duplication.
It avoids 1249744 bytes of copying over 68 successful native resizes and performs
only two fallback copies (8976 bytes). Peak owned is 298692 bytes; cleanup
returns owned to zero. Time to this later failure is 103.21 host seconds /
9755 guest ticks; that is not a speed win or a completed-operation benchmark.

Three alternating 8 KiB comparisons produce identical output. Copy-only host
seconds: 5.378, 5.207, 5.285; native: 5.378, 5.299, 5.233. Medians 5.285 versus
5.299 show no meaningful speed difference. Guest ticks are 438/439/439 versus
435/438/438. These are repeat launches following earlier tests; baseline's
first recorded trace launch is the large failed operation, not a cold-start
8 KiB benchmark. All eight copies (7768 bytes) are eliminated; peak owned drops
17500→14412 bytes (17.6%). Diagnostic candidate image costs 528 bytes more,
and package costs 756 bytes more, than the diagnostic baseline.

These are Snow measurements, not stock SE hardware acceptance. The remaining
large-file limit needs a separately reviewed working-set/partition or upstream
diff strategy; removing recovery headroom is not recommended. Keep native resize
opt-in until broader runtime/System 7 coverage. No production defaults change.

Equal-file status 0, missing-file status 2, Ctrl-C status 130, and a subsequent
full 2000-operation allocator run all pass. Every checked invocation reports
remaining=0, including large-file failure and cancellation. See [comparison
transcript](compare.json) and [test summary](compare.txt). No new Telnet defect
was reproduced. The dedicated guest listener was stopped, Clarice quit cleanly,
and its Snow workspace closed; private disks remain for reproduction.
