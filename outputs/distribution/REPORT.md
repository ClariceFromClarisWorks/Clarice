# Core / utility separation — build 2026111550

16 November 2026. Host and fixed-fast-forward Snow Plus / 4 MB / **reported
System 6.0.7** acceptance. No physical-SE, 6.0.8, HTTP self-update or power-loss
acceptance is claimed. No executable-format, loader-policy or process-model change.

## Implementation

Core has no CLX resource payloads and never walks the utility collection at
startup. Namespace/preferences/history/profile initialisation remains. A shared
43-name/image list builds the external kit and drives explicit setup. Nano stays
optional. No resident cache or additional persistent allocation.

`appinstall setup DIRECTORY` streams absent commands through the existing staged
file installer. `install FILE NAME` is the single-command form. Existing files,
including damaged/incompatible ones, are skipped unchanged. Source-directory
validation happens once; cancellation is checked even while skipping files.
`repair NAME DIRECTORY` selects an external source explicitly. `replace`, full
`check`, receipts and `recover` retain their previous semantics. Existing
transaction siblings block replacement; setup stops on error and can be rerun.
Missing standard-command diagnostics point to setup/help.

The native offline application disk contains Core, Utilities and notices/licences.
It reuses the clean Apple Driver43 packaging tool, not a new distribution format.
A plain unpartitioned HFS image did not mount as Snow SCSI media; the final native
HDA was mounted and used successfully. See [installation](../../INSTALL.md).

## Size and startup

| Measure | Bundled 1549 | Core 1550 |
|---|---:|---:|
| MacBinary | 1,045,632 B | 265,344 B |
| Resource fork | 1,045,457 B | 265,142 B |
| Native CODE payload | 262,842 B | 263,551 B |
| Embedded CLX payload | 780,568 B | 0 |

Core package: **259.12 KiB**, 74.6% smaller. The external 43 files total 960,440 B
(multicall images are duplicated under their existing command names).
Minimum/preferred application partition remains 768 KiB.

Matched diagnostic measurements on the same disposable System/Clarice Data:

| Variant | C-main to ready, guest ticks | First draw ticks | Free heap / largest block, bytes |
|---|---|---|---|
| Old bundled core, existing-tool checks | 307, 302 | 309, 304 | 404634 / 401014 |
| New core, no payloads/checks | 197, 194, 193 | 199, 195, 195 | 404856 / 401048 |
| New core with payloads added back | 197, 193 | 198, 195 | 404526 / 401030 |

Old check phase directly measured **107 ticks** each run. Repeated ready time
falls about **36%**, primarily by removing that work. Payload-only comparison
has identical native CODE and every non-CLX resource; it shows no meaningful
startup speed benefit and only 330 B heap / 18 B largest-block difference.
Do not interpret 762 KiB removed from disk as resident-memory savings.

These are guest ticks, not host wall time or physical-SE seconds. Snow ran at
fixed maximum fast-forward (~15–16x displayed). Timings exclude Finder/Segment
Loader before C main and the report's own file writes. Old probe also reads a
one-byte run selector. Existing pre-investigation timings used a different
profile/history state; compare the matched rows above, not absolute values
between those fixtures. Diagnostic hooks are absent from the normal build.

## Actual validation

- ASan/UBSan: **77** loader/installer scenarios; no leaked allocations/descriptors.
  Includes failed/short writes, close/flush/rename errors, rollback failure,
  simulated interruption at flush boundaries, cancellation, validation OOM,
  incompatible/corrupt files, receipts and recovery. Existing loader also
  validated 31 real CLX images/import sets without execution. Shell smoke:
  31 comparisons and three cancellation cases; 32 builtin help topics pass.
- Normal core launched with an entirely absent Clarice Data twice. Final fresh
  setup from the mounted **Clarice HDA** installed all 43 files, verified afterwards
  byte-for-byte against the kit. **Awk installs and runs on the first attempt.**
  No locked embedded awk image competes with its validation allocations.
- Final fresh setup: 142.75 guest seconds (8,565 ticks), explicit operation only.
  `ls /bin`: 1.91 guest seconds including shell/redirection overhead; command
  execution 94 ticks, loader 59. Awk and sort/uniq pipelines execute normally.
- Existing bundled-install fixture and valid independently supplied command
  survived replacement of the core and restarts. Explicit setup kept damaged
  grep; `check` rejected it (126); repair succeeded; recovery restored the damaged
  original exactly. A leftover `.new` blocked setup and survived; moving it aside
  allowed retry. Source media without a valid directory now fails clearly.
- Real guest heap-pressure probe reserved 290,000 B: awk verification failed
  cleanly, no installed partial command; release restored 290,008 B including
  native allocation overhead, retry and awk execution succeeded. Probe-only
  reserve command is not in the production app.
- Guest Ctrl+C during install returned **130**; retry returned **0**. Host tests
  separately cover interruption/rollback boundaries; abrupt physical power loss
  is not simulated by an ordinary Ctrl+C test.
- Native HFS repair/recovery of an independently supplied command restored the
  entire MacBinary byte-for-byte, including a 37-byte resource fork, type/creator,
  flags and dates. Replacement inherited Finder metadata; old resource fork stays
  with the recoverable previous version, as before. Profile unchanged.
- Three final normal-build setup reruns: free heap 401140 / 401138 / 401140 B;
  largest block 397450 B throughout; tracked command remainder zero. Two-byte
  variation is bounded shell bookkeeping, not progressive retention.

## Boundaries and reproduction

Build with `scripts/build.sh`; create offline media with
`scripts/distribution-disk.sh [existing-Driver43-template]`. Generated binaries
are local under `outputs/distribution/build`, not committed. Artifact hashes,
resource census, metadata comparison and guest transcripts accompany this report.
`tests/distribution/prepare-probe.py` creates isolated timing/heap diagnostics;
`guest.sh` exercises them; `validate.py` checks captured evidence. Matched packaged
probe adds the legacy Commands.r to its copied CMake input only.

Complete collection installation is not atomic: completed commands remain after
failure and rerunning fills gaps. Recoverable leftovers require inspection.
No automatic downgrade, repair, utility upgrade or deletion of backups. No new
power-loss guarantee. Existing updater still replaces the MacBinary application;
this task does not retest its unrelated handoff issues or add utility updating.
