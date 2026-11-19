# Clarice current state

## vi response experiment — 19 November, Snow only

Isolated `codex/vi-refresh-audit-20261119` preserves the accepted1905 port,
safe saves/recovery and upstream1.37.0. Conservative clean-screen invalidation
lets known non-mutating motions skip unchanged viewport formatting. Normal CLX
51392B (+148), invocation peak23396B (+2), no idle allocation or functionality
removed. Optional profiling is compiled out normally.

Companion Core cursor-only drawing is on `codex/vi-response-audit-20261119`.
Together, matched local Snow vertical event-to-presentation232–233→83–85ms;
horizontal180→52ms. vi-format stage752→61ticks in the diagnostic case. These are
guest ticks, not15× host-time or physical acceptance. One-tick goal remains unmet;
status/output and full viewport redraw dominate the remaining costs.

52 real-guest incremental/forced-redraw comparisons pass, including queued edits,
undo, settings, long lines, scrolling and motion. Actual growth failure, save and
recovery host tests pass. Companion normal Core1942+normalvi passes exact20j/20
arrows,120-key burst/save, Ctrl-C and Command-period130 recovery with remaining0.
No share/hardware deployment, push or release. [Evidence](vi-response/README.md).

## BusyBox vi completion — build 2026111905, Snow only

Branch `codex/busybox-vi-completion-20261118`. Optional upstream BusyBox1.37.0
now installs as `vi` through the existing repository/updater; `bbvi` remains a
comparison alias in the private fixture. Core/standard utilities unchanged.

- Preserves unsaved edits on orderly disconnect/cancellation/failure before runtime
  cleanup, including pending undo-queue text. Exclusive `.part` creation in
  `/etc/clarice`, checked writes/close, native rename to `.recover`. Original never
  overwritten. Best-effort writable-disk recovery, not crash/power-loss journalling.
- Growth failure preserves old pointers/length; deterministic ASan/UBSan failure,
  overflow and recovery-I/O tests pass. Existing shared safe-save27 failure/
  interruption cases, metadata/forks and repeated saves pass.
- Final `vi` passes43 Snow editor/save/error observations and exact8KiB save.
  Local-window two-line edit/Ctrl-[/save/readback and terminal restoration pass;
  host Escape injection did not reach the Snow Plus guest (no Core mapping change).
  Disconnect/cancel recovery exact-content comparison, reconnect and subsequent
  editing pass. Three512KiB OOM opens recover with native heap357916/largest355660
  unchanged and remaining0. No physical-SE/System7 acceptance claimed.
- Retained -Os/LTO: package51244 B/image36176 B/953 relocations. Recovery-only
  ordinary -Os was57992 B; -O2 was81696 B and rejected. Matched Snow loader
  ~54–56→42–46 ticks; median screen-ready1.138→0.972s empty,1.701→1.451s8KiB.
  Host timings include Telnet; original pre-recovery timings remain similar.
  Working peak21266/23314 B unchanged, no permanent Core allocation. Clean
  rebuild byte-identical. First8KiB insertion still~0.25s; no physical speed claim.
- Literal search/limited ex, byte text, RAM-resident buffer/undo, remote80x24;
  no shell filters/job control/automatic.exrc. Nano/other editors remain available.

[Build, installation, evidence and limits](busybox-completion/README.md).
Physical machines, shared repository, main and release candidate untouched.



## BusyBox vi evaluation — 18 November 2026, isolated branch

- Optional genuine BusyBox1.37.0 `bbvi`, build2026111871, installed through
  Clarice update in private Snow. No Core/sbase/other utility replacement,
  physical SE use, merge or push. Branch codex/busybox-evaluation-20261118.
- 36912 B loaded image (78% below Heirloom167504 B), 56704 B package,
  1352 relocations. Same runtime1.9/raw terminal and safe HFS save services.
- Three matched Snow runs: median8KiB ready1.884s vs Heirloom6.248s/xvi11.910s;
  whole open/insert/discard3.026s vs6.849s/12.810s. Not physical timing.
  First visible insertion is slower:267–332ms vs Heirloom61–74ms.
  Working peak21266 B empty/23314 B file; remaining0 throughout.
- 43 guest observations and full8192 B save equality pass; host sanitized save
  failure injection passes. Clean final rebuild byte-identical. Literal search
  only; limited ex, no process/filter/autoexrc or unsaved disconnect recovery.
- Keep sbase/awk/shell/network. Shortlist additional applets individually:
  base64, diff, then safe patch; standalone compression needs separate assessment.
  This evaluation does not port or authorise integration of those commands.
- [Full evidence and limitations](busybox/README.md),
  [Telnet observations](busybox/TELNET-FINDINGS.md).


## Original vi versus xvi — isolated Snow comparison, 18 November

Separate `codex/upstream-vi-comparison-20261118` branch based on xvi investigation;
genuine Heirloom/Berkeley vi pinned at `3f1e9786d249`. Optional vi 2026111864 installed
through Clarice update; Core 2026111823 and telnetd 2026111814 unchanged. No SE use,
merge, push or release. 256780 B package / 167504 B image versus xvi's
156044 B / 99436 B. O2 LTO brings vi within existing CLX limits.

Matched interleaved Snow/Telnet tests: 8 KiB full-screen startup vi **5.3–5.5 s**,
xvi **9.1–10.3 s**; empty vi **2.3–2.9 s**, xvi **2.2–2.4 s**. Emulator only,
not physical performance acceptance. Vi startup output is roughly eight times
smaller; its larger executable does not imply slower file interaction.
44 guest regression observations and host safe-save/temp-I/O sanitizer checks
pass, including full 8 KiB save byte comparison. Clean rebuild reproduces the
installed package. HFS temp seeks, open-file unlinking and read-only detection
adapted without Core changes. Unsaved disconnect recovery remains a production
blocker; not a production-complete port. See [comparison and measured costs](vi/README.md)
and [Telnet findings](vi/TELNET-FINDINGS.md). Recommended next performance work:
xvi terminal adapter improvement, then repeat the same comparison; not started.

## xvi investigation — separate branch, Snow only, 18 November

Experimental genuine STEVIE-derived xvi port on `codex/xvi-investigation-20261118`;
base `7e3e036`, with runtime 1.9 dependency supplied by the newer local guest
Core 2026111823. No Core/Telnet source changes, merge, push or SE access.
Installed optional xvi 2026111841, then help-only 1843 via `clarice update`
in isolated Snow. Final package 156044 B; measured image 99300 B.
Telnet insert/search/substitution/save/reopen and locked/readonly protection pass;
host safe-save adapter sanitizer/failure tests pass. Remote emergency preservation
is a production blocker: cancellation bypasses upstream hangup recovery.
Local save and final-package Telnet smoke pass. Resource data/map/application
area preserved; 39 system-reserved header bytes differ, explicitly recorded.
Not production-complete.
See [investigation, costs and remaining validation](xvi/README.md) and
[Telnet/runtime issues for the ongoing rework](xvi/TELNET-FINDINGS.md).


## Password-protected supplementary Telnet — Snow validated

Core **2026111810** (runtime 1.8), optional `telnetd` **2026111811**, deployed through
new repository manifests and `clarice update`, including automatic Core relaunch.
No SE access or push. Service disabled after acceptance; no pending repository upgrades.

- Preferences → **Telnet server…** controls the allowed IPv4 peer, port, masked
  password and enable/disable. Also `telnetd --preferences`. Settings persist in
  `/etc/clarice/telnet.conf` through existing fork-preserving safe replacement.
  Blank password preserves its verifier. Server intentionally stays off after
  Clarice restarts; `telnetd` starts it with the saved password settings.
- Genuine libtelnet retained. Password checking uses Joseph Birr-Pixton's CC0
  fastpbkdf2 with existing public-domain libutil SHA256, inside the optional module.
  Salted verifier only; no plaintext password in settings, history or argv.
  No shell before authentication; no password echo; overlong input rejected;
  60-second login deadline and five-second failed-login cooldown. Explicit
  `--no-password --peer` remains available for controlled tests.
- Plus/4 MiB/System 6.0.8 Snow, fixed maximum acceleration (~15×): wrong/empty/
  overlong passwords, password change, no pre-auth execution, cancellation and
  reconnection pass. Preferences save/reopen/port change/Cancel/restart persistence
  pass. Locked settings save preserves original bytes; malformed settings refuse
  startup and recover after restoring the saved file. Local input/scrolling,
  serialized commands, streams, interrupts and lifecycle regressions remain passing.
- Host ASan/UBSan, upstream KDF known answer, allocation/cancellation/recovery tests,
  31 shell comparisons + 3 cancellation cases, and 193/195 import checks pass.
- Enabled image **27,928 B**, native driver storage unchanged **20,692 B**, plus
  demand-owned session/protocol state. Three enable/stop cycles: heap
  **365156 → 365154 → 365156**, largest block **363042** throughout; no progressive
  loss. No disabled-service networking allocations. Core MacBinary +1,408 B versus
  1804; package size is not a RAM/startup measurement. Final authenticated login
  **1.42–1.47 s host elapsed at accelerated Snow speed**, not physical-SE timing.

Telnet is **unencrypted**. The 1,000-round PBKDF2 setting is a legacy CPU compromise,
not contemporary offline password hardening; use a unique strong password on a
trusted network. HFS provides no credential-file access protection. Physical SE,
MoofTCP and System 7 acceptance remain pending. No serial remote sessions or GUI
editor support added. [Evidence and usage](remote-sessions/README.md).

## Resumable repository updater — Snow accepted, build 2026111802

18 November: the simpler per-package updater now works end to end in disposable
Plus/4 MiB/System 6.0.8 Snow at fixed maximum acceleration (~15×). SE untouched;
no push, tag or release. Remote-session development is separate.

- `clarice update [-y] [--check] [--source URL]` compares a private HTTP repository,
  adds newly listed tools, preserves known newer versions, and installs tools
  through existing appinstall before replacing/relaunching Core with its helper.
- `/etc/clarice/installed.catalog` records each successful package; an
  `update.incomplete` marker survives failure and clears only after tools complete
  and, when required, the new Core relaunches. Retry retains successful tools.
  Torn final catalog writes are repaired; malformed complete records fail clearly.
- Source is `/etc/clarice/update.conf`. Publisher `scripts/repository.py` accepts
  only a complete sealed build, publishes immutable CLX/MacBinary payloads before
  the index, and rejects changed contents at an already published version.
- Normal startup checks only the marker, without a utility scan or network access.
  No persistent networking/cache allocation added. Runtime 1.6 exposes the existing
  installer and native SetEOF; older installations need one offline bootstrap.

Actual guest acceptance: initial installation, repeat/no-download runs, HTTP 404
  after a successful tool upgrade, resumption skipping that tool, corrupt download
  preservation, Command-period cancellation/status 130 and retry, incomplete-marker
  warning after application relaunch, full 47-tool catalog adoption/update, Core
  1801→1802 replacement and automatic relaunch. Native HFS readback matches all 47
  expected tool lengths/CRCs. Previous Core and per-tool recovery files remain.
  Existing awk, sort/wc pipeline and explicit executable checks pass.

Host acceptance: actual catalog/index code under ASan/UBSan (torn records/header,
  malformed/duplicate input, allocation failure/cancellation), 81 loader/installer
  cases including repository retries, 482 native-replacement model scenarios,
  helper recovery decisions, plan corruption, both-fork extraction failure paths,
  confirmation and import-table tests. Clean sealed build validates 32 distinct
  CLX images for 47 standard names. This is not physical hardware acceptance.

Measured guest repeats: full repository/no-change update 556/553 ticks, loader
  51/49 ticks; native free heap 358046 B and largest block 357008 B unchanged.
  Catalog after complete adoption: 2154 B including disposable test entries.
  Core MacBinary 271104 B; updater CLX 113312 B, loaded only for the command.
  These accelerated guest ticks are comparative, not stock-SE wall-clock claims.

Limitations: private HTTP CRC provides corruption detection, not authentication;
  catalog is not continuous same-size corruption detection (use appinstall check).
  HFS replacement is recoverable, not collection-atomic or power-loss-safe.
  Recovery archives consume disk space. Future incompatible runtime changes may
  need an offline bootstrap. Core interrupted-rename decisions have host coverage;
  arbitrary resets/power cuts during HFS metadata writes are not guest acceptance.
  Prior whole-collection experiment remains preserved locally but is not wired
  into this updater or startup. Existing older reports below are historical.

[Installation and source hosting](../INSTALL.md) ·
[Guest and host evidence](repository-update/README.md)

## Earlier supervised deployment — historical physical-SE baseline

18 November: supervised complete utility deployment now passes on physical SE
and Snow, both retaining the accepted diagnostic Core2026111713. All47 standard
utilities transferred in one bundle, fully loader-checked before installation,
replaced using existing appinstall, and read back against manifest lengths/CRCs.
Physical preferences/profile comparison and command/pipeline smoke checks pass.
Previous utilities retained; six older recovery files archived through Clarice.
Optional nano/user files were not replaced. [Evidence and exact scope](coherent-deployment/current-se/README.md).
The diagnostic Core differs from the normal Core in the generated bundle; this
is a compatible supervised development deployment, not whole-manifest Core binary
identity or acceptance of the future coordinated updater.

Branch codex/coherent-deployment from main110f937; transaction approach approved.
Host packaging checkpoint pushed; main110f937 also pushed at user request. No
release or tag. Root causes remain skip-existing setup, Core-only HTTP updates,
unbound utility manifests and reusable build outputs in the previous workflow.

Implemented: fresh full-build directory (previous build retained), invalidated
seal before rebuild, partial-command override disabled for full builds, source/
build-bound generated manifest, production-loader validation of actual CLX files,
Core MacBinary/resource/build checks, kit-content checks, and one plain USTAR
bundle. 47 commands/32 distinct CLX images passed a clean normal build. Bundle
1351680B; Core270464B; utilities1031308B. No decompressor on the SE.
[Evidence](coherent-deployment/build-evidence.json),
[host tests](coherent-deployment/host-tests.txt).

New collection transaction model passes42 injected failures and42 interruptions;
1564B native manifest passes every-byte corruption checks. Restricted upstream
microtar wrapper passes complete real bundle plus24 malformed/truncated archive
cases under ASan/UBSan. Original MIT source/licence retained. These are host
checks, not native filesystem durability acceptance. Native file adapter compiles
against pinned68000 headers but is not yet connected to production installation.

18 November follow-up (SE off): added a bounded recovery-journal decoder and
native exact-length/close-checked reader. Experimental journal v2 records ordered
command names, preventing a changed inventory from reinterpreting saved identities.
The never-deployed v1 format is rejected. Host ASan/UBSan tests pass every-byte
corruption, every truncation, wrong paths/inventory and unchanged output on failure;
the collection model still passes 42 failures and 42 interruptions. The codec
passes in accelerated Snow Plus/System 6.0.8, status 0, tracked remaining 0.
Native reader compiled against 68000 headers; actual HFS journal recovery is not
accepted yet. Record: 3,628 bytes, no heap allocation in codec; no idle overhead.
[Focused evidence](coherent-deployment/journal/README.md).

Outstanding: native staging, completion-marker/recovery handling and startup gate; integrate
bundle with offline installer, appinstall and HTTP updater; managed-file ownership
and legacy conflicts; test actual native failure paths, cancellation/low memory,
repeat upgrades and Snow. Existing updater/installer still have their old behaviour.
Do not treat this bundle as a completed automatic updater or claim future
release consistency is guaranteed. The supervised deployment above uses existing
per-command recovery, not the unfinished collection transaction.
No new startup memory, runtime checks or executable-format changes are active yet.

Next authorised milestone is [Remote Sessions](../references/remote-sessions-2026-11-18.md),
conditional on completing deployment consistency. Investigate/reuse first and
present the technical proposal before remote-session production changes. SE is
off; continue Snow only, no push/publish/tag/hardware deployment. Physical remote
session acceptance remains required later and cannot be inferred from Snow.

## Recovery steering — separate development helper

17November: shared Clarice tools/se-control now has a Snow-tested reconnect
candidate; [report](se-control-recovery/REPORT.md). Partial
input/drag/capture loss, listener restart, repeated heap checks and offline Quit
pass. No renderer/RC changes for recovery. Helper now installed through Clarice;
two physical disconnects reconnected unaided in5.94/6.16s, heap26802 stable.
Old helper application retained; new socket in shared Clarice checkout at
work/se-control-recovery/physical.sock. Serial adapter now present; independent serial maintenance and safe
port arbitration remain a separate architectural step, not an existing shell
server or a general MacTCP/process restart facility.


Disk cleanup now archived25 old builds/packages through Clarice into
Macintosh HD:Clarice Archive 20261117. Normalized fork-length/type/creator/flag
manifests compare equal; active app/two recent recovery copies/helpers retained.
Older installed ls lacks -d and cmp is absent; verified with existing shell
facilities instead. No permanent deletion or utility upgrade; staging stays in/tmp.

## Terminal renderer — build 1713 integrated for development

17 November: user-authorised main integration of the 34 hash-verified build1713
source files; original RC branch/tag unchanged. Earlier draft source is retained
in its own checkpoint. Focused viewport/HFS and SEControl regressions pass.
No new binary, hardware session or release acceptance is implied by this merge.
[Report and evidence](viewport/REPORT.md). Viewport/prompt composition, resize
anchoring, long-input reflow and clipped scrollbar staging pass focused host/Snow
tests. Cross-volume preference rename/flush corrected; native negative/recovery
and Snow theme checks pass. Automatic paging removed; explicit `more` advance,
quit and cancellation pass. No commits, push or release.

Physical1703's faster row painting still produced a user-observed top-down wipe.
Snapshot1704 was rejected for slower total redraws. Direct composition1708 avoids
snapshot/per-row copies;1709/1710 remove redundant row validation after whole-view
validation. **Physical1710 paired medians for24 large jumps:549→447ticks light,
613→494dark (18.6%/19.4% faster)**. Sparse medians299→285/347→333 show no systematic
regression. [Actual trials, limitations and interaction results](viewport/direct-1710/README.md).
Glyph composition262–281ticks per23frames dominates; final transfer30–40ticks,
dark inversion43–48. No claim of VBL-atomic presentation.

Physical35pixel cases,22scrolling bursts/20views and147scrollbar comparisons pass.
Native pressure/recovery passes: owned0, freeheap unchanged, largestblock−158B.
Payload46,208B plus Handle bookkeeping, demand-allocated/purgeable; command
admission requests native purging only when its existing preflight would refuse.
Physical Ctrl+C and Command-period each return130/recover; ordinary ls, selection,
scrollbar dragging and typing-to-live pass. No old SE applications deleted or
renamed. Previous working copies remain recoverable.

Snow nano edit/discard/reopen restores terminal/status0, stable freeheap/largest,
tracked remaining0. Telnet full-screen less, exit, reconnect and local cancellation
restore the shell; exact cancellation status was obscured by automation errors.
Physical **visual** acceptance is still pending: screenshots cannot resolve brief
flashes. A user-observed boxed-minus mouse pointer moved with the mouse; switching
to Finder and back restored the arrow, and a subsequent scrollbar drag remained
normal. Trigger unresolved; [evidence](viewport/direct-1710/physical/pointer/README.md). Root graphics drafts are not integrated; RC remains untouched.

The next isolated experiment extends complete-screen composition to completed,
noninteractive bulk output, preserving incremental/sparse fallback.1711 passes
293 Snow pixel comparisons.1712 refines the same renderer’s timings to exclude
fixture generation: six dense48-line batches133→115ticks light/147→122dark
(13.5%/17.0%); clipped variants also improve, small/sparse cases remain near
unchanged. [Raw results and reproducible source overlay](viewport/direct-1712/README.md).
1712 physical matched dense48-line output now passes293pixel comparisons:
155→135ticks light,183→150dark (12.9%/18.0%); painting124→104/152→119.
Ingestion unchanged31ticks. Clipped dense cases also improve. One paired batch
per case; fallback noise −11..+18ticks, no small-case speed claim. No RC promotion.

Physical1712 **System6 clipboard** checks pass: actual mouse selection followed by
Command-C/V and Edit Copy/Paste; copying historical text returns to the live
prompt without execution. [Screenshots and scope](viewport/direct-1712/physical/clipboard/README.md).
No clipboard implementation changes. The user-reported PowerBook failure with
both methods remains unreproduced; PowerBook unavailable, System7 investigation
deferred. Up-arrow transient duplication has a focused prepare-row-before-move
prototype1713:48 Snow pixel comparisons and12 unchanged-before-move checks pass.
Whole-scroll time is approximately unchanged; exposed-row gap falls sharply.
[Evidence](viewport/direct-1713/README.md). Physical1713 paired up-arrow tests pass48pixel/12preparation checks.
24-step medians194→179ticks light (7.7%),191→186dark (2.6%);
post-move complete-repair interval~46–51→2–4ms per step. No new bitmap.
Actual held-arrow video reviewed: completed rows/bar correct; transient mixed
CRT/copy scan remains ambiguous at30fps, no human flash-free acceptance.
1713 physical Ctrl+C/Cmd-period separately return130/recover, owned0, stable
326596free/323310largest at those checks.
User-authorised webcam capture now supplements screenshots (video only,
1280×720/30fps, private ignored work/ clips; no physical flash-free claim yet).

Separate trusted-local PowerBook listener fixed its initial peer rejection;
640×400 capture, mouse/keys and background Finder control pass. User is testing
networking; RC1702 Core and independently installable utilities are served on its
separate development HTTP endpoint. **User reports ping fails on the PowerBook and
Ctrl+C does not restore the prompt.** Not reproduced or diagnosed; Open Transport
is a hypothesis, not an established cause. Command-period suggested; outcome
unknown. Deferred at user's request; do not claim PowerBook networking acceptance.

## 0.5.0 integrated candidate — hardware acceptance pending

17 November: build **2026111702**, branch `codex/release-0.5.0-rc`.
[Candidate review and evidence](release-050-candidate/REPORT.md). Recovery baseline
74b5132 retains prior networking source. Normal build/offline kit now include
47 external commands, with native DNS/status/ping, pico HTTP, FTP and libtelnet
Telnet. No networking linked into Core or added to startup. Nano stays optional.
Focused FTP warning/diagnostic and unsupported ESC-consumption fixes pass host
sanitizers; truncated terminal-control EOF fails cleanly. Fresh graphical Snow
installation/relaunch, installed Core fork comparison, command checks/help,
pipelines, binary FTP round trips, chunked HTTP, remote Vim/less, reconnect and
local cancellation exercised. Core/awk also work without .IPP or Ethernet.
FTP heap379554/379552/379554, largest379298, tracked survivors0. Not hardware
performance measurements. Initial offline result extraction lost buffered writes;
repeat batch sent closed evidence over FTP and explicitly flushed destination.
User will perform physical-SE testing later; no agent hardware session planned.
Candidate1701 HDA/MacBinary hashes and47-image manifest rechecked17 November.
Final stock-SE acceptance and post-acceptance review remain release blockers. No release/tag/main merge. Existing updater28/slow handoff remain. Greeting now reads the native System-file version (6.0.8).
[Focused greeting/updater review](update-review/REPORT.md): two disposable Snow
1701→1702 HTTP updates automatically relaunched; both forks and recovery copies
verified offline, with type/creator/flags/creation date preserved. Error28 did not
reproduce and is not declared fixed. Updated offline kit includes47 utilities.
No physical acceptance claim.


## Prior upstream reuse evidence

17 November: [inventory](reuse-audit/REPORT.md) and [next batch](reuse-next/REPORT.md).
Find uses genuine musl matching (checkpoint d299d46). Mktemp now restores sbase
`-d` through exclusive HFS creation; a demonstrated cancellation leak while
publishing the created name is fixed with a mktemp-only ownership cleanup hook.
HTTP response/chunk parsing now uses pristine picohttpparser, preserving native
transport, staged writes and updater decoded-byte limits. Dynamic bounded header
storage is freed before body I/O; existing transfer buffer reused, no idle Core RAM.
Host sanitizers/upstream comparisons and accelerated Snow binary/error/cancel/
reconnect/repeated-run tests pass; final native failure-injection evidence is in
that report. CLX mktemp16396→18092 B; wget19328→23244 B. No physical speed claim.
Repeated HTTP heap settles to379486/largest379442; mktemp shows no progressive
loss. Normal wget/updater builds pass; no application update cycle performed.
Physical acceptance pending; SE and RC untouched. Remaining reuse candidates:
upstream od, HTTP redirects/other framing policy, FTP adapter prototype. Libvterm
screen replacement still requires review for its substantial memory increase.

16 November 2026, Adelaide. Development **2026111550** separates Core from the
standard utility collection. Host and fixed-maximum-speed Snow Plus/4 MB/
**reported System 6.0.7** validation, with bounded physical System6.0.8 acceptance below. No release,
tag or main change. Development checkpoint branch:
`codex/core-distribution-2026111550`; accepted `0686839` has been pushed.
Offline-installer UX follow-up is locally validated; ordinary Core remains 1550.

## SEControl / PowerBook 180 preparation

- Separate helper adds optional eight-key bursts with native queue backpressure,
  no automatic Return, and dynamic monochrome screenshots/mouse bounds.
  No Clarice production changes. Helper partition224→256KiB; no resident bitmap.
- 16 host tests pass; Snow Plus ordered burst/invalid-input/backpressure and
  cancellation/release checks pass. Mac II640×480 capture and off-SE-range mouse
  movement pass. Foreground Clarice burst/pwd, close/reopen and background capture
  also pass with stable post-capture heap24992 in the existing older-app fixture.
  Physical SE burst input now passes:67 characters19.545→4.062s (4.8×),
  exact output checksum verified. PowerBook/System7.1 remains pending.
- Private test kit: work/powerbook-180-kit/Clarice-PowerBook-Test.hda, containing
  accepted1550 Core, native installer,43 utilities and new helper/source/licences.
  Not bootable; private config included, not for public distribution. Existing
  physical helper retained; separate SEControl Fast now active. Snapshot heap
  returns to27068; capture7–15ticks versus old1–3 is a documented regression.
  Details: se-control-upgrade/REPORT.md.

## 0.5.0 physical acceptance — ready for release-candidate review

- Stock SE/68000/4MB; Finder About confirms System6.0.8/Finder6.1.8. Clarice's
  6.0.7 greeting is a display/reporting defect. Physical results are distinct
  from the Snow6.0.7 fixture. Details: [acceptance report](acceptance-050/REPORT.md).
- Remote Core1514→1550 update/relaunch passed; no error28 observed this run,
  intermittent fault not proved fixed. All original recovery apps retained.
- Native offline fresh installation cancelled and resumed successfully, then
  auto-launched Core1550. All43 freshly installed commands fully verified;
  fresh awk and mixed pipeline passed. Repeat installer preserved private-wc,
  modified wc/Finder creatorT050 and profile checksum/exported sentinel.
  Full-collection bytewise comparison was cancelled as redundant/slow, not passed.
- Original Data restored; profile/preferences and /bin checksums match pre-test
  records. Canonical running app confirmed Macintosh HD:System 6.0.8:Clarice.
  Disposable Data retained as Clarice Data Tested 050; original independent
  utility versions deliberately preserved. Core ordinary startup has no utility
  installation/scanning; no new matched physical startup benchmark collected.
- Two physical pairs: wc~71%, tr~39%, find/du~88% faster. Guest module ticks
  include loading/full verification of temporary images; exact breakdown in
  acceptance-050/physical-timings.json. No universal emulator/hardware ratio.
- Portable script failures0; binary/file/text/pipeline/error paths passed.
  Ctrl+C/Cmd-period returned130, zero owned survivors and subsequent recovery.
  Repeated wc/tr/find heap382008 stable, largest381826→381812. Genuine awk
  exhaustion2 then recovery0, heap381546/largest381378 stable, remaining0.
- Nano edit/save/exit/terminal restoration passed, ownedpeak77160/remaining0.
  Native APPL copy fork lengths/type/creator/creation date preserved; independent
  physical resource-byte comparison not performed. Installer verified staged
  Core forks by CRC/length; accepted Snow fork/failure tests remain separate.
- Immediate themes, selection, scrolling/history, typing-to-live, Up/Down,
  pending-line cancellation and foreground typeahead passed. Original prefs
  restored byte-for-byte. Unchanged renderer has no new human sub-frame test.
- No new functional/data-integrity blocker observed. Proceed to dedicated RC
  review, not publication. Known updater28/slow handoff, wrong OS greeting and
  unmet nano3s target remain explicit. PowerBook/System7.1 deferred. Physical
  disk-full/power-loss failure was not forced; no power-loss atomicity claim.

## 0.5.0 bounded utility performance audit — Snow validated

- Core remains 2026111550; external utilities only. Complete-command warm guest
  ticks: wc64KiB2983→763, tr4434→2687; paired100-file find2098→159,
  du2095→146. Native traversal reuses catalog metadata and explicit directory IDs;
  isolated upstream UTF ASCII fast paths avoid unnecessary general conversion.
- No permanent allocation. wc/tr peaks unchanged4208/3668 B, zero tracked
  survivors; repeated free/largest heap stable. Traversal saves ~504 B per depth
  frame. Images grow68 B(wc),124 B(tr),372 B(shared find/du/file); Core untouched.
- Host sanitizer/reference, fault/cleanup and QOL-1 tests pass. Final Snow output
  comparisons pass across volumes, relative/encoded names, forks and UTF/binary.
  Final find cancellation returned via Command-period; automated Ctrl+C arrived
  as plain c and follow-up counters were not captured. Hardware keyboard/cleanup
  acceptance is recorded above.
- [Audit, timing limits and evidence](performance-050/REPORT.md). Stop this bounded
  optimisation batch and proceed to consolidated hardware acceptance. sed regex,
  disk-spilling sort and command-substitution costs remain; no speculative rewrite.

## Offline first-install UX — completed in Snow

- Distribution now has a native **Install Clarice** launcher: open it, click
  Install, wait for installed Core to open. No appinstall knowledge or manual
  copying. Fixed destination is active System Folder:Clarice and Clarice Data.
  Launcher finds sibling media files by its own native path, not the disk label.
- Reuses absent-only setup, media cp, both-fork verification and existing native
  update transaction. Existing utilities stay untouched; older managed Core is
  retained as Clarice Previous. Same/newer Core is kept. Explicit repair remains
  separate. Installer skips user profile/preferences; launched Core uses them.
- Finder-to-prompt fresh install, Ctrl+C after six utilities, resume to all 43,
  bundled-1549 upgrade, automatic launch and repeat installation passed. Disk-full
  copy failure preserved old Core; freeing disposable fixture space allowed retry.
  Exported comparisons prove previous Core forks/metadata, independent utility
  (including resource fork), profile and binary user file preserved.
- Normal Core resources, including CODE, are **byte-identical** to accepted 1550:
  no added normal-startup work or resident allocation. Installed Core runtime
  reports 403202 B free / 401056 B largest block, zero tracked allocations.
  No new startup speed claim; reuse accepted distribution timings below.
- 482 existing transaction fault scenarios passed under host sanitizers. Focused
  guest acceptance is maximum-speed Snow Plus/4 MB/**reported 6.0.7**, not SE or
  6.0.8. Abrupt-reset leftovers require inspection; no power-loss guarantee.
  Prior Core copies outside the managed location remain untouched. A retained
  previous/staging file blocks another Core replacement until inspected/moved.
- [Installation instructions](../INSTALL.md), [focused evidence](install-ux/REPORT.md).
  Physical single-Finder/MultiFinder acceptance remains pending; no SE access.

## Core and utility distribution — implemented

- Core MacBinary **259.12 KiB** (was 1021.12 KiB), zero embedded CLX resources.
  All 43 standard command files are on separate offline native HFS media. Nano
  remains optional. Existing namespace, loader, resolver and updater retained.
- Ordinary startup no longer checks/installs utilities. Matched diagnostic
  repeated ready time **302 ->193–194 guest ticks**; removed catalog-check phase
  measured 107 ticks. Adding payloads back gives 193 repeated ticks: package-size
  reduction is not the source of startup speed. No new permanent allocation;
  payload-only resource-map saving ~330 B. Partition request remains 768 KiB.
- `appinstall setup DIRECTORY` and `install FILE NAME` install absent files only.
  Existing damaged/newer/custom commands remain untouched. `repair NAME DIRECTORY`
  uses external media; check/replace/recover keep existing validation/backup rules.
  Missing commands give an installation hint. [Offline instructions](../INSTALL.md).
- Final normal-build fresh installation from the native Clarice HDA installs all
  43 commands, byte-for-byte verified; **first-run awk OOM resolved**. Awk and
  ordinary pipelines execute. Explicit full setup takes 8565 guest ticks;
  startup never repeats it. `ls /bin` 1.91 guest seconds, loader 59 ticks.
- 77 host loader/installer scenarios, 31 shell comparisons, three cancellation
  cases, 32 builtin-help topics. Snow damaged preservation/repair/recovery,
  incomplete-stage refusal/retry, Ctrl+C130, real 290000-byte heap pressure and
  successful recovery. Native recovery preserves both forks, Finder metadata
  and dates byte-for-byte; profile unchanged. Repeated setup heap stable within
  2 B, largest block fixed, tracked command remainder zero.
- [Evidence, timings and limitations](distribution/REPORT.md). Collection setup
  is resumable, not atomic; leftovers need inspection. No physical/6.0.8 or new
  HTTP-updater acceptance. Native HDA reuses the existing Apple driver/header;
  no System/ROM/user data included. Generated packages stay untracked.

## Second utility batch — build 2026111549

- Genuine pinned sbase `dd`, `sleep`, `printenv`, `mktemp` are independent CLX
  commands. `env` and `time` are small shell wrappers: existing scoped execution
  avoids a nested-CLX process API. No new process architecture or resident cache.
- Runtime 1.5 adds native data-fork resize for dd only. Checked sizes, allocation
  before destructive opens, output-block seek units, explicit zero-filled HFS
  extensions and correct 32-bit record-count formatting. Ordinary writes remain
  partial on error; this does not replace the fork-preserving editor save path.
- Export snapshots are elastic. Explicit exported HOME/PWD now override their
  synthetic defaults; env -i/-u can suppress those defaults within a saved scope.
  Parent environment/functions/arguments/directory restore on return or failure.
- Native exclusive-create mktemp refuses unsafe name-only mode; -d is not added.
  Sleep cooperatively waits in Macintosh ticks. Time reports elapsed wall time
  on stderr; user/sys accounting and -p are explicitly unsupported.
- Snow: 37 utility cases, eight exported files compared byte-for-byte, native
  allocation/locked-file failure preservation, 2,049-byte exported value,
  pipelines, repeated runs and Ctrl+C status130. Repeat free heap settles at
  290,482 bytes; largest block alternates 289,034/289,164; tracked remaining=0.
  Workload includes the executing script, so these are not idle-heap figures.
- Host: eight dd reference cases, output/size/allocation failure, temp collisions
  and cleanup; 64 large environment entries; nine dash scope comparisons plus
  149 recovery trials (41 injected failures); 39 focused and 54 POSIX cases,
  1,268 parser recovery trials, 65 installation/loader scenarios and editor
  environment failure/cancellation/repeat coverage all pass.
- Help: all 43 bundled command names +32 builtin topics +nano exercised in the
  guest. Status/stdout/stderr and 72-column checks pass (upstream nano:80).
  Source/content review and regression contracts describe actual options and
  Mac limitations. Exhaustive option combinations are not claimed.
- Evidence, exact scripts, content inventory, binary checksums and build manifest:
  [utility-expansion/RESULTS.md](utility-expansion/RESULTS.md),
  [help-inventory-final.json](utility-expansion/help-inventory-final.json).
  Consolidated System 6.0.8 physical acceptance remains pending.

## Virtual streams and memory query — build 2026111548

- `/dev/null` and `/dev/zero` use stateless tokens through existing standard and
  portable file I/O; no HFS backing files or permanent allocations. Normalised
  paths and relative `/dev` paths resolve through the existing namespace.
  Read/write modes are enforced; seeks fail. Native catalog tools do not list
  these as physical files; old direct-HFS tools use them through redirection.
- `command -v [--] name` queries existing CLX/script PATH resolution, builtins,
  functions and implemented reserved words. One name; execution/-p/-V not added.
- Independent `free` (2.6 KiB class) reports installed RAM, application heap
  extent, free/largest blocks and purgeable capacity, not fictional Linux or
  machine-wide free-memory figures. Runtime 1.4 adds only `cli_memory`; older
  command compatibility remains. No purge or permanent allocation.
- Fixed pinned Multiversal PurgeSpace A062 glue: A162 must return A0. The original
  guest result incorrectly showed an address as contiguous capacity. Corrected
  Snow sample: free 398310 B, largest 398248 B, after-purge largest 398256 B.
- Host stream boundaries/access modes, shell restoration, six dash command-v
  comparisons, free formatting, 39 shell cases, 11 stream and 65 installer cases
  pass. Guest 23 cases, corrected memory query, six repeat pairs and actual
  infinite-zero cancellation/recovery pass (130). Repeat free heap settles at
  399858/399860 B; largest block stays 399630 B; tracked remaining always zero.
- Physical SE remains off. This is fixed accelerated Snow, reported 6.0.7.
  The second batch is completed in 1549 above; exhaustive option combinations
  and consolidated physical acceptance are not claimed.

## Utility expansion — first checkpoint, build 2026111547

- All 38 bundled command names, 29 builtin help topics and experimental nano
  actually executed their help in Snow: success, stdout, no stderr. Clarice help
  fits 72 columns; upstream nano retains its 80-column help. This verifies basic
  coverage/format, not exhaustive option conformance for every command.
- Corrected stale read/export/unset/printf/sh/test/preferences descriptions;
  expanded appinstall and modified upstream help. Native usage errors now use
  stderr; awk rejects unknown options instead of warning and continuing.
- Genuine pinned sbase cmp/cksum, independently bundled. Binary data forks only;
  POSIX checksum is corruption detection, not authentication. Host 1 MiB reference
  comparisons, ASan/UBSan and 16 runtime failure/cancel/cleanup trials pass.
- Snow: 16 cases include binary differences, stdin, pipeline CRC, empty/missing
  files and invalid options. Twelve repeated cmp/cksum pairs finish with zero
  tracked allocations. Free heap 376836 -> 376804 B; largest block 295896 ->
  295914 B. The 32-byte difference is recorded, not asserted to be a leak or
  proof of perfectly stable native heap. No permanent allocation introduced.
- The following 1548 batch adds virtual null/zero, command -v and free.
  Comprehensive help review and the second utility batch remain.
  New-command cancellation/failure tests are host-based; hardware acceptance is
  deferred. Editor, loader, safe-save and existing utility work preserved.

[Evidence, inventory and reproduction](utility-expansion/RESULTS.md).

Validated checkpoint **33f0075fb14385cd82d792f611261313b60e1583** on
`codex/se-alignment-2026111521` is preserved. Renderer and UX work through 1533 is checkpointed on
`codex/se-ux-2026111533`, inheriting Research Before Reinvention from 12ccb4a.
No tag or release. Physical loader profiling has resumed; build 1534 changes only
the approved installed-fast metadata validation path, not nano.

## Focused utility close-out — build 2026111546

- Startup profiles preserve their result, including Ctrl+C=130. Source execution
  also propagates exit/return/cancellation from loop conditions before restoring
  outer state. Host success/error/return/OOM, 80 cancellation points and recovery;
  Snow startup and ordinary foreground cancellation both return 130.
- `find` repeated predicates now combine with AND; added small `-path` filter and
  multiple roots using borrowed arguments. No expression tree, persistent cache
  or new working allocation. Host depth/failure/cancel tests and real Snow
  quoted-path/pipeline/cleanup workflow pass against dash/macOS reference output.
- `ls`/`df` unchecked 32-pointer arrays were a real stack-overwrite risk after
  shell argument growth. Replaced with allocation-free argv rescanning. ASan
  reproduced the old overflow; 130 host and 100 Snow operands each pass.
- Three final find runs: free heap **396578 / 396576 / 396578 B**, largest block
  **393318 B**, tracked remaining **0**. No persistent allocation added. Catalog
  order and final-component-only shell globbing remain documented limitations.
- Host 31 basic/3 cancel, 54/39/41 comparisons, 12 scripts, 3090 parser/adapter
  recovery trials and six upstream-script variants pass. Editor environment
  failure/cleanup regression remains passing. No new physical acceptance.
- Reviewed active bounds: sort's 16 merge levels/20 temporary slots, formatter
  510-byte format strings, and canonical terminal input 255 bytes remain explicit.
  Nano uses raw input; these are not new line-record or exported-value limits.
  No demonstrated ordinary-workflow failure justified changing them this batch.

[Evidence and reproduction](utility-closeout/RESULTS.md),
[compatibility](utility-correctness/COMPATIBILITY.md).

## Native heap and editor environment — build 2026111545

- Root cause: upstream awk overwrites argv entries. Passing the shell-owned vector
  lost its program-string allocation and caused a duplicate free attempt. CLX now
  lends a temporary pointer-vector copy, released on normal/error/cancel return.
  No checksum/ABI or allocator redesign; no permanent memory added. Two-argument
  commands require 12 transient bytes plus native block overhead.
- Before: native heap walk gained one pointer block per run, handles stayed flat;
  compaction did not recover it. Fixed matched new awk settles at 362452 B free;
  ten empty runs remain 362352 B. Four final walks retain 769 pointer / 36 handle
  blocks. Native stdio's previously measured reusable pool remains unchanged.
- Editor exports now grow on demand, including values >1024 B; TERM=clarice and
  private-variable exclusion preserved. Host 80 x 4096 B, 88 import-failure sites,
  cancellation and repeats pass. Snow 40 x 2048 B: five successes, free heap
  309742/309744 B, largest 306934 B, owned peak 97220 B, remaining=0. Three injected
  import failures return 2 and recover with stable heap and explicit diagnostics.
- Existing nano/ncurses objects relinked with only the terminal adapter recompiled.
  Six launches, disposable save/reopen. Matched three-run loop: free heap
  399650/399658/399658 B; largest 236688 B; owned peak 87684 B, remaining=0.
  30 exports and a 1500-byte value present. Canonical 255-byte input bound unchanged.
- Host 54/39/41 shell comparisons, 12 scripts, 31 basic/3 cancel, 9874 parser and
  3090 scope fault trials pass. Loader mutation/error/cancel/admission tests pass.
  Six upstream-script guest variants still match dash exactly. Native awk heap
  exhaustion fails explicitly and recovers; 1000 entries exceed this fixture's
  available heap, so that workload is not labelled successful. Ordinary Ctrl+C
  returns 130, skips the next command and restores streams; a following 500-entry
  awk array returns 0 with remaining=0.
- Normal MacBinary 957952 B (+256); experimental nano 268212 B (+212), separately
  installed. Local build artifacts excluded from Git; no physical-SE acceptance.

[Evidence, limits and reproduction](heap-audit/RESULTS.md),
[compatibility](utility-correctness/COMPATIBILITY.md).

## Practical scripts and environment audit — build 2026111544

- Syntax cleanup no longer overwrites existing errors with OOM. Seven malformed
  quote/parameter/compound cases recover with status 2; 128 injected failures on
  those same inputs still report genuine OOM, free allocations and parse again.
- Three complete unchanged upstream scripts (musl specs, ncurses 6.5 definition
  and parameter generators), six default/space-path variants, match dash exactly
  on host and native guest. Full upstream Caps produces 414 entries. The default
  ncurses path exposed `set - Caps`; ordinary positional set and set-minus forms
  now work. No Unix build-system, trap/PID or echo-extension compatibility implied.
- awk previously dropped exports beyond 18 entries or its 164-byte buffer. Rebuilt
  standalone awk now imports on demand through existing invocation allocation.
  Host: 30 x 4096-byte exports, low-memory failures, cancellation before execution,
  remaining=0. Snow: 30 x 512-byte exports, three successful repeats, owned peak
  67723 B, remaining=0. No idle reservation or runtime ABI change.
- The subsequently isolated argv-ownership leak and editor environment limits
  are corrected in 1545 (above); original 1544 evidence remains historical.
- Retained host 54 POSIX, 39 focused, 41 elastic and basic/cancellation cases pass;
  9874 parser and 3090 scope failure positions pass. Native Ctrl+C returns 130,
  skips the following command and restores stdout/stderr. Guest runs fixed max
  FFWD (~15–16×); synthetic entry briefly normal speed. Normal MacBinary 957696 B
  (+128 B), separate awk.clx 188460 B. Existing utility/editor work is preserved.

[Practical results/provenance](shell-practical/RESULTS.md),
[guest evidence](shell-practical/guest.json),
[memory comparison](shell-practical/memory-comparison.json),
[compatibility matrix](utility-correctness/COMPATIBILITY.md).

## Parser safety and elastic capacity — 16 November

- Fixed the reproduced parser-constructor NULL dereference and unchecked AST,
  token-buffer, heredoc, arithmetic and partial-copy failures. Explicit ownership
  cleanup preserves existing functions and restores shell streams/state. Uses
  mrsh's existing AST/destructors; no parser rewrite or region allocator.
- Removed source-size, total-token, function-count/name and source-chunk ceilings.
  Storage follows available heap. Explicit source/execution (8/16), parser (32)
  and deep expression-chain (64) stack guards remain. Interactive input is still
  1023 bytes; completion safely declines names outside its existing fixed buffer.
- Host: 9,874 parser, 8,268 AST-copy, 660 arithmetic and 1,268 whole-shell injected
  allocation positions; single failure and continued exhaustion, zero retained
  tracked bytes, subsequent command recovery. Existing 39 focused, 12 whole and
  41 elastic cases pass, as do 31 comparisons / 3 cancellation cases and 329
  adapter allocation failures. Long-name completion passes sanitizer checks.
- Snow diagnostic run: all 39/12/41 cases, eight capacity/stack-guard cases and
  282 actual injected failures pass; zero retained parser bytes after each trial,
  pre-existing function remains callable. Diagnostics are excluded normally.
- Normal Snow build: 32 KiB source, 600 simple commands, 48 long-named functions,
  200 arguments; three actual heap-exhaustion attempts return 2 with an error,
  execute no partial program and recover. Free heap 411350 → 411360 → 411360 B.
  Actual Ctrl+C returns 130, skips the next command and restores both output
  streams; portable remaining=0. Fixed uncapped FFWD about 15–16×.
- Ordinary host adapter `echo hello` remains six allocations / 259-byte peak /
  zero retained bytes. No permanent cache or reservation. Parser checking adds
  host CPU cost (recorded, not a physical-performance claim). Very large source
  parsing still shifts its read buffer repeatedly and does not poll inside lexical
  scanning; large-script throughput remains a limitation.
- Earlier elastic variables/arguments/read/substitution work and utility/editor
  changes are preserved. The following batch extends quoted `$@`, export visibility,
  printf and test. No claim of full POSIX conformance.

Evidence and reproduction: [parser results](parser-safety/RESULTS.md),
[diagnostic guest](parser-safety/guest-diagnostic.json),
[normal guest](parser-safety/guest-normal.json),
[compatibility matrix](utility-correctness/COMPATIBILITY.md).

## POSIX expansion, scope and formatting — build 2026111543

- Quoted `$@` preserves vector boundaries, empty arguments and prefixes/suffixes;
  quote-group handling resolves the empty `"$@$@"` dash discrepancy. Braced special
  parameters now parse. Default/alternate expansion retains nested quoted fields.
- Export flags distinguish local, exported and exported-but-unset variables;
  preserve flags on reassignment, clone and restoration. Child sh drops locals and
  functions; functions/sourced scripts share variable changes, subshells isolate
  them. No-argument dot correctly shares changes made by `set --`. Native external
  commands receive only exported entries, plus existing HOME/PWD defaults.
- printf streams integer/string/character/escape conversions with literal width,
  precision, format reuse and flags. No width-sized allocation; 64-byte temporary
  output blocks and Ctrl+C polling. `test`/`[` gain grouping/negation and checked
  integer operands, retaining arity-sensitive operand interpretation.
- Host: all 54 new dash comparisons, 39 focused, 12 complete, 41 elastic cases,
  31 existing comparisons/3 cancellation cases pass. 3,090 new whole-shell fault
  trials restore streams, preserve the seeded function/variable and leave zero
  tracked bytes; parser/AST sweeps and 329 adapter failure trials still pass.
- Normal Snow build at fixed uncapped FFWD (~15–16×): all 54 new and 39/12/41
  retained cases pass, plus native awk export visibility. Actual Ctrl+C interrupts
  billion-column printf with 130, skips BAD and restores stdout/stderr. Three
  repeats: heap 314612 / 314612 / 314608 B; largest block 225530 B throughout;
  portable remaining=0. These are script-context measurements after running awk,
  not directly comparable to the previous idle parser-only fixture.
- Small host `echo hello`: six allocations, 267-byte peak (previous 259), zero
  retained; quote position adds transient word state and export flags add per-variable
  state. No permanent cache or reservation. Normal MacBinary 957568 B (+4224).
- Remaining: full POSIX certification is not claimed. Floating printf and dynamic
  width, legacy test -a/-o, Unix permission predicates, complex vector parameter
  operations, shell options and the documented native path/terminal limits remain.
  Large-source lexical cancellation/throughput limitations are unchanged.

[Results and reproduction](shell-posix2/RESULTS.md),
[guest evidence](shell-posix2/guest.json),
[compatibility matrix](utility-correctness/COMPATIBILITY.md).

## Portable scripts and compound redirection — 16 November

- Fixed the imported parser's missing compound redirect-list support: redirection
  now covers the entire group/loop/subshell/if/case, through the existing shell I/O
  owner. Restores previous streams after completion/error/control flow/cancellation.
  Redirect lists are owned/copied/freed with their AST. Failed copies reject and
  free partial trees and retain existing function definitions.
- Host: 12 complete scripts match dash, plus existing 31 comparisons/3 cancellation
  checks and 39 focused cases. Eleven scope/failure/cancellation checks pass with
  no open native references; 18 allocation-position trials exercise the added
  parser/copy sites (10 reached injections, 8 beyond-allocation controls).
- Snow Plus/4 MB, reported System 6.0.7: all 12 script outputs match, using actual
  CLX utilities; od values compared after portable whitespace normalisation.
  Fresh native metadata/locked-file/listing/df checks and all 34 external --help
  exits pass. Repeated scope/here-document/read batches: heap **399,996 B** and
  largest block **375,926 B**, identical across three runs; portable remaining=0.
- Real guest Ctrl+C interrupts a redirected infinite loop with **130**, skips its
  following command, restores stdout/stderr and permits subsequent file writing.
  Functional runs at fixed uncapped FFWD (~15.6–16×); briefly normal speed for
  synthetic keyboard setup only. No physical results inferred.
- Memory cost: one 4-byte optional pointer per parsed command, redirect storage
  only when used, one 4-byte copy-failure generation counter; no new permanent heap
  allocation. Scoped execution reuses the existing simple-command frame, including
  its temporary ~4.14 KiB argument structure; not a new stream implementation.
- Shared newlib 4.6.0 retains **2,180 B** after the 32-stream probe and reuses all
  32 FILE addresses on two repeats. This is useful stdio capacity, not automatically
  reclaimable general heap. No libc change justified or made.
- Complete declared-utility acceptance inventory and supported/partial/unsupported/
  platform-limited shell matrix: [compatibility](utility-correctness/COMPATIBILITY.md).
  Capacity limits, function-definition redirects, full export/subshell semantics,
  complete printf/read remain gaps; parser OOM hardening is completed above. No POSIX claim.

[Evidence](utility-correctness/RESULTS.md), [libc finding](utility-correctness/LIBC-POOL.md).
Local normal app: `outputs/utility-correctness/build-1540/Clarice.bin` (diagnostics
and nano profiling OFF). Bundled resources remain separate from independently
rebuilt/tested utility candidates; startup does not replace existing installations.
The pushed checkpoint excludes generated packages, raw control sessions, private
preferences, legacy build-log noise and disposable large-output fixtures. All
existing branches/tags/releases preserved; subsequent task changes remain local.

## Utility elasticity and small-command audit — 16 November

Experimental normal build **2026111538** and independently built awk/sort are
host/Snow validated; no physical deployment or new cache. Full inventory and
reproduction evidence: [utility audit](utility-audit/AUDIT.md).

- Actual installed ls: 18 Snow invocations (help, empty and ten-entry directory).
  Warm loader median **40 ticks / 0.67 s**, of which reading is 13–14 ticks,
  receipt work 16, preparation 8, imports/relocations about 1–2 each. First load
  53 ticks. Reading does not dominate; retain on-demand CLX, no caching/builtin
  conversion. These module timings exclude shell parsing and final prompt paint.
- Removed Clarice's artificial 16 KiB awk/sort record ceilings using upstream
  growth and existing spill merging. Small sort batches remain bounded; large
  records transfer buffer ownership instead of allocating a duplicate.
- Shell read now consumes an oversized record, reports failure and preserves its
  variable instead of silently succeeding with a truncated value. Exact 127-byte
  records and subsequent reads work; no new permanent allocation.
- Host: 37 elastic-text cases, 30 awk comparisons, 45 tail/sort comparisons,
  spill/error/cancellation tests, 31 shell comparisons and focused read boundaries.
  Snow: exact 64 KiB records, multi-run sort/unique, genuine heap exhaustion,
  recovery, Ctrl+C status 130 and zero command-owned survivors. Small sort peak
  4,492 bytes; 64 KiB sort 208,408; awk length 152,101. Large fields still exhaust
  the available application heap explicitly. No claim that every command is elastic.
- Rebuilt grep/sed: accelerated Snow now verifies exact 64 KiB grep and single
  substitution output, 32 KiB global substitution, and subsequent small-command
  recovery. Sed no longer scans beyond the requested positive occurrence or
  repeatedly scans the growing substitution destination. Host: 35 comparisons
  against pristine upstream, allocation failure and cancellation cleanup pass.
  Global 64 KiB substitution still exhausts this guest's available command heap:
  explicit status 1, no output and zero portable-owned survivors; not silent success.
- find/du now use demand-allocated iterative traversal instead of depth 8.
  Host depth-24, allocation-failure-at-each-frame and cancellation tests pass;
  Snow verifies 26 entries, explicit maxdepth 12, and expected allocated-space
  total. Native path/catalog limits remain. cp/rm now also use iterative traversal;
  depth-24 guest copy/removal, allocation failure, cancellation and recovery pass.
- Test at fixed uncapped FFWD, dynamic acceleration disabled. Startup sometimes
  stays around 1.5× until FFWD is toggled off/on; verified runs show 15.1–15.4×.
  Use prepared guest startup scripts: current automated key/clipboard delivery
  is unreliable at this speed. This is not a demonstrated Clarice paste defect.
- Disposable-volume exports are reliable after explicit native flush/unmount.
  Earlier host-emulator termination without guest unmount lost catalog changes
  despite a name-based FlushVol success. Cause is not established; preserved
  evidence must not be treated as successful byte verification. No production
  filesystem changes made to compensate for the test shutdown problem.
- Guest reports **System 6.0.7**, Plus/4 MB; physical SE/6.0.8 acceptance pending.
  Focused results: [utility correctness evidence](utility-correctness/RESULTS.md).
  Native-error descriptions and inverse pinwheel are now guest-validated below.
  Stream/descriptor elasticity and cp/rm traversal are validated. The broader
  utility/POSIX audit remains incremental, not a full compliance claim.

## cp/rm and portable streams — 16 November

- Removed depth-8 cp/rm restriction with explicit demand-allocated DFS frames.
  Both-fork replacement remains unchanged. Partial directory trees after failure
  are reported, not rolled back as a whole; source files remain intact.
- Removed 12-entry portable stream/descriptor ceilings. Stable stream cookies grow
  individually; descriptors grow on demand and reuse closed numbers. Standard
  streams remain small/static. Command-owned records/buffers are all reclaimed.
- Host ASan/UBSan: depth 24, 25 copy allocation boundaries, removal allocation
  failure, cancellation and cleanup; 40 simultaneous streams plus 40 descriptors,
  fdopen ownership, repeated invocations, low-memory and cancellation checks with
  native handle counts restored. Existing 30 awk / 19 sed comparisons pass.
- Snow fixed uncapped FFWD (~14–15.5×): deep cp/rm and fault injection pass;
  failed/cancelled overwrite preserves both forks and Finder metadata; no leftover
  copy temporary siblings. 16 streams and 16 descriptors separately pass three
  times, as do injected runtime OOM/cancellation and subsequent recovery.
  Real upstream awk writes 16 simultaneously open files, repeated; all bytes exact.
- Native combined capacity test reaches EMFILE after 16 streams plus 11 descriptors
  in this guest: genuine system-wide open-file capacity remains. All portable
  survivors zero. Stream test peak 13,816 B; repeated free heap 397,948 B unchanged.
  Shared newlib FILE records retain roughly 2.10 KiB after first larger use; largest
  free block can shrink from allocation placement (first stress: 388,476→364,500 B).
  This is libc high-water storage, not a new Clarice resident cache; no repeated loss.
- Normal external packages prepared locally: cp/rm 12,840 B each (+708), awk
  188,312 B (+532). Other installed/bundled commands are not silently updated;
  they acquire the new runtime when rebuilt. No application/SE update, push or release.
  [Evidence and commands](utility-correctness/RESULTS.md); guest remains System
  6.0.7/Plus/4 MB. Physical SE and 6.0.8 acceptance remain pending.

## Native errors, activity display and POSIX checks — 16 November

Normal local application **2026111539** passes Snow Plus/4 MB/System 6.0.7
at fixed uncapped FFWD (~15.6×). SE remains untouched; no push or publication.

- Native error reporting retains numeric codes and adds shared descriptions;
  missing-file, disk-full, memory and unknown-code fixtures pass, as does actual
  cat failure. Unknown errors are not guessed; upstream Unix errno remains separate.
- Pinwheel CopyBits now uses neutral port colours for exact pixel transfer.
  Native QuickDraw tests pass all four frames, both themes and both destination
  polarities, with exact restoration after cancellation. Free heap stays 422,922 B;
  no additional bitmap or permanent allocation. Clock/catch-up tests pass.
- Shell fixes: IFS non-whitespace empty fields, quoted `$*` joining, substitution
  status for assignment-only commands, sh -c positional arguments, read -r/escape
  handling and IFS trimming. Temporary assignments preserve unrelated builtin
  effects. Two small mrsh parser fixes preserve empty assignment boundaries and
  prevent `$#` following another expansion being swallowed as a comment.
- Host ASan/UBSan: existing 31 shell comparisons + 3 cancellations, read overflow
  boundaries, and 39 new dash comparisons pass. The same 39 cases pass byte-for-byte
  in eight Snow batches. Existing pipeline, spilling-sort, cancellation and
  output-failure checks pass against the current runtime.
- Shell remains bounded (127-byte variables, 32 arguments, 8 KiB source, 256-token
  guard); read supports one variable. Embedded quoted `$@`, broader parameter
  grammar and other documented shell gaps remain. No general POSIX compliance claim.
  These changes add two scalar substitution counters (8 B in the 68k build),
  temporary read quote markers and no resident cache. Full shell memory/latency
  profiling and physical/System 6.0.8 acceptance are not claimed.

[Tests, references and evidence](utility-correctness/RESULTS.md).
Normal MacBinary: `outputs/utility-correctness/build/Clarice.bin`, with SHA256SUMS;
activity diagnostics and nano profiling are OFF. Independent utility candidates
remain separately packaged; the normal application does not silently replace them.
Next: continue capability/failure coverage and remaining artificial shell limits,
then consolidated physical acceptance when authorised.

## Physical SE screensaver — 16 November

The user completed After Dark/Flying Toasters installation and shut down the SE.
User reports Flying Toasters activates after **10 minutes of inactivity**. Before
future SEControl screenshots or keyboard input, send a mouse movement to wake the
display, allow it to restore, then capture/check the foreground application before
typing. Screensaver activation was not independently verified remotely.
Clarice wget successfully transferred the control panel and module beforehand;
no Clarice production changes were needed. Leave hardware off until available.

## Nano post-loader experiment — 16 November

Keep the existing strip renderer; omit trailing-space drawing after full-row
clearing and render directly at the destination bit phase where the same buffer
fits. QuickDraw clipping, inverse spans and cursor behaviour remain. No additional
bitmap or permanent allocation. The shared newlib format adapter now covers nano's
vsnprintf/sprintf paths: `Read 128 lines`, not `Read zu lines`.

Matched physical warm startup, measured internally to painted input readiness
(excludes screenshot overhead): empty **5.68 → 5.10 s**, 8 KiB/128 lines
**9.19 → 8.57 s**, 8 KiB/two long lines **7.34 → 6.88 s**. Two runs per case;
portable remaining=0 throughout, peaks unchanged. Drawing totals respectively
62–65 → 34–35, 103–105 → 69–71, 69–71 → 39–42 ticks. Physical editor-body,
shortcut-bar and scrollbar pixels match exactly (corrected status line excluded).
Normal nano image grows 196 bytes; no new idle heap reservation. Instrumented app
free heap is stable at 418,906 bytes versus prior 419,010, not accumulating per run.

Host dirty-buffer/clip/theme/cursor checks and safe-save fault injection pass.
Snow normal nano repeated saves preserve exact data, resource fork, type/creator,
flags and creation date; cancellation returns 130 without saving, remaining=0.
Normal 1537 opens/exits nano successfully. Physical normal nano installed with prior command retained; new-file editing,
save, readback and exit passed with remaining=0. Clarice was then closed for the
night; no additional hardware work is running.
No high-speed physical flicker capture or fresh matched cold run this evening.

Remaining priorities: short-file open/construction ~2.8 s (native reads only
6–10 ticks), final ncurses refresh ~1.65 s including output/rendering, warm CLX
~1.42 s. Counters overlap; do not sum. Cold preparation/receipt policy unchanged.
Three-second target remains unmet. Hardware work is stopped for tonight; no further renderer campaign. [Evidence and methods](nano-profile2/README.md).

## Executable loading — accepted focused experiment, 16 November

Accepted checkpoint `2e84229` was pushed to `codex/clx-fast-2026111534` on
16 November, preserving AGENTS `43eb445` and checkpoint `7eb7064`. Main and
releases were not changed. Build **2026111534**
retains receipt-gated installed-fast loading but omits its repeated metadata hash.
One cancellation poll and all structural/import/relocation bounds remain. Full
installation/repair/check/strict and unknown/changed-file verification remain.
No executable-format change, receipt removal, permanent allocation or nano change.

Physical stock SE loader: cold **356 → 215 ticks (5.93 → 3.58 s)**; warm **190 →
85/86 (3.17 → 1.42/1.43 s)**. Fresh follow-up cold 222, warm 87 ticks. The removed
metadata/event interval accounts for the improvement; preparation remains 132–139
cold versus 3–4 warm. Receipt work remains about 15–16 ticks (~0.25 s).
Host 64 scenarios pass, including structural corruption, strict/check rejection,
revocation, replacement/recovery and cancellation. Snow disposable unchanged-identity
metadata corruption passes only in fast mode, then fails strict/check and succeeds
after explicit replacement. This intentional residual corruption risk is documented.
No retained invocation allocations in repeated physical nano and native-probe runs.
[Acceptance and evidence](loader-fast2/ACCEPTANCE.md).

Whole-nano physical observations include first rendering, not just loading:
complete-screen capture upper bounds **12.26 s cold empty, 9.35 s warm empty,
19.88 s warm 8 KiB/126 lines**. Input was confirmed after complete rendering
and test edits discarded. Wi-Fi/capture overhead is included; these are not exact
internal startup marks. Cold 8 KiB remains unmeasured here. `wc` subsequently confirmed the historical
fixture has 126 lines; its previous two-long-lines label was erroneous. The three-second editor target remains unmet. Earlier partial
frames must not be counted as complete startup. Both SE and Snow greetings report
System 6.0.7; do not claim independent 6.0.8 acceptance from these captures.

Minimal returning native APPL comparison completed with the same bounded argument,
stdout/stderr/status body. Snow CLX **25/27/27 ticks**, native through driver
**126/127/126**. Physical CLX **34/34/33**, native **157/155/155**. Native includes
file exchange, separate 160 KiB partition, actual launch, execution and return;
module timing excludes final prompt painting. Both routes return status 7 and
recover memory. This small-command result does not establish a large-app or nano
advantage. Keep CLX; no production process replacement is justified by this probe.
[Methods and limits](native-bench/RESULTS.md).

Receipts are useful only for the current checked-versus-unknown launch policy;
staging and retained backups provide recovery. Recommend policy review before
removing receipts, not another trust database. Cold NewPtr/image/metadata/stdio
preparation is the next loader measurement; do not hide it with permanent caches.
[Native-first and receipt assessment](loader-review/SIMPLIFICATION.md).

Runtime allocation already adapts to available application heap minus 32 KiB
headroom; there is no current 96 KiB command ceiling. Small 2 KiB pages and direct
allocations are invocation-owned. Genuine limits include the 768 KiB partition,
contiguous space and 64 KiB stack. Separate loader limits (256 KiB image, 8192
relocations, 128 imports) need compatibility/overflow review before changing.
Initial utility limits: FILE/FD tables 12, temporaries 20, recursive traversal depth
8; inventory is preliminary, with no widespread utility rewrites authorised.

Deployment issue: Finder was scrolled past the current app and contained many prior
`.cu-*` copies. Remote inspection located build 1534 as **Clarice Updatastute** on
Macintosh HD and confirmed its greeting before measurements. Older copies were
opened during identification; those are not benchmark evidence. No application
contents or backups were deleted. Owner requests one clearly named current app,
identifiable recovery and safe staging cleanup. Updating an existing copy is already
the intended workflow; broader cleanup remains future updater work. No push/release.

## Terminal UX and capabilities — 15 November 2026

- Configurable physical Control/Option/Command, CLI and Preferences popup;
  physical Control always works. Macintosh Command shortcuts retain precedence;
  Command+Option sends conflicting controls in Command mode. Native KCHR mapping
  is centralised; editor control bytes no longer collide with arrow encoding,
  including input queued during loading. Queue overflow rejects without partial
  delivery; actual native queued Ctrl-underscore opens nano's Go to Line.
- Greeting reports native hardware/build details. Allocation-free runtime 1.3
  `cli_hardware` provides a reusable versioned snapshot; older commands retain
  compatibility. Plus/SE API reports System 6.0.7 despite installed 6.0.8, labelled
  as reported. Later CPU/FPU/colour-machine qualification remains outstanding.
- Full historical viewport with no pinned prompt; typing/paste returns live and
  retains the trigger. Themes repaint immediately, preserving input/history.
  Preferences and Command mode/dark theme survive Snow application restart.
- Visible update `[Y/n]`, Enter accepts, n cancels, `-y` skips only confirmation;
  warning explains only Clarice closes/relaunches. Existing verification/recovery
  unchanged. Shared width-aware progress has current/total IEC sizes, percentage,
  speed, unknown totals, narrow-window fallbacks and one-second throttling.
  Explicit 32-bit arithmetic tests cover counters through 4 GiB; no huge-file
  transfer claim. Native Plus and SE counter probes pass.
- Host sanitised input/preferences/progress/HTTP checks and 61 loader/installer
  scenarios pass. Snow UX probe zero failures; 322 clipping/pixel comparisons pass;
  both themes and normal post-nano scrolling: zero text/bar mismatches. Option-C
  returns 130; Command-X exits genuine nano. Kermit binary round trips exact;
  cancellation/recovery work, owned remainder zero. Normal 1529 → 1531 `-y`
  update automatically relaunched; prior app and Finder metadata verified.
  Diagnostic → normal update reproduced historical error 28; cause still unknown.
  Physical 1525 → 1531 with Enter and 1531 → 1533 with `-y` relaunched normally.
- Plus capability query ~22 ms, no retained heap; normal launch-to-ready counter
  318 ticks (5.3 s; excludes time before main/first completed paint). Resource fork
  +2,291 bytes over 1525 in final 1533; no new bitmap, partition or stack increase.
  Snow dense history 336.8 ms versus historical 327.1, now drawing an extra historical row;
  not a matched same-workload renderer comparison. Physical wget 64 KiB old 347,
  new 352/322 ticks: no demonstrated throughput regression from progress display.
- Physical controls, Command-C/V, both immediate themes, historical viewport and
  retained triggering keystroke pass. Output interruption returns 130; both themes
  pass 22 bursts + 20 views with zero text mismatches and 147/149 clean scrollbar
  checks. Capability query ~26 ms, retained allocation zero; model/CPU/FPU/RAM/
  monochrome display match this SE. Later CPU/FPU/colour hardware remains pending.
- Physical new command images installed/validated with backups retained. Update
  warning/Enter tested; staging 72.4 s, verification 33.8 s. Owner reported ~4 min
  for Clarice to return during the first update; SEControl lost contact and was
  restarted manually. Final `-y` update capture confirms 1533 without another
  helper restart; coarse polling does not establish exact relaunch duration.
  Update handoff remains slow. [Evidence and limits](ux-six/REPORT.md).
- Final 1533 physical nano: queued Ctrl-underscore opens Go to Line; Ctrl-C
  cancels it; Option-X exits. In Command mode, 8 KiB file/search/Command-X pass.
  Three runs return to **420,960 bytes free heap**, owned remainder zero; peak
  portable allocation 74,572 empty / 101,736 populated. No editor save changes.
- Resumed physical loader profiling: cold empty **356 ticks / 5.93 s**, first
  and repeated 8 KiB **190 ticks / 3.17 s** each. Cold preparation 139 ticks falls
  to 4/5; warm metadata validation 108/107 dominates, followed by reading 38/35,
  relocations 22, record lookup 14/16 and imports 4 ticks. Full payload verification
  is already skipped for this recognised installed image. These are loader times,
  **not submission-to-editable-screen timings**; the three-second total target is
  still unmet. Next focus: physical allocation/preparation and metadata-pass cost,
  preserving current verification policy; no speculative renderer or nano rewrite.

## Historical terminal graphics investigation — 15 November 2026

- Inspected original NCSA Telnet 2.7b5 and BetterTelnet 2.0fc1 source archives,
  plus pinned Flynn source; attribution, licences and source hashes recorded.
  No third-party code copied into Clarice. No runtime changes, build or deployment.
- Native text runs, pixel scrolling, dirty tracking, glyph reuse and output
  batching largely confirm techniques already used. Flynn's retained full
  bitmap/shadow costs more RAM; its glyph loop offers no demonstrated advantage
  for large dense history jumps. Direct visible erase/draw would compromise
  completed-row presentation. No suitable low-risk experiment justified.
- **1525 remains unchanged**, with previous physical dense redraw 416 ms and
  glyph work approximately 253 ms. No new application performance or regression
  results claimed. Prioritise physical loading/nano profiling; optional dark
  inversion or sparse-glyph ideas first need targeted cost evidence.
  [Investigation and ranked opportunities](terminal-history/REPORT.md).

## Direct aligned output — 15 November 2026

- Accepted baseline measurements attributed about **98 ms/frame** to a separate
  row-alignment pass. Eligible Monaco 9 ASCII history spans now compose directly
  at the required bit phase in the same 4096-byte buffer. No additional bitmap,
  persistent allocation, renderer preference, cursor or wider renderer change.
  QuickDraw clipping and existing short-span/font/selection fallbacks remain.
- Three alternating pairs, 24 completed large-jump redraws each, 80×24/light/128
  history lines; quiet SEControl. Physical dense **678 → 599 ticks**, mean frame
  **470.8 → 416.0 ms (11.7%)**. All pairs improve: 678/599, 679/601, 678/598.
  Directory-style **264 → 265 ticks**, effectively unchanged (183.3 → 184.0 ms).
  Snow dense **579 → 471 ticks (18.7%)**; directory **228 → 228**. Paired heap
  delta zero. These are complete redraw timings, not new `ls` throughput results.
- Across all matched draws Snow is about **20% faster** than the SE; the retained
  path ranges from 16% to 27% faster by workload. SE delivers 79–86% of Snow's
  speed here. The former provisional 60% assumption is not supported for these
  workloads; do not generalise this ratio to disk I/O, loading or nano.
- Physical dense presentation **158 → 61 ms/frame**; alignment **97 → 0.7 ms**,
  the residual including checks/timing overhead. Glyph work increases **204 →
  253 ms**, consuming about half the avoided pass. Text/glyph work now accounts
  for about 61% of the 416 ms total. Counters are coarse and nested; medians do
  not sum exactly. Dense redraws remain expensive.
- Host: 1,339 actual M68000 direct-composition cases with bounded/even accesses
  and read-only glyph storage; 8,034 glyph/phase/theme cases; 356 existing
  compositor/ABI cases. Existing tail-clearing, selection and scroll guards pass.
  An exactly fitting tail's unnecessary zero-byte write was corrected before
  physical installation; a strict write guard now catches it.
- Snow and physical diagnostic: **322 dirty-buffer/clipping comparisons** pass,
  including actual occlusion, heap delta zero. Each theme passes 22 bursts + 20
  views, zero text mismatch and 147 clean scrollbar checks. Real selection,
  dragging and output/pager Ctrl+C (130) pass on both machines. User watched SE
  rapid dragging: **no blanking or missing scrollbar**. Snow's 80-sample drag
  captured no transient blank bands; sub-sample flashes are not excluded.
- Genuine nano opened the 8 KB fixture and exited in Snow; subsequent native
  pixel/bar checks pass, owned remainder zero. No new nano total-startup,
  safe-save or physical nano timing claim. Normal 1525 native pixel/bar checks
  pass in Snow and on SE; normal output/pager Ctrl+C returns 130 on both.
- Normal resource fork **940,593 bytes, +556 over 1521**; no additional persistent
  buffers, partition (768 KiB) and stack reserve (64 KiB) unchanged. Code growth
  is a small memory cost, not a new cache. Final physical free heap **414.06 KB**
  (423,998 bytes), 558 bytes below 1521; owned remainder zero. Retained recovery
  applications include the 1521 baseline; no rollback rerun claimed.
- **Recommended next priority:** current physical command-loading and nano-startup
  profiling before more renderer work. Latest controlled Snow startup is 7.23 s
  empty / 10.08 s for 8 KB. This task's cold nano loader alone is **4.85 s**:
  read 61, validation 85, imports 4, relocations 19, preparation 110 ticks.
  Investigate cold allocation/preparation and metadata work first; full payload
  checksum is already skipped for verified images. No loader redesign begun.
- Earlier accepted glyph reuse, redundant-clear removal and row alignment remain.
  [1517](jump-perf/REPORT.md), [1519](clear-perf/REPORT.md), [1521](align-perf/REPORT.md).
  Their separate diagnostic binaries are not interchangeable timing baselines.
  Dark-to-light invalidation and updater `Y/n`/`-y` are addressed in the UX work above.
  [Current report](direct-perf/REPORT.md), [measurements](direct-perf/measurements.json).

## Listing throughput — 15 November 2026

- Build **2026111514** removes temporary numeric formatting/copy work and uses
  bounded 68000 printable-run copying and aligned row clearing. No HFS changes,
  new permanent cache, partition increase or drawing/cursor redesign.
- Controlled alternating old/new Snow runs, real `ls -la` through final prompt:
  **1,000 files 10.18 → 9.62 s (5.6%)**; 100 files 2.60 → 2.55 s. Three-run
  medians; equal-depth command paths, same session/dataset/geometry. Module median
  562 → 529 ticks; paint 83 → 82; seven large-listing frames throughout.
- Chunked 96-tick bulk presentation, complete rows, immediate first/pager/final
  display and two-tick input polling remain. Earlier cadence/output work reduced
  1,000-file time 13.20 → 10.03 s; [previous evidence](chunk-perf/REPORT.md).
- Native HFS sync/async walks both measured 300 ticks, with 338 distinct catalog
  block reads; no safe speedup was demonstrated. [HFS evidence](hfs-cost/REPORT.md).
- Actual 68000 tests: 75,540 numeric fields, 8,911 copy cases, eight row clears;
  host formatter/writer/HFS/volume regressions pass. Normal guest output matches
  accepted data byte-for-byte; Ctrl+C status 130 in output and pager. Native scroll
  pixel mismatches zero (24 bursts + 20 views); 153 scrollbar checks also match.
- Normal idle free heap **416.16 KB**, 128 bytes below prior build; invocation
  owned remainder zero. Core +280 bytes; application resource +412 bytes.
  Renderer remains automatic native QuickDraw, with no renderer preferences.
- Separate-run timing shifts of about 44 ticks remain unexplained; use the
  interleaved paired result, not the fastest isolated run. Nine-plus seconds is
  still slow; timestamp reuse varies with content. No new nano measurement.
- Physical SE (CPU code 1, user-reported stock mode/4 MB/6.0.8): unchanged
  452-file directory, old build 1506 → normal 1514 listing median **8.43 → 5.47 s
  (35% faster)**. Ordinary loader/execution counter **10.03 → 6.10 s**; this excludes
  PATH/final prompt. Cumulative deployed improvements, not today's isolated gain.
  After-listing heap 415.99 KB (425,970 bytes), owned remainder zero; Ctrl+C 130 in
  output and pager. Setup was cancelled at 452 files; do not call it a 1,000-file test.
- Physical missing scrollbar section was captured in old build 1506. It has not
  reappeared in 1514 after listings, pager, thumb dragging, overlap/focus switching
  or resizing. Native check: 22 bursts + 20 views, zero pixel mismatches; 147
  scrollbar checks, zero mismatch. This is focused evidence, not a universal fix claim.
- Full power cycle restored Wi-Fi after MacTCP -23015; cause unproved. New update
  downloaded in 63 s, staged in 72.40 s, verified in 34.38 s, automatically relaunched.
  Previous app: `Macintosh HD:.cu-000001c6-78c4021a.old` (resource 939,251 bytes).
  No updater/firmware changes made. Other installed commands preserved.
  [Current report/build](output-perf/REPORT.md), [physical measurements](output-perf/physical-measurements.json).

## Implemented and tested scope

- Native System 6 shell, external `/bin` utilities, HFS `/mnt`, symlinks, PATH,
  preferences, bounded history/scrollback and finite spool pipelines. These are
  Unix-like subsets, not full POSIX compliance or live Unix processes.
  [Userland](userland-next/TEST-REPORT.md), [preferences](preferences-text/TEST-REPORT.md).
- QOL-1 cross-volume resolution, QOL-2 input batching/fail-closed overflow and
  QOL-3A changed-region selection have focused host/Plus guest evidence:
  [filesystem](qol1/TEST-REPORT.md), [input](qol2/TEST-REPORT.md),
  [selection](qol3a/TEST-REPORT.md). Physical cursor rendering is working; Snow
  artifacts alone do not justify changing it.
- Independent CLX command installation preserves existing applications at startup;
  explicit replacement/repair retains recovery and checks runtime compatibility.
  [Task 2](task2/TEST-REPORT.md), including 43/43 guest acceptance checks.
  Installation-verified fast-launch code and explicit strict verification are
  present; do not infer a new nano benchmark from host tests in clx-fast/.
- Genuine GNU nano 9.2 tiny and ncurses 6.5 edit and save in Snow. Same-volume
  staged saves preserve the resource fork and applicable Finder metadata, with
  fault/recovery tests. [Safe-save evidence](safe-save/TEST-REPORT.md).
  This is recoverable replacement, not atomic or guaranteed power-loss safety.
- Runtime allocation is demand-driven within available application heap and
  recovery headroom; the former universal 96 KiB ceiling is obsolete. Focused
  demand/exhaustion tests reached 346,080 bytes with zero retained allocations.
  Stream update/seek/error services pass focused guest tests.
  [Runtime evidence](runtime-completion/PROGRESS.md).
- Serial 57600 8N1, local PTY shell and bounded E-Kermit binary transfers have
  host/Snow coverage. [Serial-3](serial-3/TEST-REPORT.md). Physical short echo,
  reconnect and binary transfer evidence is in [hardware progress](hardware-update/PROGRESS.md);
  tested USB serial setup requires host pacing of 2 ms per byte for Kermit.
- SEControl is separate development infrastructure. Physical Wi-Fi screenshots,
  keyboard/mouse control, Finder/Clarice launch and disconnect recovery work.
  [Graphical-control report](hardware-control/graphical/REPORT.md).
- External HTTP wget uses MacTCP, bounded streaming, incomplete-file cleanup,
  errors/cancellation and progress displays. Physical CLX download, verification
  and appinstall have passed. No HTTPS/authenticated internet update claim.

## Performance and limits

- Normal application partition request is 768 KiB, with 64 KiB stack reserve.
  History payload is 16,896 bytes; normal rendering uses no bitmap cache.
  Diagnostic cached-renderer comparisons allocate 29,696 bytes temporarily.
  Command/editor working allocations are released; there is no resident nano.
- Nano's three-second target remains unmet. Latest recorded controlled repeated
  dispatch-to-ready times after small-allocation-page optimisation: empty 11.15 s,
  8 KiB 14.07 s (previous 12.17/17.68 s). Temporary 8 KiB workload peak increases
  about 21 KiB; no idle arena. [Timings](nano-startup/REPORT.md).
- Native APPL/CODE alternatives were reviewed, not installed as a general worker
  backend. [Loader review](loader-review/REVIEW.md). Separate command partitions,
  general live pipes, complete awk system()/command pipes, SDK, Telnet and SSH
  remain unimplemented or incomplete; no new work is authorised by this list.
- Physical SE: user reports 4 MB, System 6.0.8, MultiFinder and BlueSCSI networking.
  MiniPerformer+ remains installed but was bypassed with programmer switches;
  subsequent CPU code is 1, consistent with 68000. Earlier CPU-code-3 timings
  are accelerator-state-unverified and must remain separate.

## Current updater acceptance

Private HTTP manifest/package uses CRC-32, lengths and native compatibility;
CRC detects accidental corruption and does not authenticate the source. Both
forks are staged beside the application. An independent native helper retains the
previous application, commits only after the running app exits, and relaunches.
The System 6 unavailable HOpenResFile trap and documented resource-fork scratch
bytes have been addressed; native launch uses a working-directory reference.

Snow full normal-build configured-source workflow passed: 2026111402 to
2026111502, automatic relaunch, --check and pwd. Extracted previous/new forks
and applicable Finder metadata match (hardware-update/snow-full-integrity.json).
Snow HTTP transport is an HFS fixture, not network acceptance.

Physical SE **passed** the same normal `clarice update` workflow over MacTCP/Wi-Fi.
Downloaded 919.50 KB in 58 s (15.85 KB/s); staged readback 2113 ticks (35.22 s).
After confirmation the independent helper installed and automatically relaunched
build 2026111502; --check and pwd succeeded. Active app is
`Macintosh HD:Clarice Update`. Retained previous target:
`Macintosh HD:.cu-00000a12-78c4020e.old` (data 0/resource 941423 bytes).
Original Clarice and Clarice Dev also remain unchanged. Configured source:
`http://<MACBOOK_LAN_IP>:8080/update.txt` in `/etc/clarice/update.conf`.

SEControl's heartbeat timed out during handoff; user reopened SEControl only,
then remote commands verified Clarice's automatic relaunch. Exact relaunch latency
was not measured; physical forks were not exported for host byte comparison.
The one-time binUnpk bootstrap needed its omitted bundle flag restored using a
disposable probe; no new production utility was added. Subsequent updates need
neither binUnpk nor manual Finder/restart operations.

[Tested package and command](hardware-update/accepted-build/),
[physical result](hardware-update/physical-update-acceptance.json),
[full evidence and limitations](hardware-update/PROGRESS.md).
Existing native rollback and 482 host fault scenarios remain relevant evidence.
No power-loss safety, authentication, public update service or peak-memory claim.
[Pending hardware checks](PENDING-HARDWARE-TESTS.md). No further updater expansion.

## Physical performance programme — in progress

- Full SE shutdown/restart restored Wi-Fi after MacTCP -23015. Exact network
  cause unproved. CPU code 1 rechecked after power cycle. First launch
  main-entry-to-ready 331 ticks (5.52 s), excluding OS loading.
- Static binary-search CLX import table preserves 180 production/181 diagnostic
  bindings. Physical `cat` import resolution **16 → 2 ticks**; observed loader
  **45–56 → 35 ticks**. After-run heap 424918 bytes, owned remainder zero.
  No persistent cache or allocation; isolated lookup code/constants shrink
  6970 → 3438 bytes. ASan/UBSan lookup checks, 61 loader/installer scenarios
  and validation of 24 real command images pass.
- Snow repeated nano: empty **449 → 434 ticks** (7.48 → 7.23 s), 8 KB
  **623 → 605 ticks** (10.38 → 10.08 s); loader **203 → 187 ticks**.
  Import phase 17–18 → 3–4 ticks; metadata validation remains 85 ticks.
  Before/after diagnostic configurations have different heap layouts; total
  change is not an isolated measure of the lookup improvement. Three seconds
  remains unmet. Physical loader-only measurements are now recorded in the UX
  section above; a controlled physical submission-to-editable-screen run remains
  outstanding.
- Snow safe-save regression preserves resource fork, type/creator, flags and
  creation date. Repeated nano exits return tracked allocations to zero.
  Four selection probe cases pass with at most one tick/highlight, no pixel
  mismatch or heap delta. Synthetic one-tick input lost 24/240 characters;
  that stress case is **not passed**; physical confirmation remains pending.
- Physical scroll benchmark: 30 frames native 259 ticks, cached C 500, cached
  assembly 460; worst frames 8/16/15 ticks. Four cache release cycles stable,
  zero test failures. No rendering preferences or cursor changes made.
- Host-only quiet mode and bounded 120 s listener timeout added; 14 focused
  host tests pass. Screenshots work again after the power cycle.
- Pre-exit update warning is implemented and visible on physical hardware.
  Physical profile package download 56 s; staging 4023 ticks (67.05 s), readback
  2088 ticks (34.80 s). Automatic relaunch then unexpectedly quit with error 28.
  Opening the installed application from Finder succeeded; --check confirmed
  build 2026111506 and `cat` worked. Cause not isolated. The later normal 1514 update/relaunch passed; see above.
- The same profile package passed Snow automatic update/relaunch; extracted
  old/new forks and metadata match expected contents. Incremental Retro68 build
  dependency on the version stamp fixed after a mismatched package was safely
  rejected. Normal experimental build 2026111507 was not deployed; it is superseded by
  the accepted normal 1514 deployment above. No diagnostic nano installed on the SE.

Evidence: [measurements](physical-perf/measurements.json), screenshots and focused
logs in [physical-perf](physical-perf/). User priority is now Snow interaction and
rendering performance first, physical retest after meaningful gains, then updater
error 28. Keep the working application and previous versions.

### Snow interaction/rendering — 15 November

- These earlier runs were Snow-only while the SE was shut down. The provisional
  60%-speed estimate was not measured; current matched scrolling gives 90.4%
  throughput. Neither number is a general conversion for other workloads.
- Source audit found earlier scrolling/selection implementations still present
  relative to v0.4.0; installed SE binary/preferences equivalence remains unproved.
  Typing after selection now clears only the affected region: eight matched
  guest cases fall from 32–45 to 3–4 ticks with identical pixels. No allocation.
- ncurses uses the existing strip to batch rows and inverse spans; parser avoids
  dirtying rows solely for attribute bytes. 8 KB first paint 118 → 96 ticks;
  cold total ~12.27 s, warm empty 6.80 s, warm 8 KB 9.65 s. Three seconds unmet.
  Nano edit/search/save passed, both forks/metadata preserved, exit allocations
  zero. These preceding results are retained, not rerun in the drawing pass.
- Matched same-build drawing probe identifies redundant full cached-screen
  presentation and duplicate Control Manager drawing. Reuse native pixel scroll
  for cached presentation, copy exposed rows, retain the staged scrollbar bitmap.
  Full redraw and covered/clipped fallback remain. No new persistent allocation.
- Complete 100-file ls median: native 173 → 172 ticks (effectively unchanged),
  cached C 191 → 177, cached assembly 186 → 174 (6.5% shorter). Assembly output
  sequence 353 → 197 ticks (44.2% shorter); history 281 → 218 (22.4%). Native
  history 231 → 199 (13.9%). These are guest timings, not physical acceptance.
  Assembly worst history frame is still 18 ticks; smooth drag latency unproven.
- Light and dark native/cached pixel checks pass: each 24 bursts + 20 views, zero pixel
  mismatches and 153 scrollbar staging/reference checks with zero mismatches.
  Focused host selection and 1560 scroll-guard checks pass. Ctrl+C stops native
  and cached assembly 1000-file listings, paged/unpaged, with status 130 and prompt
  restoration. Stock Snow modifier delivery failed; existing patched Snow fixed
  the automation without Clarice input changes. Normal quick scroll/ring/copy
  checks pass; four cache release cycles stable (29704 bytes including overhead).
  Restored native heap 425716 bytes, tracked remaining zero. Guest API 0607 is
  the previously documented 6.0.8 media/resource discrepancy, not a new OS claim.
- Native strip batching and scrollbar changed-scanline-copy experiments were
  measured and discarded. Only the invalid-row cursor-bounds correction remains
  from the strip experiment. No cursor design change.
- Normal experimental build 2026111509 compiled; resource 938732 bytes (+238
  versus 1508), no new heap cache. Matched probe heap stable within renderer;
  whole probe delta -2 bytes. This stage was not deployed; later 1514 hardware
  acceptance is recorded above. No push or publication.

[Drawing evidence and limitations](draw-performance/REPORT.md),
[matched counters](draw-performance/matched.txt),
[normal experimental build](draw-performance/experimental-build/Clarice.bin).
Earlier interaction/editor evidence: [Snow measurements](snow-performance/measurements.json).
### Listing, enumeration and scrolling — 15 November

- Normal experimental build **2026111511**; Snow Plus/4 MB/System 6.0.8 only.
  The SE was unchanged during that earlier stage; current deployment is above.
  No push/publication.
- Earlier duplicate executable-selection removal remains: complete warm native
  ls medians 10 files 118→108 ticks, 100 files 173→163. Loader checks unchanged.
- Deeper 1,000-entry scan counters put 284–289 ticks (~4.75 s) in native HFS
  calls, versus 10–13 setup and 24–33 name handling. Cost per 100 entries stays
  roughly constant. Later [HFS investigation](hfs-cost/REPORT.md) shows 338 unique
  catalog reads per scan and substantial cache-hot CPU cost; exact CPU/I/O split
  remains unmeasured.
- Reuse the synchronous catalog parameter block and avoid an intermediate name
  copy; restore explicit volume/parent/index every call. Normal warm scans now
  **323 ticks (5.38 s)** versus prior 336–338; cold first pass369. Same-image
  controlled reuse experiment supports a ~4% gain. Still too slow.
- Native small-scroll heuristic now accounts for per-row Toolbox costs. Matched
  four-row rendering: six frames 82→57 ticks using pixel reuse; large moves can
  be slower, so retain patching there and leave cached weighting unchanged.
  No new cache/idle allocation; normal resource +13 bytes, core CLX -20 bytes.
- Listing already streams bounded rows; captured output advances before the
  scan completes. More frequent painting alone was rejected: 100-file internals
  110→213/336 ticks at six/three-tick intervals. Smooth single-line bulk display
  remains unmet. Prior actual drag captures remain ~250–298 ms native and
  ~195–254 ms cached; no new end-to-end drag-latency claim this pass.
- New ASan/UBSan listing-loop tests and existing fail-closed volume checks pass.
  Actual two-volume guest fixtures pass for spaces, hidden/encoded names, cd/pwd,
  relative/absolute listings, disposable operations and missing-path errors.
  Native pixel checks: 24 bursts/20 views, zero mismatches; 153 clean scrollbar
  checks. Four Ctrl+C cases pass (native/cached, paging off/screen), status130.
  Final tracked allocations zero. No cursor design or loader validation changes.
- Follow-up catalog evidence is now above; async gave no gain. Retain chunked
  presentation and investigate remaining non-native work before painting more often.

[Current evidence](catalog-perf/REPORT.md), [experimental build](catalog-perf/experimental-build/Clarice.bin).
Earlier [dispatch/drag evidence](list-drag/REPORT.md) retains its original measurements.

## Development checkpoint — 15 November 2026

Application CODE/resources through 1533 have physical-SE acceptance above.
Bundled utilities do not automatically gain fresh hardware acceptance: wget and
updater were exercised physically; Kermit regression was Snow/host; other utilities
retain earlier evidence. [Exact CLX build inputs](ux-six/bundled-inputs.json) record
hashes, not new acceptance. Existing pinned build scripts remain authoritative;
generated resources/packages and local configuration stay outside Git. Generated
timestamps are not guaranteed bit-identical on rebuild. Focused input, progress
and confirmation checks are rerun for this checkpoint; no fresh hardware run.

## Independent native networking foundation — 17 November 2026

Isolated in `Clarice-Network` / `codex/native-network-foundation`; the 0.5 RC
checkout is unchanged. Optional 68000 ipconfig (1,756 bytes),
ping (3,944) and hostname-enabled wget (19,328) build with existing CLX imports.
Shared command-local .IPP/DNR services add no resident shell service or settings
writes. Public-interface research and implementation are in
[the networking report](network-foundation/REPORT.md).

Host sanitizer/fault tests pass. Snow Plus/4 MB now passes all four pairings:
MoofTCP 1.0 / BlueSCSI-patched Apple MacTCP “2.1”, each with MoofPort 1.0 and
DaynaPORT SCSI/Link from the 7.5.3 installer. Follow-up adds 15 byte-exact 4 KiB
hostname downloads, native status, ICMP replies/timeouts, DNS/HTTP errors,
ICMP cancellation/recovery on all three additional pairings, and active HTTP
cancellation/partial-file cleanup on both DaynaPORT pairings. Five repeated
HTTP invocations per pairing show free heap within two bytes, constant largest
block (391,182 or 391,218), and zero tracked remaining allocations.

[Matrix evidence](network-foundation/matrix/README.md) records exact binaries,
configuration and limits. Restoring the missing native DA Handler enabled the
fixture's MacTCP Control Panel; selecting Ethernet fixed an initial LocalTalk
misconfiguration. No Clarice production changes/rebuild were needed this pass.
Direct System-file `vers` evidence identifies 6.0.8 despite SysEnvirons reporting
6.0.7; this corrects the prior fixture label. No full physical matrix/System 7.1 acceptance,
unmodified MacTCP 2.0.6 acceptance, or performance comparison is claimed.
DNR cancellation must drain the installed resolver; no public hard-abort guarantee.
No release, push or RC changes. FTP follow-up below; Telnet not begun.

## Native FTP follow-up — 17 November 2026

Small external `ftp get/put`, 19,056-byte 68000 CLX, reuses native transport/DNR,
HFS operations and progress display. Bounded streaming, passive EPSV/PASV,
password stdin, explicit errors/status, non-overwriting staged downloads. PUT
uses STOR; failed uploads may leave a remote partial file. Data forks only; use
MacBinary for complete Macintosh files. No new Core service/idle allocation.

[FTP evidence and usage](ftp/REPORT.md): sanitizer/fault tests and Snow
MoofTCP/DaynaPORT plus Apple MacTCP/DaynaPORT binary round trips pass; six64KiB
round trips, empty files, hostname/numeric hosts, missing/existing-file errors.
Byte comparisons pass. Moof download cancellation, timeout cleanup and subsequent
FTP execution pass; upload cancellation host-tested only. Three repeated cycles
per stack keep free application heap within2bytes and largest block constant;
zero tracked remaining allocations. Native driver system-heap peak/stability
was not directly measured. Physical FTP follow-up below; System7.1 acceptance remains pending.

Physical SE foundation smoke test: existing Core1550 unchanged, temporary CLX
files only. ipconfig returned192.168.1.252/255.255.255.0; named HTTP download
returned0 for4096bytes. Ping to MacBook timed out(-23035), so no physical echo
reply acceptance. `cksum` absent: physical download byte integrity unverified.
[Screen evidence](network-foundation/physical/result.png). No installed network
configuration, app, utility collection or recovery copy was replaced.

### Physical FTP acceptance — 17 November 2026

Existing SE Core1550 and installed network stack unchanged; FTP executed from
/tmp only, isolated from0.5 RC. Five64KiB round trips byte-exact, empty GET/PUT,
missing550 and existing-file refusal pass. First script was manually interrupted;
rerun preserved its first successful download and correctly refused that existing
target. [Actual logs/screens/limitations](ftp/REPORT.md#physical-se-follow-up--17-november-2026).

Ctrl+C download cancellation and Command-period upload cancellation both returned
130; no partial local destination or .ftp temporary files remained. Upload
warning correctly permits a remote partial/complete file: in this test all bytes
had already been queued and the complete file remained on the server. Fresh
GET/PUT after both cancellations passed byte comparison. Five repeated runs:
application heap387,038–387,040, largest386,986 stable, remaining0; recovery pair
heap394,952/largest394,784 unchanged. Native system-heap peak remains unmeasured.
64KiB upload totals320–371ticks including loader51–69ticks; no claimed perfectly
isolated throughput benchmark. No production changes required; servers stopped,
SEControl retained, no Core/bin/config updates, no push/release. Telnet not begun.

Open Transport is not a System6/stockSE target: Apple specifies68030/040 or
PowerPC, supported System7.1+ releases and5MB minimum on68k. Future suitably
configured PowerBook180 testing should first try OT's MacTCP compatibility
interface; DNS/resource discovery and ICMP need explicit testing. No OT backend
or compatibility claim added; source in network-foundation/RESEARCH.md.

## Native Telnet client — 17 November 2026

External networking branch now adds bounded monochrome VT100/ANSI to the accepted
NVT client: `telnet [--dumb] HOST [PORT]`. Reuses native TCP/DNR and Core's existing
nano screen/keyboard/rendering services; no Core or FTP changes. TTYPE/NAWS,
cursor/erase/wrap/scroll regions, insert/delete, inverse, application arrows and
status replies. Ctrl+] then q exits; Command-period cancels; Ctrl+C sends IP.

[Actual implementation, evidence and limits](telnet/REPORT.md#vt100ansi-acceptance):
ASan/UBSan,200k VT fuzz bytes, Core pixel-state bridge comparison, allocation and
acquisition failure, cancellation and repeated cleanup pass. Snow Plus/4MB/6.0.8
then physical stock SE/4MB/6.0.8 passed real macOS Vim9.1 edits/saves (host bytes
verified), paging/search, and less668 search/inverse display. Physical application
arrows verified on the wire. Cancel130, reconnect/remote exit0, local escape0 and
local terminal restored. Snow warm heap403,262/largest401,056; SE
heap402,672/largest398,880 stable, remaining0. System-heap peak not measured.

CLX17,924B, runtime image12,612B; VT model11,116B plus existing on-demand Core
screen10,804B and native ring16,384B. No idle allocation or extra bitmap. Physical
load52–54ticks; no all-in latency or redraw-speed claim. Burst painting bounded
to4KiB with input checks each512bytes. Client remains an explicit VT100 subset:
ASCII/monochrome, approximated DEC graphics, no colour/UTF-8/bold/underline/blink,
alternate-screen history, VT52, application keypad/function-key translation,
TLS or urgent SYNCH. Session dimensions fixed. System7/OT/arbitrary telnetd and
remote GNU nano unverified. Host nano is Pico; only Vim/less are claimed.

Temporary servers/Snow stopped, all seven PTY shells reaped, SEControl retained.
Only `/tmp/telnet-vt2` downloaded on SE; installed Core/bin/settings and0.5 RC
unchanged. No push/release. Prior NVT and real-shell evidence remains in the report.

## Networking distribution policy

Network status/ipconfig, ping, wget, FTP and Telnet client are **Standard Unix
Utilities**, external `/bin` commands, gated on required validation/integration
review before promotion. The separate0.5 candidate remains unchanged. Shared
native TCP/DNR code is linked into invocation-owned CLX commands, not Core;
no networking startup/idle overhead or required stack/driver/BlueSCSI hardware.
Core works without networking. Reuse existing installer/appinstall and Clarice
Data; nano/editors remain optional, future Telnet server optional/off by default.
AGENTS.md and INSTALL.md record this policy; no new packaging mechanism.

## libtelnet port — Snow validated, 2026-11-17

[Port evidence](telnet/LIBTELNET-PORT.md) supersedes the earlier review-only
recommendation following explicit port authorisation. Unmodified pinned upstream
libtelnet now owns protocol/negotiation; thin invocation allocator adapter;
native TCP/DNR and VT renderer unchanged. No duplicate parser, idle allocation,
Core/FTP/0.5 RC changes. SE left off; previous source/binary and guest candidate
retained for recovery. No push/release.

CLX24,920B (was17,924), runtime18,260B (was12,612). Ordinary m68k protocol storage
574B plus allocator overhead; upstream SB capacity bounded16KiB, demand allocated
and released. Host ASan/UBSan, all11 upstream fixtures, allocation/growth failure,
queued negotiation, existing VT fuzz and restoration regressions pass.
Snow Plus4MB/actual6.0.8 MoofTCP/DaynaPORT: real Bash/Vim save/less search, IP,
local cancellation130, reconnect/exit0, local escape0, --dumb0; malformed/oversized/
truncated rejection1/1/1 followed by recovery0. Three PTY children reaped.
Repeated heap400166/400168/400166; largest399480, no progressive loss observed.
Matched short-session median165vs163 guest ticks; loader53vs48. Network deadline
included, not a CPU speed claim. All-system peak and physical performance pending.
Existing ASCII/renderer, binary-refusal and no-urgent-SYNCH limitations remain.
