# Resumable repository updater — 18 November 2026

Build **2026111802**, Snow Macintosh Plus / 4 MiB / System 6.0.8, fixed maximum
acceleration (about 15×). Disposable copies of existing Snow disks; no SE access,
push, tag or release. Existing checkpoints and abandoned collection experiments
remain preserved. This replaces the proposed collection rollback workflow.

## Implemented

`clarice update [-y] [--check] [--source URL]` reads the configured private HTTP
repository, compares the flat installed catalog, adds new standard tools and
upgrades older versions. Successful tools are recorded individually. Failure
leaves `update.incomplete`; retries keep completed work. Core uses the existing
MacBinary extractor and native helper last, with completion after relaunch.
Normal startup checks only the marker. No resident network service/cache added.

Production paths: `src/commands/repository-client.inc`, `src/update/repository.h`,
existing `clarice.c`/helper, and a thin `cli_appinstall` runtime adapter.
The native installer still validates CLX and retains previous files and metadata.
Older recovery files are archived by HFS catalog ID. The helper handles confirmed
replacement interrupted before/after the target rename; unrelated identities fail.

`scripts/repository.py` consumes the complete build seal, publishes immutable
payloads then the flat index, and refuses a different payload at the same version.
Local catalog appends are closed and flushed after successful installation. A torn
final append/header is recoverable; malformed complete records are reported.

## Actual evidence

- [Host tests](host-tests.txt): production parser/catalog, bounded records and
  overflow, malformed/duplicate index, torn append/header, allocation failure,
  cancellation; 81 loader/installer cases including repository recovery; 482
  replacement model cases; helper recovery branches; plan/extraction/confirmation
  and sorted import lookup. ASan/UBSan used. Reproduce with
  `sh tests/repository/run-host.sh`.
- [Initial install/repeat](guest/repo.log), [failure after one completed tool](guest/partial.log),
  [resume](guest/resume.log), [corrupt payload rejection](guest/corrupt.log).
  Server requests show only the remaining tool was downloaded on retry.
- [Cancellation](guest/cancel.log): Command-period returned **130** during a throttled
  binary download. [Retry](guest/cancel-resume.log) succeeded; no partial tool installed.
- [Complete update](guest/full-update.log): all 47 standard commands adopted/updated,
  new updater installed, Core downloaded/extracted/verified, helper replaced and
  automatically relaunched **1801 → 1802**. [Relaunch screenshot](relaunched-1802.png).
- [Post-update checks](guest/final.log): marker absent, explicit CLX checks, awk,
  sort/wc pipeline, user sentinel retained, previous Core still present.
  [HFS readback](readback.txt): all 47 installed tool lengths/CRCs match the sealed
  repository. Final catalog matches all 47 tools and the Core package exactly.
- [Another partial update](guest/restart.log), application quit/reopen,
  [startup warning](incomplete-after-relaunch.png), [successful retry](guest/restarted.log).
  Only the still-missing package was fetched; completed tool survived restart.
- [Configured-source invocation](guest/default.log) passes using `clarice update -y`.
- Snapshot inspection confirms completion marker absent and all 26 recorded
  completed-download temporary files cleaned up. Snapshots were cloned while Snow
  was paused; only copies were mounted by host HFS tools.

Guest scripts are alongside logs. HTTP request log records fixture ordering.
Fixture names `repoone`, `repotwo`, etc. are disposable installation probes, not
new production utilities. Readback catalogs include those test names.

## Costs and boundaries

Full 47-tool no-change repeats: **556 / 553 guest ticks**, loader **51 / 49 ticks**;
free native heap **358046 B**, largest block **357008 B**, unchanged between repeats.
Core staging **1016 ticks**, staged verification **496 ticks**. These are accelerated
emulator guest measurements, not physical-SE performance claims.

Core MacBinary **271104 B**; external updater **113312 B**. Catalog after complete
adoption **2154 B** including test records. Repository records grow on demand during
invocation; no new permanent bitmap/network buffers. A complete clean build validates
32 distinct CLX images for 47 standard command names. Source-change rejection was
also observed during the earlier development build; the final build sealed cleanly.

Runtime **1.6** adds the existing installer entry and native SetEOF export. Older
Core needs one offline bootstrap. An ABI-breaking future tool collection is not
installed past compatibility checks merely to reach its Core update.

CRC/private HTTP is not authentication. The catalog does not detect undetectable
same-size corruption; explicit appinstall checking remains available. Recoverable
HFS replacement is not atomic or proven safe against arbitrary power cuts. Core
interrupted-rename branches have host fault coverage; no arbitrary power-cut test
was claimed. Recovery archives require occasional deliberate housekeeping. Physical
SE acceptance and the unrelated historical intermittent updater error 28 remain
unclaimed; this Snow handoff succeeded without that error.
