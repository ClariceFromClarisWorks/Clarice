# Batch 1 closure corrections — build 2026111962

Accepted product **e57f34f**, local main. Baseline 1961/6effb1d. Five reported defects reproduced, corrected without changing
parser, loader, receipt or runtime formats. No physical hardware access.

| Finding | Small correction and acceptance |
| --- | --- |
| Escaped bracket hyphens | Carry quoted `-` through both pattern encoders. Case, prefix/suffix trimming and native HFS glob tests distinguish literal hyphens from ranges. |
| External HOME | The CLX environment adapter uses `/home`, matching the shell. Actual Snow awk default, explicit override, env -i suppression and subsequent default restoration pass. |
| Positional tilde | Only lexically unquoted text (mode 0), not expansion-produced text (mode 2), qualifies for tilde expansion. `$@`, scalar, `~/file`, quoted and ordinary lexical cases match dash. |
| Pipeline failures | Preserve cancellation before restoring stage state; skip rewind after cancellation. Internal setup/rewind failures bypass `!`, while ordinary command statuses are still negated. Injected rewind failure returns 2; concurrent cancellation returns 130; streams and next-command execution recover. |
| Loader receipts | Memory refusal does not revoke receipts. Stdio failure reports memFullErr rather than paramErr. Six injected admission/allocation failures preserve exact receipt bytes and permit fast retry; genuine checksum/structural failures still revoke. |

Focused host checks: 10 closure shell cases, 61 new allocation-failure/recovery
trials, native glob/environment shims under ASan/UBSan; existing 48 shell cases,
3 cancellation cases, 54 POSIX cases, 347 allocation faults, 92 loader/installer
scenarios, cumulative loader-pressure tests and remote session/service tests pass.
Native rewind and individual allocation fault injection remain host-only.

Normal 68000 Snow Plus/4 MiB/System 6.0.8 local and loopback-remote acceptance:
see [guest evidence](guest.json) and [local output](guest-local.txt). Local pressure
reached free 46,776/largest 46,656 bytes; refused with 126, retained byte-identical
receipt, released all pressure allocations, then executed via installed-fast mode.
Warm repeated diagnostic loads: heap 335,590/largest 335,538 bytes unchanged,
remaining command allocations 0. Existing recovery copies and utility files preserved;
corruption testing uses a separately installed disposable diagnostic.

Cost versus 1961: text +16 bytes, data +16, BSS unchanged; Core MacBinary unchanged
at 437,376 bytes. No new persistent heap allocation, bitmap, startup check or I/O.
Only existing transient pattern storage may hold an extra escape byte. No speedup
claim; this is a correctness batch, with physical acceptance outstanding.

Offline HDA readback verifies both Core forks/Finder fields, Release and all 49
utilities; tar files match the sealed kit. All standard utility payloads are unchanged.
Publication checks and product identity: [verification](verification.json).
No GitHub push or hardware deployment; stop after these five corrections.

Publication verified: 51 catalog entries/36 unique payloads, exact HTTP/AFP equality,
sealed offline/source packages and preserved prior 1961 index and payloads.
