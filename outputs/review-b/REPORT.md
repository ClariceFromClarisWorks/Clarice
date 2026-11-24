# Review Batch B — build 2026111957

Seven P2 findings confirmed against f64c9d0 and corrected. Existing formats and
native installation/recovery operations retained; no new transaction mechanism.

1. Reject an older repository Core before root migration or installation writes.
2. A current installation without an incomplete marker returns before snapshot
   verification, cleanup or marker writes. Byte-matched catalog adoption remains;
   pending transactions still follow the existing bound-index retry path.
3. Read locked source utilities without changing them. Carry their lock on the
   native snapshot file, reapply it after verified restoration, and unlock only
   identity-bound retired snapshot files during pruning. Older inventory decoding
   is unchanged. A changed, locked installed target still requires explicit unlock.
4. Restore plans accept the same signed File Manager fork-size range as snapshot
   records. Core download/package bounds remain unchanged; copying stays streamed.
5. Reject managed names `core`, `catalog`, `inventory` before any snapshot writes:
   these collide with reserved native leaves on case-insensitive HFS. Existing
   inventory codecs/restoration remain compatible; no new naming/schema migration.
6. Propagate failed layout/reflow, return failure from CLI geometry changes and
   avoid preference writes. Preserve grid/font/full-window state on failure;
   dialog changes apply layout before committing other preferences/history limits.
7. Publisher verification requires LF lines, final LF, no CR/NUL, and guest-sized
   lines, rather than Python's more permissive splitlines interpretation.

Host ASan/UBSan: original failure reproductions, no-op/adoption/pending/error
branches, geometry failure/status propagation, 8,505 metric/bounds combinations,
restore-size boundaries, reserved-name rejection before I/O and publisher framing.
Native locked snapshot:404 returned-error +404 reset boundaries; locked restore:
199 errors +199 resets. Existing Batch A, CLRSNAP1/2, checksum, interruption,
low-memory, cancellation, disk-full, identity, metadata and cleanup tests pass.
Existing repository/installer tests include90 loader/install and482 transaction
scenarios. [Native results](recovery-host.txt), [baseline evidence](reproductions.txt).

Snow Macintosh Plus/4MB/System6.0.8, fixed uncapped mode (~15x): offline upgrade
1956→1957, root installation and automatic relaunch passed with a locked268,520-byte
nano-profile/data fork and318-byte resource fork. Two repository no-op runs pass;
recorded snapshot checksum3824832129/5828bytes and catalog3430282633/3857bytes
remain unchanged, and no incomplete marker exists. CLI42×16 then66×24 succeeds.
The independent new helper restores/relaunches1956 and recovers deliberately
altered nano-profile contents and its lock. Data, resource payload/map, type,
creator, flags and dates match; only already-permitted resource scratch differs.
Legacy Saved inventory is byte-identical. User note/configuration remain; accepted
66×24 preferences survive restart/rollback. Fault injection is host-model evidence,
not a physical power-loss guarantee. >16MiB size acceptance is codec/plan-tested,
not a multi-gigabyte guest transfer test. Physical acceptance remains pending.

Core MacBinary434,048→435,584bytes (+1,536). Main CODE text236,712→237,052
(+340), data34,872→34,852 (−20), BSS41,456 unchanged. No new persistent allocation
or startup inventory work. Recovery reuses bounded buffers; native attribute checks
add transient stack state. Guest remaining command allocations0. Repeated scripted
checks show free/largest334→313KiB with purgeable capacity358KiB unchanged; these
native resource effects are reclaimable, not claimed as zero raw heap variation.
No-op updates avoid the previous full snapshot verification cost; no timing claim.

Product **59e5116**, integrated local main. Published build **2026111957** to
`http://192.168.1.112:8089/full/index.txt` and `Macintosh Exchange/Clarice/index.txt`.
All51 entries/36 payloads, sealed offline HDA/archive and committed source verified
byte-for-byte; prior1956 index/payloads retained. [Verification](verification.json).
No hardware operations, GitHub push or Batch C work.
