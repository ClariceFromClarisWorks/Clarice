# Review remediation A — build 2026111956

All five reports reproduced against 397c2af; corrections reuse the current installer,
marker, bounded native copying and snapshot rotation. No new transaction system.

1. Offline verification failure retains staged Core alongside the durable plan.
2. New update markers bind retries to index length/CRC32. A changed index fails
   before installed changes; resume the original immutable index or recover.
   Legacy v1 markers remain readable, with snapshot membership checks and exact
   expected payload checks for originally absent tools. Their original index
   cannot be reconstructed retroactively. Snapshot/catalog formats are unchanged.
3. Reopening an unchanged offline kit with all destinations present does not rotate
   recovery or rewrite installation state. Pending work/missing tools still resume.
4. A different helper left before the first Inventory is preserved under an ID-based
   name outside the snapshot, using bounded copy/readback before deleting its old
   location. This one-time legacy residue is deliberately retained, not adopted as
   known-good. Unknown collisions fail closed. Both missing-file and missing-parent
   HFS errors are handled. Snow exposed the latter; the host model now models it and
   rejects cross-directory HRename, matching the native APIs.
5. Cached offline helpers must match APPL/CLUP, exact fork lengths and both checksums,
   using the same resource normalization already accepted for shipped helpers.

Host ASan/UBSan: focused five-finding regressions; corrected first-helper retry at
402 returned-error and 402 reset boundaries, collision and allocation failure;
existing legacy inventory/restore, 732 previous-helper boundaries, 498 preparation
errors +498 resets, 403 first-snapshot boundaries, 215 rotation and273 restore
failures; repository/catalog/helper tests and482 Core transaction scenarios pass.
Both forks, protected metadata, user-owned files and allocation/descriptor cleanup
are checked. These are fault-model tests, not physical power-loss guarantees.

Snow Plus/4MB/System6.0.8 at fixed maximum acceleration: offline upgrade/relaunch,
repeat-install unchanged inventory (cksum1570465105,5828bytes), interrupted package
installation, changed-index rejection and original-index retry pass. Status sequence
1,1,0,0,0: missing payload, changed index, successful retry, two installed probes.
Free heap/largest block360KiB before/after. Final native-helper retry passed without cleaning the failed preparation first.
Independent snapshot-helper rollback restored/relaunched1956 and removed both
new probes. Preferences, user note, original Legacy Saved inventory, nano-profile
both forks/type/creator/dates/flags survived. The saved old helper differs only
in already-documented system-reserved resource bytes. Final-run free/largest
heap334KiB before/after (358KiB purgeable); raw runtime heap345,054/largest344,874
and remaining allocations0 after failed update and successful retry.

No physical operations or GitHub push. Existing recovery material is preserved.

Core package432,384→434,048bytes (+1,664); main CODE text236,712, data34,872,
BSS41,456 unchanged. No new resident allocation or ordinary startup work. A new
update hashes its small downloaded index once; markers grow18bytes. First-helper
rescue reuses2KiB streaming buffers and retains one old helper outside the snapshot.

Product **f64c9d0**, integrated local main, normal build **2026111956**. Published
through canonical HTTP `http://192.168.1.112:8089/full/index.txt` and AFP
`Macintosh Exchange/Clarice/index.txt`. All51 entries/36 payloads, sealed offline
HDA/archive and committed source verified byte-for-byte; previous1955 index and
payloads retained. No physical deployment, GitHub push, public release or P2 work.
