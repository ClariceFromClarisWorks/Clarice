# Review Batch C — build 2026111958

Both reported costs confirmed and corrected without format/runtime redesign.

Rotation retains the retired-directory ID from its successful post-rename full
verification, using it immediately for identity-bound pruning. It no longer loads
and hashes that same snapshot again. Resumed calls reset this temporary state and
verify afresh; resumed pruning also avoids a redundant final recheck. Candidate
and post-rename verification, failure handling and old snapshot formats remain.

Matched production-header/native-HFS-model benchmark:73→51 reads and46,532→31,364
bytes per rotation (−32.6% bytes). Two100-rotation host CPU batches:18.479→14.658ms
and18.512→14.339ms. These are actual model I/O counts and host CPU measurements,
not SE/Snow disk timings or a claim that whole updates improve by33%.

Ping now calls existing cli_pause_ticks for the unused portion of its60-tick
interval. That service handles events and uses WaitNextEvent with a one-tick wait;
no new scheduler or stack/network code. Slow replies/timeouts add no extra delay.
A controlled command-level clock model changes60 direct busy polls into one pause
request with identical60-tick spacing; this is not a measured guest CPU percentage.

Host ASan/UBSan: rotation155 returned-error boundaries; preparation429 errors and
429 resets; previous-helper663 boundaries; first snapshot403 boundaries; locked
preparation404 and locked restore199 errors/resets; restore277 boundaries; legacy
checksums, corruption, cancellation, disk-full, allocation failure and cleanup pass.
Additional regression corrupts a snapshot after an earlier successful check and
reuses the same context: the subsequent call rejects it and preserves retirement.
Native ICMP/DNS lifecycle, malformed replies, timeout/loss, command cancellation,
interval remainder/slow-probe tests pass. Batch A/B focused regressions pass.

Snow Plus/4MB/System6.0.8, fixed uncapped speed (~15x): normal offline upgrade,
snapshot rotation and automatic1958 relaunch pass. Apple MacTCP/MoofPort fixture:
responding host, unreachable host, successful/failed DNS, four repeated runs,
local Ctrl+C and Command-period130, prompt and next-probe recovery pass. Matched
four-probe command totals3.95→3.98 guest seconds including loader/output; pacing
is intentionally retained. Actual RTT67–100ms (native tick resolution), loss100%
for the timeout fixture. Heap343182/largest343000 stable across all four repeats;
owned peak/remaining0. Purgeable capacity356KiB unchanged. Host fault tests cover
resumed recovery/rollback; no new physical or MoofTCP guest acceptance is claimed.

Core package435,584bytes unchanged; main CODE237,052/data34,852/BSS41,456 unchanged.
Ping5,644→5,708bytes (+64). Rotation context adds4 transient stack bytes and a local
integer; no persistent allocation, bitmap, startup scan or disk-format change.
Existing full verification before/after rename remains intentionally intact.

Integration/publication identifiers follow in CURRENT-STATE and verification.json.
No physical operations, GitHub push or further milestone work.
