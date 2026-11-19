# BusyBox vi completion — 18 November 2026

Optional genuine BusyBox 1.37.0 editor, private build 2026111905. This branch
retains the accepted evaluation and adds recovery, a smaller measured build and
installation under `vi`. Core and the standard utility collection are unchanged.
Requires the existing CLX runtime 1.9 (tested Core 2026111823).

## Changes

- A compile-time, editor-only cleanup callback preserves modified text before
  invocation allocations are reclaimed. Includes pending upstream undo-queue
  edits, which are not yet reflected in modified_count. No signals/process layer.
- Exclusive `/etc/clarice/vi-XXXXXXXX.part` creation, bounded 4 KiB writes, checked
  close, then native same-directory rename to `.recover`. Never overwrites the
  original or an existing recovery. Failure leaves a clearly incomplete `.part`.
  No heap allocation in the preservation routine; no resident state or cache.
- Upstream text growth now commits end/size only after allocation succeeds.
  Buffer reset clears pointers before a fallible allocation. These contained
  adaptations keep recovery from reading freed or out-of-range text after OOM.
- Existing safe HFS replacement remains the normal save mechanism. Both forks
  and Finder metadata remain owned by that shared service.
- Size-optimised whole-program LTO, preserving the exported platform entry
  points, cuts unused code/relocation work. No instruction beyond 68000, new
  library, permanent allocation, loader change or renderer change.

## Matched comparative performance

Three runs per input, same accelerated Snow Plus/4 MiB/System 6.0.8 fixture,
Telnet display-and-key oracle. Host elapsed times include transport/capture;
guest loader ticks corroborate the comparison, not physical-SE performance.

| Configuration | Empty ready median | 8 KiB ready median | Loader ticks empty / 8 KiB |
|---|---:|---:|---|
| Original evaluation | 0.934 s | 1.404 s | 46–51 / 46–48 |
| Recovery, ordinary -Os | 1.138 s | 1.701 s | 54–56 / 54–56 |
| Recovery, -O2 | 1.118 s | 1.639 s | 54–56 / 52–54 |
| Recovery, retained -Os/LTO | 0.972 s | 1.451 s | 44–46 / 42–45 |

LTO improves the recovery-enabled build by about 15% in ready time and 18–20%
in loader ticks. Relative to the original, ready times are effectively similar
(~4% slower in this batch), with recovery added; do not claim an overall speedup
from unrelated older timings. First visible insertion remains ~0.13 s empty and
~0.25 s at 8 KiB: it has not been optimised away. All are emulator comparisons.

Final package 51244 B, image 36176 B, 953 relocations; original 56704/36912 B,
1352 relocations. The recovery-only ordinary build was 57992 B. -O2 inflated the
package to 81696 B without a useful enough gain and was rejected. The first LTO
configuration retained an unused upstream dependency and failed import validation;
it was rejected before guest deployment. The corrected configuration passes.
Working peaks remain 21266 B empty / 23314 B at 8 KiB. No Core idle allocation.
The final help-only wording build reproduces byte-for-byte on a clean rebuild.

## Validation and limits

Host ASan/UBSan: actual growth routine allocation failure/overflow/moving growth;
recovery create/collision/short-write/write/close/resolve/rename failure; existing
safe-save adapter failures; shared HFS transaction's 27 failure and interruption
points, both forks/metadata, locked/read-only/disk-full/cancel and 20 saves pass.

Snow: 43 existing editing/search/undo/arrow/read-only/locked/creator/save/output/
status observations pass with the final executable installed as `vi`, including
full 8192-byte saved-content comparison. Disconnect recovery uses unique inserted
text and exact byte comparison, reconnection, subsequent editing and remaining=0.
Final IAC IP cancellation closed the raw connection before client disconnect;
unique inserted text was recovered exactly and subsequent editing passed. Three
512KiB opens exhausted available memory safely: heap357916/largest355660 B
unchanged each time, remaining0, reconnect and help passed. Local-window insert
of two lines, Escape, :wq, exact readback and terminal restoration pass. Installed
vi1905 catalog and explicit appinstall check pass. See the corresponding JSON
and local screenshots. No new runtime/editor allocation remains after exit.

Recovery is best-effort on orderly invocation failure/cancellation/disconnect,
not crash/power-loss protection or continuous swap-file journalling. It requires
writable storage and free space; recovery contents are data forks, not a complete
Macintosh-file backup. Normal saves preserve the original's metadata/forks.
Recovery files are user data and are not automatically deleted. Recover by opening
one in vi and explicitly saving to the desired filename after inspection.

Literal-only search, limited ex, no shell filters/job control/automatic .exrc,
byte-oriented text, fixed remote 80x24 and RAM-resident document/undo remain.
No physical SE/PowerBook, System 7 or single-Finder acceptance is claimed.
Guest transport tests used an explicitly enabled, loopback-forwarded test service
without a password; production authentication was not changed or retested here.
Original attribution, GPL notices/source and independent port patch are retained.
No changes to the shared update repository, release candidate or main branch.

## Build and installation

Run `scripts/build-busybox-vi.sh`; output `work/busybox/vi.clx` (`bbvi.clx` is the
same image retained for comparative tests). This is an optional external app.
Add it to an existing private repository, preserving its current Core/tools:

```
python3 tests/busybox/publish.py REPOSITORY work/busybox/vi.clx 2026111905 --name vi
```

Then use the normal local `clarice update -y`. Do not replace an existing newer
version with this version. The private Snow repository was updated this way;
previous `vi` was retained by the existing installer as `.clx-vi.old`.

Offline first installation: `appinstall install /mnt/DISK/vi.clx vi`.
For an explicitly chosen replacement use `appinstall replace /mnt/DISK/vi.clx vi`;
inspect/preserve an existing recovery copy if the installer asks. Normal startup
performs no new installation or editor work. Use `vi --help` for limitations.

Basic use: `vi /etc/clarice/profile`, `i` to insert, Escape, `:wq` to save/quit;
`:q!` deliberately discards edits. `u` undo, `/text` literal search, `n` next.
After abnormal exit, inspect `ls /etc/clarice/vi-*.recover`; open a selected file,
then `:w /desired/path`. A `.part` file must be treated as incomplete.
