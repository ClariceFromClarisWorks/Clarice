# Batch 3B — HFS correctness, build 2026111967

All three defects reproduced against 15f2c3e and corrected without changing the
namespace, file-copy transaction or safe-save architecture. Publication pending.

- Portable `cli_remove`: use existing no-follow resolution, with slash/colon
  suffix guards. File, directory and dangling links remove only their own entry;
  suffixes reject safely. Ordinary files and empty directory slash operands work.
- Safe-save: correct the native-create argument order to TEXT/CLRC. Existing-file
  restoration is unchanged. Real vi and nano create the correct native metadata.
- Copy fallback: retain backup date and all extended Finder bytes for same-volume
  moves. Across volumes clear icon/comment/home-directory IDs, on-desktop flag and
  desktop folder reference; preserve the remaining metadata. Existing Finder
  hasBeenInited clearing remains. cp -p shares the same corrected helper.

The volume-specific distinction follows Apple's [File Information Records](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-465.html).
Desktop database comment *text* is not migrated between volumes; copying its ID
would reference unrelated destination data. No Desktop database layer was added.

## Acceptance

- `tests/filesystem-p2/unlink.py`: actual resolver/adapter, baseline negative
  control, file/directory/dangling links, suffixes, native operands, invalid paths,
  native error/retry and cleanup under ASan/UBSan.
- `tests/ports/safe-save-host.c`: baseline Finder-code assertion fails; corrected
  test passes, including27 native failure points,27 interruptions, allocation
  failure, short writes, full/read-only/locked media, cancellation, rollback,
  repeated saves and no retained allocations.
- `tests/filesystem-confidence/copy-faults.py`: baseline backup-date assertion
  fails;232 same/cross-volume cases pass, covering every native boundary, partial
  writes, cancellation, source/destination integrity and closed fork references.
  Prior Batch3A and filesystem-confidence regressions also pass.
- Normal 68000 Snow Plus,4MiB,System6.0.8, maximum fast-forward (~15x):31 filesystem
  checks,12 initial editor checks and18 repeated-save/cancellation checks. Two
  disposable HFS volumes; same-volume parent+leaf change forces copy fallback.
  Native probe checks FInfo, all16 FXInfo bytes, all three move dates, both forks.
  Full-volume failure preserves source and old destination; cancellation130,
  retry, subsequent commands and prompt restoration pass.
- vi/nano each save new and existing files, then each save three further new
  files. Actual TEXT/CLRC and empty new resource forks verified; existing resource
  payload/maps, Finder information and creation date retained. No save siblings
  remain. Existing safe-save backup-date behaviour is unchanged (it resets on
  editing); the backup-date correction here is in copy/move.
- Three15-copy/unlink batches: free heap321004, largest320836 bytes on every batch.
  Six alternating editor runs: free324916..324976, largest323256..323330; no
  progressive free-heap decline. All command remaining-allocation counts0.
  Local Ctrl+C/Command-period both return130 and recover. A final host connection
  was retried after the previous single-peer connection drained; no guest reset.
- Resource fixture is the existing valid upstream-Rez test file. Comparisons
  allow documented system-reserved bytes16..127 to change during native copying;
  meaningful headers, application bytes, payloads and maps are compared. This
  does not change production verification or claim arbitrary opaque-fork equality.
- Offline HDA Core/installer/recovery forks and Finder metadata, Release and49
  utilities match inputs; all51 USTAR members match the sealed kit.

## Cost and limits

Core MacBinary438528→438656 bytes; code+168,data−8,BSS41448 unchanged. Shared files
image20636→20792 (+156),code+132,BSS168 unchanged. No new heap allocations or
startup work. Unlink still uses768 bytes of local storage (compiler overlays the
temporary catalog record), with16 more saved-register bytes. Only suffix unlink
adds a catalog check; move metadata uses the existing set-catalog operation.
No speedup or hardware performance claim. Physical acceptance remains pending.

Build/host evidence and native results are in this directory. Previous1966
distribution and unrelated work are retained. No GitHub push or hardware access.
Stop after Batch3B.
