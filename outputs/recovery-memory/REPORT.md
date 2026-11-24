# Recovery-memory regression — build 2026111952

## Cause and correction

Recovery combines the repository with installed.catalog, so catalogued optional
`nano-profile` is preserved; unrelated development files are not swept into the
inventory. Copying already streamed both forks in 2,048-byte blocks. The unnecessary
executable-verification callback instead allocated the whole CLX image (198,756
bytes) and metadata (69,732 bytes): 268,488 transient bytes. Normal Core also cannot
link this archived diagnostic's profiling-only imports. Snapshot preparation now
preserves catalog-bound bytes without linking them. Catalog size/CRC, both-fork
readback, Finder metadata, inventory validation and rotation remain enforced.

Explicit CLX checks/install validation use bounded full verification: a separate
2,048-byte stack buffer, no executable heap allocations, all checksum, structural,
relocation-bounds and import checks retained. Native memory/I/O/cancellation errors
are propagated; the old wrapper masked memFullErr (-108) as paramErr (-50).

Snow Plus/4 MB/System 6.0.8 reproduced the original failure with a 30 KB shell
value retained: free 163,728 bytes, largest 91,068; after purge 188,056/91,076.
Another run failed the metadata allocation with 218,048 free/217,896 largest.
Remote services were disabled. The host harness using the actual old source and
archived image reproduced -50 with a 198,756-byte allocation request; the new
verifier succeeds with zero requests, including under a zero-byte heap ceiling.
This eliminates the 262.2 KiB executable allocation, not the existing inventory
and native streaming buffers. The verified 68000 function has 2,052 bytes of local
stack plus saved registers; ordinary execution does not reserve this frame.

## Narrow checksum compatibility

New inventories use explicit CLRSNAP2 and per-entry checksum policy, without
changing the wire record sizes. Resource Manager forks must pass the existing
bounded header/layout checks before excluding only documented system-reserved
bytes 16–127. Header 0–15, application-reserved 128–255, payloads, maps, remaining
fork bytes, data forks and protected Finder metadata remain checked. Opaque or
malformed forks are exact; they do not gain a blanket resource-fork exception.

CLRSNAP1 remains readable with its original exact utility-resource checksums.
Legacy restoration verifies the immutable source under those rules before deriving
a temporary native-copy expectation; it never rewrites the inventory or old restore
journal. Legacy partial inventories retain their old convention. Old helpers reject
CLRSNAP2 rather than misinterpret it; new snapshots include the compatible helper.

An interrupted pre-fix preparation can contain the old helper. Retry replaces only
a duplicate verified against the complete previous snapshot, tolerating different
dates on that disposable duplicate alone. Both forks and protected metadata must
match; unknown files and the sole recovery helper are preserved. This uses the
existing preparation/rotation machinery.

## Focused acceptance

Host ASan/UBSan: 90 loader/install cases; legacy inventory validation/restoration
and interrupted prefixes; 732 helper retry error/interruption boundaries; 498
preparation error and 498 interruption boundaries; 403 first-snapshot boundaries;
215 rotation failures; 273 restore boundaries with no required manual intervention.
Disk-full, memory failure, cancellation, both forks and user-file retention remain
covered. No retained allocations/descriptors in the host model.

| Test | Result |
| --- | --- |
| Native copy modifies only reserved resource bytes | Pass: Snow differences confined to 48–125; all other bytes identical |
| One meaningful payload byte corrupted | Rejected by production verifier in host tests |
| Resource map/header/data fork corrupted | Rejected |
| Opaque fork byte 48 changed | Rejected |
| Existing CLRSNAP1 utility scratch byte changed | Rejected under original exact checksum |
| Existing CLRSNAP1 restoration through native copy | Pass; original inventory unchanged |

Snow used the archived 268,520-byte nano-profile data fork plus a valid 318-byte
Resource Manager fork with meaningful payload/map, explicit Finder metadata and
preserved user files. The earlier 175-byte synthetic opaque fork is not used to
justify normalization. Actual readback: data identical, header/payload/map identical,
type/creator/flags/dates preserved (guest-forks.json).

The final sealed offline installer resumes the pre-fix failed preparation, verifies
the existing legacy generation, installs Core at startup-volume root and automatically
relaunches 1952. A constrained update prepares the snapshot and installs the probe;
Ctrl+C returns130, retry completes with0 and keeps completed work. Free heap/largest
block remain321/232 KiB before/after retry, tracked remaining allocations0. The final sealed Core also completed an uninterrupted constrained update with
status0, preserving the same321/232 KiB free/largest values and zero tracked
remaining allocations (final-update.txt, final-memory.txt). These are before/after
measurements, not a complete native snapshot peak-memory trace.

Independent guest rollback works with the normal Core path absent: the snapshot's
Recover Clarice restores and relaunches1952, removes the probe absent in that
snapshot, preserves unrelated Failed Core/user notes, keeps the inventory byte-for-byte
unchanged and restores both optional-command forks/metadata (guest-restore.json).
No physical hardware test or deployment has occurred.

## Cost and limits

Core MacBinary 428,672 → 430,848 bytes (+2,176); main CODE text +844/data +20,
BSS unchanged41,436 bytes. No partition increase or new permanent buffer; four
transient bytes per recovery member plus four per header identify checksum policy.
Standard utilities are unchanged. No additional ordinary startup work.

No atomicity/power-loss guarantee beyond the tested native interruption/retry model.
An installed pre-fix updater cannot apply a fix it cannot reach: use the corrected
offline installer once, keeping existing files, to bootstrap this failure. Do not
delete nano-profile or the interrupted snapshot. Physical-SE acceptance remains
pending explicit authorisation. Flag allocation/verification paths for the later
independent code review; this task introduces no further recovery architecture.
