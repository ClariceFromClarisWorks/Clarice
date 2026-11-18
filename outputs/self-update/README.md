# Built-in self-update — 18 November 2026

Authorized after the PB180 reached Core 2026111882 while retaining a legacy
external updater. That updater limited its old single-application manifest to
1023 bytes, rejecting the modern repository before parsing it. Updating Core
alone did not repair `/bin/clarice`.

Separate tree `Clarice-Self-Update`, branch `codex/self-update-20261118`, based on
committed integration dd96ea8. No other agents' uncommitted work incorporated.
Physical SE/PB180 and other running Snow instances are untouched by this change.

## Design and costs

`clarice` is now a shell builtin. The application carries the existing updater
as a demand-loaded native `CLXU` resource, bundled with the exact Core build.
No `/bin/clarice`, external wget, separately installed updater or initial network
fetch is required to run it. Core updates include builtin behavior; the repository
continues to add/update independent external commands. Old repository `clarice`
entries are ignored; old local files are preserved rather than deleted.

This uses the native Resource Manager and existing CLX validation/relocation
implementation, not another loader. The resource is locked during execution,
validated fully, relocated in its owned memory, then released without marking it
changed. It is neither preloaded nor retained as a cache. No image duplication or
extraction file is needed. The on-demand resource retains its relocation metadata
until completion (about 13 KiB beyond the executable image); normal external
modules still release their metadata before execution. The tiny resource wrapper
and builtin dispatch are resident; the updater payload is not.

The native handoff helper remains necessary to close and replace a running
classic Mac application safely. It is embedded within the update implementation,
prepared automatically and retains the existing replacement/recovery behavior.
It is an implementation detail, not a separately acquired or installed product.
`appinstall` remains available for explicit repair and offline tools.

Startup update completion now invokes the builtin, removing its old absolute
`/bin/clarice` dependency. Remote updates remain refused under the existing local
input-ownership restriction; help works remotely. HTTP/CRC policy, permissions,
recovery backups and newer-package preservation are unchanged. No automatic
startup network polling or wholesale installation of unlisted optional tools.

Historical native resource/CLX embedding in `scripts/command-resource.py` provides
the packaging precedent. Retro68 Rez does not accept the attempted `read` form;
the final build uses that existing hexadecimal `data` encoding. Build sealing
validates the updater's imports and checks that the application resource exactly
matches the validated payload.

## Validation

Host ASan/UBSan covers the actual loader on embedded input: valid execution,
truncation, corruption, cancellation, allocation failure and caller ownership.
Native-resource shims cover missing/unloaded resources, current-resource-file
restoration, unlock/release on error and remote ownership. Existing repository,
installer, rollback, plan, fork-copy and import-binding tests also pass. See
`host.txt`. These do not substitute for native HFS execution.

Snow fixture: private copies of stopped additions disks; Macintosh Plus, 68000,
4 MiB, System 6.0.8, requested application partition 768 KiB. Dedicated
`org.clarice.SelfUpdateSnow`, Telnet 23246, HTTP 8100. Secrets remain untracked.
Core 1882→1901 update and automatic completion were observed through the actual
native handoff. Then `/bin/clarice` was moved to `/tmp/su-old-updater`; repeated
builtin help succeeds, and a repository installs test command `selfprobe` while
an obsolete external-updater record points deliberately to a nonexistent file.
That obsolete payload is not requested. Native guest checks continue below.

First recorded help after migration is not cold: startup has already invoked
builtin `--finish`. Baseline external updater help: 1.510/1.271 host seconds;
builtin: 2.768/2.759 seconds, with concurrent host compilation. Guest module ticks
baseline 73 first; builtin 165/167, including 157/158 ticks full payload validation.
The module timing excludes native resource acquisition; host command timings
include it. These are diagnostic observations, not a controlled speed comparison.
Free heap after repeated builtin calls is stable at 332816 bytes, largest 331930;
no retained updater image. Baseline post-help free heap was 333724. That 908-byte
difference also includes the newer integration Core, so it is not an isolated
idle-cost measurement. Resource payload 113568 bytes; image 100604 bytes.

## Migration and limitations

The user explicitly excludes legacy migration support before v1.0. No bootstrap
feed is being published or maintained for this task. An older development build
that cannot consume the repository can have its application replaced directly.
The prepared legacy manifest under ignored work/ is an unused experiment.

This does not incorporate the other agents' pending updater recovery, mounted
source or Telnet changes. Reconcile those before Main integration. Physical
hardware, System 7/MultiFinder, exhaustive native disk-full/power-loss and the
broader future ABI transition remain outside this Snow acceptance. Repository
orchestration currently installs tools before Core and still rejects incompatible
tools; this change does not introduce a dependency solver or claim compatibility
with an arbitrary future runtime transition.

## Completed bounded repository checks

With no external updater installed, native installation of `selfprobe` version 1
succeeds. Replacement with version 2 succeeds and retains the old file. A corrupt
version 3 payload returns status 1; version 2 remains runnable. Repeating a
version 1 feed preserves version 2 and clears the incomplete marker. Restoring an
older external updater still leaves `command -v clarice` resolving to the builtin.
No obsolete updater payload was fetched. See install.json and regress.json.

Command-period returns 130 and subsequent builtin help works; the retained module
counters are from the preceding invocation, so this does not establish the exact
cancellation phase. Host tests separately cover resource/loader cancellation.
The required native helper was absent before the builtin Core handoff;
preparation and relaunch succeeded automatically. See no_dependencies.json.

The initial 1901 diagnostic and subsequent clean build differ in generated build
metadata. They are not advertised as byte-identical packages. Final candidate uses
a new identity, 2026111902, with a fresh full build and seal; no already published
version is overwritten.

The final 1902 build seal validates 35 distinct command images/import sets
(including the bundled updater) and stages 49 independent command names. The
application is 394112 bytes packaged; package.json and build-seal.json identify
it exactly. Its full repository index is 5579 bytes, deliberately above the old
1023-byte manifest limit that triggered this task.

Before the final handoff, both `/bin/wget` and `/bin/clarice` were moved aside, and
the builtin's exact helper name was confirmed absent. The built-in command is
responsible for fetching the full repository, restoring wget as an ordinary tool,
preparing its helper and replacing Core. No external updater is in the new feed.

The final full-repository update relaunched build 2026111902 and displayed
“Clarice update complete.” in the isolated Snow guest. There is no startup
version check against `/bin/clarice`: the bundled updater comes from the same
application build by construction. A stale external file cannot shadow the
builtin. This adds no startup filesystem scan or network polling.

Final catalog validation matches Core and all 49 external command identities
(version, size, CRC). The unlisted selfprobe remains version 2. The native helper
was prepared automatically, wget restored, and `/bin/clarice` stayed absent.
Repeated final-build help leaves free heap stable at 332816 bytes. A subsequent
full repository update reports all tools current, with no incomplete marker:
15.48 guest seconds for the complete no-change operation in Snow. This is not a
physical SE timing or a startup cost. See final.json and repeat.json.

The isolated guest was closed cleanly and its test HTTP server stopped. Other
agents' workspaces and user-facing update servers were left untouched. Candidate
is not merged, pushed or published. Recommended next step is review/integration
with the concurrent updater fixes, followed by physical System 6 validation.

## Physical PB180 follow-up

User-authorized System 7.1 test passes: Core1882→1902 automatic handoff, builtin
help and repeat repository update with the external updater archived, cleanup
and utility smoke checks. Help0.31/0.26s; no-change update3.10s. This is 68030
PB180 evidence, not stock-SE acceptance. [Full record and limits](pb180/README.md).
