# Installer C — 26 November 2026

Build **2026111978**, product **f571f7c**, integrated local main. Existing native
installer now offers Uninstall. Application-only is the default and keeps all
Data and the location record. Full deletion requires a separate unchecked option,
a permanent-loss warning and a Remove click; Return cannot confirm removal.

Identification reuses the location codec, installed catalog, Core stamp and native
open-file guard. Invalid/inconsistent installations and pending updates are
refused. Legacy discovery remains supported; after confirmation it records the
identified locations using Installer B's existing writer. Native catalog IDs and
literal leaves drive deletion, without Unix resolution or following links.
The catalog survives until all other Data contents are removed. Interrupted final
cleanup accepts only an empty protected scaffold with a valid location record and
absent Core. No new transaction format or snapshot mechanism. The running installer
and active System Folder cannot be contained in Data being removed.

## Acceptance

- ASan/UBSan native/dialog tests pass: 55 I/O boundaries with injected I/O errors,
  disk-full errors and resets, 20 cancellation positions, retry, locks, invalid or
  missing authority, foreign Core, special native names and outside-tree sentinels.
  No leaked references or invocation allocations. Installer A/B and existing
  recovery fault matrices pass unchanged in behaviour.
- Normal 68000, 4 MB System 6 Snow, fixed maximum acceleration: custom app-only
  removal, custom full removal, recordless legacy full removal, native locked-file
  error **-45** and subsequent successful retry, invalid-record refusal,
  Command-period cancellation, and reinstall at the retained custom location pass.
- Cancel at both confirmation stages leaves the entire system disk byte-identical;
  the final normal candidate also passes Command-period at full confirmation.
- **101 Data files**, both forks and MacBinary Finder metadata, are byte-identical
  after app-only removal. Full removal leaves the sibling two-fork Sentinel exact,
  and retains unrelated Saved Clarice/Saved Previous applications.
- Reinstalled Core and all49 utilities read back against the seal; full installed
  catalog and selected-location record verified. Ten command runs have remaining=0,
  free heap349578–349582, largest block349370 (stable; small live argument variation).
- Final HDA Core/Installer/Recover and49 utilities read back correctly; USTAR
  payloads match the sealed kit. Staged repository: **51 entries /36 payloads**.

## Costs and limits

Core and helper CODE/package sizes unchanged. Installer CODE **+4861 bytes**,
MacBinary **+4992 bytes**. Requested partition remains768KiB. New installer-only
state: three integers; removal uses a768-byte bounded traversal stack and no
new dynamic allocations or permanent buffers. No ordinary Core startup work.
Peak native File Manager/Resource Manager allocations and installer latency were
not separately instrumented; command heap evidence is not a measurement of those.

Removal is not atomic or undoable: Cancel stops further deletion, not prior work.
Locked files require explicit Finder unlocking before retry. Invalid catalogs or
pending updates require existing repair/recovery first. Trees deeper than128
levels fail safely and may require manual handling. System7 and physical hardware
are untested; System6 Snow evidence does not establish physical6.0.8 acceptance.

## Distribution

Verified build is staged at `work/installer-c/publication`, with immutable source,
seal, offline HDA/USTAR and checksums. **Not published:** Macintosh Exchange is
unmounted. Canonical HTTP remains byte-identical to accepted1977; neither active
index was advanced, so no partially published generation is presented as current.
Remount the existing share, then publish these verified artifacts through the
existing mirror procedure. Previous distribution/recovery material and unrelated
work are preserved. No hardware access or GitHub push. Stop after Installer C.
