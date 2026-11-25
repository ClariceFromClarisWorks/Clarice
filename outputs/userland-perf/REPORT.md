# Shell/CLX performance — build 2026111961

Accepted product **6effb1d**, build **2026111961**, local main.
Baseline product 98c0b8a/build1960. Normal 68000 Snow Plus/4 MiB/System 6.0.8;
uncapped emulator observed around1.5x under load. Times below are guest TickCount
seconds, including native scheduling/I/O, not host elapsed time or physical-SE results.

| Finding | Change and evidence |
| --- | --- |
| Parser input copying | Retain mrsh's buffer, advance a read cursor; compact only on refill or alias replacement. Data/fd/shared-buffer inputs, lookahead, reset and alias tests pass. 8 KiB/32 KiB benchmark source moves fall from33,558,528/536,887,296 bytes to 0. Existing parser/AST/ownership model unchanged. |
| Unused receipt checksum | Streaming verification computes the separate metadata digest only when creating a receipt. Mandatory payload hash, bounds/import checks, identity and receipt formats remain unchanged. Removes 51,652 extra hashed bytes per repeated awk check, with identical file reads. Ordinary execution already avoided this redundant calculation. |
| Loader headroom | Native pressure probe entered the old loader's child with 12,414 bytes free/largest 12,262. Reuse native_heap_room and the existing32 KiB recovery policy for image+metadata admission and argv. No reserved block or new executable limit. Candidate rejects that low-room case with 126; later execution succeeds after freeing pressure. Fragmented entry succeeds with 159,254 total free/largest 13,250: total reserve and contiguous-allocation checks remain distinct. |

| Matched operation | Before | After |
| --- | --- | --- |
| 8 KiB,128-command script | 210.48/209.93s | 30.95/30.58s (~85% less) |
| Full appinstall check of awk, repeated | 8.83/8.80/8.85s | 7.63/7.68/7.68s (~13% less) |
| Installed-fast ls loading | 30/30/31ticks | 30/29/29ticks (effectively unchanged) |

32 KiB/512-command candidate completes with status0 in 361.81 s. Its old-build guest
run was stopped before completion; no matched guest percentage is claimed. Host
parser CPU:8 KiB0.651→0.184ms;32 KiB8.469→0.844ms. A32 KiB padded script with one
command completes in 17.11 guest seconds. Command-heavy large scripts therefore
remain expensive beyond unread-byte copying; this batch does not diagnose or
optimise that remaining cost. No new parser event scheduler was added.

Acceptance: 48 shell comparisons/3 cancellation cases, 54 POSIX cases, 1,268 parser
allocation-failure/recovery trials, cursor refill/alias/reset tests, 92 loader/install
cases plus cumulative/contiguous allocation failure, cancellation and retry tests.
Existing remote session/service/serial host regressions pass. Normal Snow verifies
actual scripts, quoted empty arguments, arithmetic/patterns, pipelines, cancellation,
pressure recovery and repeated loads. A deliberately changed payload byte is rejected
by appinstall check with 126; subsequent commands work. Remaining allocations 0;
largest block 320,724 bytes throughout repeated runs and after the large script.
Native pressure allocations are explicitly freed; the warm fragmented probe restores
its exact free/largest figures. Fault injection is host-only.

Cost: 4 transient bytes per active parser; no new persistent heap allocation/cache.
Core text +416,data +36,BSS −4 bytes; MacBinary 436,864→437,376(+512). Matched native
free-heap samples are approximately0.5–0.8 KiB lower across builds; no progressive
loss is observed. Mandatory verification and recovery behaviour are retained.

The disposable diagnostic's initial output callback recursively routed into itself;
that test-only error and a test declaration mismatch were corrected before collecting
accepted pressure evidence. They required no production fix. Full test data:
[measurements](measurements.json). Publication identity/checksums: [verification](verification.json).
No physical access, GitHub push or next audit batch.

Publication passed: all 51 catalog entries and 36 unique payloads match the seal
and both canonical HTTP/AFP sources. Offline HDA/tar and source archive verified;
previous 1960 index and payloads remain available.
