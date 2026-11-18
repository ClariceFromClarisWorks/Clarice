# Nano buffered input — 18 November 2026

Snow-only acceptance: Plus, 4 MiB, System 6.0.8, fixed maximum acceleration.
Core 2026111823/runtime 1.10; same nano-profile command and Core for both variants.
Three warm invocations per workload per variant. Guest TickCount measurements,
not host elapsed time or physical-SE performance. No new cold-boot measurement.

| Workload | Baseline mean ticks | Buffered mean ticks | Interpretation |
|---|---:|---:|---|
| No file | 235.7 | 222.7 | Allocation variation; no attributable read benefit |
| Existing empty | 267.0 | 281.3 | Allocation variation; no read benefit |
| 8 KiB / 128 short lines | 429.7 | 403.0 | 6.2% mean; median 418→402 (3.8%) |
| 8 KiB / two long lines | 340.7 | 332.3 | 2.4% mean |

Ordinary-file open/buffer phase 118.3→98.3 ticks; read-loop samples
79/64/64→57/50/51. Long-file open/buffer phase 120.3→109; read-loop
61/74/62→58/57/58. Loader remains roughly 100–104 ticks; short-file initial
refresh roughly 80–81. Painting remains roughly 60 ticks. The three-second
startup goal is not achieved; physical confirmation is pending.

## Retained change and provenance

The pinned genuine nano 9.2 file-reading loop calls getc for each byte. Its
Newlib build did not enable the single-thread inline getc macro. The tiny
Macintosh adapter now consumes already-buffered bytes directly and delegates
refill, EOF, pushback and errors to the existing libc getc. This follows Newlib's
stdio.h __sgetc buffered-cursor technique and the existing port_getline adapter.
No upstream editor algorithm, ncurses, renderer, runtime imports or ABI changed.
No new cache or allocations. Diagnostic image +36 bytes (269384→269420);
loaded image 199152→199176. Normal optional nano is 268900 bytes.

Owned peaks unchanged at 74572/75112/101736/95662 bytes for the four workloads;
all 24 measured invocations remaining=0. Native free heap remains approximately
364168–364194 and largest block 360412–360558, without progressive decline.

## Validation

- Host ASan/UBSan: 4096-byte all-value input, buffer sizes 1/8/64/512/4096,
  repeated ungetc, EOF/clearerr, pushback after EOF, interrupted refill/recovery.
- Snow: three Command-period cancellations at different startup stages, status
  130, zero retained allocations and subsequent successful shell execution.
- Editing, save/reopen, exact contents and TEXT/ttxt metadata, successful search,
  light/dark display and terminal restoration. Baseline/candidate editor image:
  zero differing pixels in the compared editor region.
- Normal non-instrumented nano 2026111827 installed through clarice update -y;
  save produced exact NORMAL EDIT ORIGINAL plus newline, preserved type/creator,
  and remaining=0. Diagnostic nano-profile 2026111826 remains separately optional.
- Prior safe-save fork/failure coverage remains applicable: save implementation
  is unchanged. Resource-fork failure injection was not repeated this batch.

Raw evidence: same-name.json, guest-acceptance.json, normal-acceptance.json;
measure.py and collect.py retain the matched measurement procedure.

Refresh batching and initial blank-screen deferral were tested and rejected;
see ../nano-profile4 and ../nano-profile5. Original renderer restored. The
normal Core was compiled separately; measured Snow Core contains instrumentation.

One optional-app installation refused relocation allocation with telnetd active.
Stopping remote services and retrying completed successfully with recovery copy;
no updater change made. Remote update --check correctly rejects nonlocal input
ownership; final successful install was performed at the local prompt.

No SE access, push or release. Next useful step is matched physical startup
measurement, then investigating file construction/layout rather than assuming
additional refresh batching will help.
