# Offline installer UX — 16 November 2026

Accepted checkpoint `c5075e5` was pushed to the existing development branch
`codex/core-distribution-2026111550`. Main, tags and releases are unchanged.

## User gap and implementation

The accepted distribution required copying Core and typing appinstall setup.
The disk now contains **Install Clarice**, a separate native launcher built from
existing Clarice services. Open it in Finder, click Install, then wait for the
installed application to open. The confirmation fits the Plus screen and warns
against resetting. Read Me is MacRoman/CR with TEXT/ttxt Finder identification.

The fixed destination is active System Folder:Clarice, beside Clarice Data.
Source files are siblings of the actual installer, independent of media label.
Absent-only appinstall setup installs the collection. The media's cp performs the
fork-preserving staged Core copy; existing updater checks/transaction/open guard
provide compatibility, verification, rollback and native launch. No new package
format or validation mechanism. Installer does not execute the user's profile or
save preferences; the installed Core does load its normal profile.

Existing utilities are never replaced. An older managed Core is retained as
Clarice Previous; equal/newer Core is not replaced. Recovery siblings block a
subsequent replacement until inspected/moved. Other app copies elsewhere are not
discovered or removed. Explicit appinstall repair/replacement is unchanged.

## Actual focused validation

Snow Plus, 4 MB, **reported System 6.0.7**, fixed maximum fast-forward approximately
15–16x. Automated keyboard submission temporarily used normal speed; guest
functional execution used maximum speed. No physical SE or System 6.0.8 claim.

- Fresh Finder install into an empty Clarice Data namespace; Ctrl+C stopped after
  six completed utilities. Relaunch/resume installed all 43 and automatically
  opened installed Core. pwd, ls /bin and awk executed; tracked remainder zero.
- Real insufficient-space failure: only 57344 B free on the disposable destination
  disk. Core copy stopped with failure, leaving original 1549 forks/metadata
  intact and no Clarice Installing sibling. After removing obsolete test apps
  from that fixture, the same installer successfully upgraded to 1550.
- Upgrade from the bundled 1549 app to Core 1550 automatically relaunched. Both
  installed forks match accepted Core, allowing only Resource Manager scratch
  header bytes 16–127 as in existing updater comparisons. Clarice Previous's
  forks, type/creator, flags and dates match original.
- Independent valid grep fixture with a distinct resource fork/Finder metadata
  remains byte-for-byte identical as MacBinary. Profile and binary user file
  unchanged. Launched Core exported the expected UX_PROFILE=kept value.
- Repeat installation with equal Core and retained Previous successfully opened
  Core again; recovery copy remains intact.
- Existing shared transaction host test: **482 scenarios**, address/undefined
  sanitizers; operation failures, busy files, rollback failures and backup refusal.
  No duplicate rerun of accepted 1550 full utility/repair/low-memory suite.
- `tests/install-ux/validate.py` checks exported evidence; results and hashes saved
  alongside this report. Fixture images and generated applications stay untracked.

## Cost and limitations

Ordinary Core resources, including every CODE resource, are byte-identical to
accepted 1550. No installer code/resources or extra checks enter normal startup.
Reuse the accepted 302 → 193–194 guest-tick startup comparison; this follow-up
claims no additional startup speedup. Post-upgrade Core reports 403202 B free,
401056 B largest block, zero tracked command allocations.

The separate launcher is 267904 B MacBinary, 267758 B
resource fork, no CLX payload, using the existing 768 KiB requested application
partition only while running. It is distribution media overhead, not Core's idle
allocation. No fresh full-install timing or physical performance measurement was
made in this UX acceptance run.

Cancellation resumes completed utilities. Abrupt reset/transaction leftovers can
require explicit inspection/recovery; no atomicity or power-loss safety claimed.
Core-stage errors after copying may retain the stage for inspection. Read Me is
correctly typed/encoded; opening it in TeachText was not tested in this fixture.
MultiFinder busy-app refusal and native relaunch on physical SE remain pending.
A Core installed outside System Folder remains untouched: use the managed copy.
See INSTALL.md and PENDING-HARDWARE-TESTS.md.
