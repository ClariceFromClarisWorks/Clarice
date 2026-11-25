# Installer B — System 6 acceptance

Build **2026111977**, product **f67102faad6935431993a8c58ba3d3b95164992e**, local main.
No physical access, System 7 testing or GitHub push.

## Implementation

Native Core/Data folder selection uses original Standard File. Both locations
must share one HFS volume. A 524-byte checksummed **System Folder:Clarice Locations**
record is authoritative; missing records retain legacy discovery, invalid records
fail closed. Confirmation precedes its creation. Existing locations are fixed:
no relocation, automatic movement or reinterpretation of user data.

V3 snapshots reuse the existing header and its origin field for the actual Core
restore target. V1/V2 retain their original root-volume recovery semantics, byte
formats and checksum policies. Existing copy, verification and rollback machinery
is retained. Offline upgrade verifies sealed input and catalog ownership, adopts
exact legacy files, replaces unchanged older tools and preserves verified newer
compatible tools. Conflicts fail safely. Explicit tool Repair requires a kit
matching installed Core; it preserves the verified snapshot and manual per-file
backups. Kit-bound retry uses the existing v2 marker grammar unchanged.

## Acceptance

- ASan/UBSan: location codec/every-byte corruption, 12 native record I/O/disk-full/
  interruption boundaries, 222 custom restore boundaries, 349 catalog append
  failure boundaries, newer/modified/missing tool policy and repair metadata.
  Existing dialog, repository catalog and recovery fault matrices pass, including
  original V1 snapshots and interrupted legacy snapshots. See host logs here.
- Normal 68000 Snow Plus/4 MB/System 6: native chooser, cross-volume rejection,
  exact whole-disk no-write chooser/Cancel; custom Core in Apps, Data in Files;
  relaunch, namespace and all 49 tools. Final sealed Core and every utility were
  read back from both default and custom installations and matched exactly
  (only documented Resource Manager scratch bytes excluded for Core).
- Real disk-full (-34) after five installed tools; free only the disposable filler
  and retry the same kit: all 49 tools and Core install and completion clears.
  Active native Cancel and Command-period recovery, legacy 1976 upgrade and
  wrong-version Repair rejection pass. Historical Core copies, user document and
  profile remain byte-identical MacBinary files across legacy upgrade.
- Custom managed-tool upgrade creates a coherent V3 previous snapshot. Launching
  its independent Recover Clarice in Finder restores the exact previous catalog,
  older utility and custom Core target, then launches successfully. User data and
  both forks are preserved. Finder itself set the sentinel's initialized flag
  while its folder was viewed; payloads, codes and dates remained unchanged.
- Changed-tool refusal preserves catalog/snapshot. Matching-version Repair fixes
  damaged Finder codes and a missing tool, retaining both forks of the original
  in .clx-ls.old and preserving an unrelated tool and the verified snapshot.
  Snow caught two candidate-only errors (Finder-code repair and marker grammar);
  both are corrected and have focused host regressions.
- Unchanged reinstall preserves exact Core and recovery inventory without rotation.
  Its existing completion path appends a duplicate Core catalog row; effective
  catalog contents are unchanged. This minor write remains, rather than expanding
  the task into updater optimisation.

Disposable images/readbacks are in work/installer-b. No user storage was tested.

## Costs and limits

| Item | 1976 | 1977 | Change |
| --- | ---: | ---: | ---: |
| Core CODE | 285947 | 287706 | +1759 bytes |
| Core MacBinary | 440064 | 443776 | +3712 bytes |
| Installer CODE | 281221 | 289567 | +8346 bytes |
| Installer MacBinary | 435712 | 446080 | +10368 bytes |
| Helper CODE | 75638 | 75712 | +74 bytes |

Requested partitions remain Core/installer 768 KiB, helper 160 KiB. No new
persistent Core heap buffer; startup reads the small location record once.
Installer-only selection paths add 512 bytes plus flags; tool planning uses a
bounded stack table. Peak installation heap was not separately sampled.

Diagnostic entry-to-responsive confirmation: **55 guest ticks** (~0.92 guest s),
versus prior Installer A's 34-tick sample. This includes the larger form and location
lookup, excludes Finder loading, and is not a matched physical-speed comparison.
The window is shown before location discovery. Ten repeated command cycles show
remaining=0, free heap 349584–349588 bytes and a fixed 349356-byte largest block;
small variation follows shell argument storage, with no progressive decline.

System 7, MultiFinder and physical hardware acceptance remain pending. Locations
cannot be moved/renamed after installation. Repair does not repair an unreadable
Core: use the independent recovery helper. Existing manual backup conflicts are
not deleted automatically. Interrupted native HFS operations remain recoverable,
not power-loss atomic; the separate intermittent relaunch crash is not claimed fixed.

## Distribution

[HTTP index](http://192.168.1.112:8089/full/index.txt) and
**Macintosh Exchange/Clarice** contain identical verified 51 entries/36 payloads,
including optional vi unchanged. Versioned HDA/USTAR/source archives and the seal
were published before activating indexes. Full offline readback checks the native
installer/helper, Core, Release and all 49 tools. Previous 1976 artifacts and index
remain verified and accessible. See verification.json. Stop after Installer B.
