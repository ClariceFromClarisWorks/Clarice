# Installer A 2026111976

Based on accepted product a0efcc5, main evidence HEAD 49779e4. Product **9ef7ed7**
is integrated into local main. System 7 acceptance was deferred by the user.
No physical access, GitHub push, destinations or uninstall work.

## Implementation

The installer enters a native Dialog Manager welcome immediately after Toolbox
initialization. Cancel precedes namespace creation, history/profile handling,
terminal setup, preferences and remote services. Install initializes only the
existing namespace/backend. Native phase labels and a 53-step bar report package
progress; steps are not time estimates. Existing copy, verification, catalog,
snapshot and handoff routines remain in use. The existing cooperative backend
yields service only the dialog, with bounded event draining and sticky Cancel.
No alternate installer transaction or recovery format was introduced.

## Observed acceptance

- Host: `sh tests/installer-a/run-host.sh`, ASan/UBSan actual dialog controller;
  five pre-install cancellation forms, allocation failure, single execution,
  initialization/backend errors, cancellation and bounded messages.
- `sh tests/recovery/run-host.sh`: existing snapshot/restore fault-injection,
  locked files, fork checksums, corruption, interruption/retry and rotation pass.
  Raw results in recovery-host.txt; this is reused backend coverage, not a new
  graphical disk-full Snow test.
- Normal 68000 Snow Plus, 4 MB, System 6.0.8, maximum fast-forward: Cancel at
  welcome leaves the complete startup image byte-identical. Both initial and
  final1976 candidates checked. No Clarice Data existed in the fresh fixture.
- Fresh installation interrupted by the Cancel button, and separately by
  Command-period, retained completed utilities and update.incomplete. Retry
  installed all49 utilities and launched Core. Final1976 fresh launch captured.
- Upgrade1975→1976 was forcibly interrupted with Recovery Preparing present
  (53 members), before Core replacement. Relaunching the installer resumed and
  launched1976 with update-complete confirmation. Old accepted Core was intact
  at the interruption. Existing recovery tests cover finer transaction edges.
- Reinstall of1976 launches the existing Core and retains byte-identical
  previous-generation Inventory. Readbacks verify Core and all49 utilities,
  catalog1976, a user document with real resources/data and Finder codes, and
  historical applications unchanged. `python3 tests/installer-a/verify.py`.
- `scripts/release.py check` passes. Offline HDA readback validates Core,
  installer, independent helper,49 utilities and Release; USTAR payload set
  equals the sealed kit. Canonical HTTP/AFP now match the sealed1976 build:
  51 manifest entries/36 payloads, source archive and offline packages verified.
  Previous1975 index and every old payload remain verified and available.
  See verification.json and publication-verification.txt.
- Existing profile MacBinary is byte-identical across the upgrade (both forks,
  Finder fields and dates), in addition to the user-document check.

## Size, memory and timing

| Item | Accepted1975 | Candidate1976 |
| --- | ---: | ---: |
| Installer MacBinary | 463488 | 435712 bytes |
| Installer CODE resources | 309065 | 281221 bytes |
| Core MacBinary | 440064 | 440064 bytes |
| Core CODE resources | 285947 | 285947 bytes |
| Requested application partition | 768 | 768 KiB |

No new Core allocation or startup work. Dialog state is installer-only, including
a256-byte bounded status-line buffer, and native dialog storage is disposed on
Cancel/error exit. No terminal bitmaps are initialized before the decision.
Diagnostic-only confirmation measured34 guest ticks from main entry through
DrawDialog (~0.57 guest seconds); first window and confirmation coincide.
This excludes Finder loading and is one sample, not a physical-speed claim.
Before MaxApplZone, free/largest were4554/4422 bytes: these are current small heap
extent values, not total available application capacity. After installed Core
launch, free-after-purge was376928 bytes, then378220 on the next launch; heaps
were712660 bytes. Different command sequences mean these are capacity snapshots,
not a matched leak benchmark. Installation peak heap was not separately sampled.

## Remaining acceptance

System 7/MultiFinder execution is untested; the user explicitly deferred it.
Acceptance is host/System6 Snow only, not physical or System7 acceptance.
System 6 changes do not establish a fix for the separate intermittent updater
handoff address error. Original crash evidence and published recovery material
remain untouched. Working clones/readbacks are under work/installer-a.
