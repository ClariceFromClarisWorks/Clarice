# Automated Plus acceptance — executed 10 November 2026

**Task 2 guest acceptance passed: 43/43 extracted-result checks.** Existing Task 2
application and fixtures were reused. No Clarice/emulator rebuild, implementation
change, Task 3 work, push or release. Physical hardware is not tested.

## Environment and automation

Snow v1.5.0-a921b10 with the existing Plus keyboard patch, Macintosh Plus/68000,
4,194,304 bytes RAM, normal Video speed, no fast-forward or overclock, MultiFinder.
The System file's `vers` resources 1 and 2 explicitly identify **6.0.8**; SysEnvirons
reports `0607`. See [resource evidence](system-version.json), [configuration](environment.json)
and [guest status screenshot](system.png). Do not infer 6.0.7 from that API field.

Earlier AppleScript input failed with `osascript is not allowed to send keystrokes`
(error 1002). On this run System Events reported UI control enabled and a single
probe reached the guest. The permission transition's cause is unknown; no privacy
settings were changed or reset by the agent. Direct host text injection still lost
characters, even when spaced, so it was not used for test bodies.

Inspected alternatives: Snow's built-in clipboard typing and input-recording replay;
its bundled runner can replay recordings but forces uncapped speed and does not
provide this configured SCSI guest as a drop-in test. TUI documentation is historical
(the checked-out workspace lacks that frontend). No new framework or emulator
control service was installed. A tiny local CoreGraphics click helper drove the
existing **Type host clipboard contents** toolbar action, and short shell commands
launched guest-side scripts. Clipboard contents were restored afterward. Host text
commands/modifiers required careful pacing; Finder launch/activation was checked
visually. The scripts themselves were read by the guest from HFS, not typed.

A delayed initial accessibility paste and a later physical toolbar click caused
`first.sh` to execute twice. The second run correctly refused updates with retained
recovery files. Those refusal outputs are retained, not relabelled as successful
install statuses. `four.sh` uses the fresh `acceptprobe` name to capture clean
install, replacement and recovery statuses. No failed guest application test was
silently retried or discarded. Some typed timing-output filenames lost characters;
the raw files retain those names and are mapped explicitly in [timings.json](timings.json).

All destructive fixture operations occurred on `work/task2/automated/system.hda`,
a stopped-disk copy of the existing system image. The copied application's root
entry was updated with the already-built Task 2 MacBinary. The original editor
files were retained. `wc` was removed only from the disposable copy to exercise
startup installation of an absent bundled tool. Original ROM/OS media are not
included in published artifacts. Snow was stopped after guest exit/flush before
host extraction. The test instance is now stopped.

## Executed checks

[Machine-readable assertions](results.json) and [payload hashes](payload-hashes.json)
come from actual guest-created files. [Verifier](verify.py) checks those outputs;
it is not a rerun of the host regression suite.

| Requirement | Observed result |
| --- | --- |
| Install missing bundled command | Startup recreated missing `wc`; its bytes match the original bundled command and `wc --help` returned 0. |
| Install and independently replace | Fresh `acceptprobe` install/replacement returned 0; execution changed from `independent command v1` to `independent command v2`. |
| Restart persistence | `task2probe` returned v2 before and after a normal Clarice quit/reopen. Updated grep bytes before/restart/archive match the independent format-2 fixture exactly. Grep printed `alpha` before/after restart. |
| Explicit recovery | grep and probe recovery returned 0. Recovered grep data matches the original command byte-for-byte; probes executed v1 again. Updated grep was retained and archived unchanged. |
| Damaged-file preservation | Before and after another normal quit/reopen, validation returned 126 with checksum mismatch. Both damaged snapshots match the corrupt fixture exactly. Startup did not replace it. |
| Explicit repair | `appinstall repair grep` returned 0 and grep again printed `alpha`. Retained `.old` matches the corrupt fixture exactly and still returns 126 on validation. |
| Finder metadata | Type TEST, creator TST2, flags 4100, unlocked and invisible were identical in installed/backup files after replacement, restart and recovery. A fresh acceptprobe replacement independently verified the same metadata-copy behaviour. |
| Existing recovery files | Repeated updates refused with status 1 / Mac error -48 and the retained-version diagnostic; existing working versions remained usable. |
| Incompatible candidate | Wrong runtime major rejected with status 1 and an explicit incompatibility diagnostic. |
| Existing utilities | ls and df returned 0; grep printed alpha, sed and awk printed beta. |

Relevant raw evidence: [clean install status](accept-install-status.txt),
[clean replacement status](accept-replace-status.txt), [clean recovery status](accept-recover-status.txt),
[metadata before](metadata-before.txt), [metadata after restart](metadata-restarted.txt),
[recovery metadata](metadata-recovered.txt), [damage after restart](damaged-restarted.txt),
[repair result](repair.txt), [recovered probe](probe-recovered.txt).

Guest scripts actually executed: [first](first.sh), [second](second.sh),
[third](third.sh), [clean positive probe](four.sh). Screenshots show
[second completed](second.png), [third completed](third.png) and [positive probe completed](four.png).
The old manual instructions remain reproducible guidance, not the source of pass claims.

## Three ncurses loader measurements

Each unchanged `cursesdemo` run reached its ncurses screen and exited via Q
(exit-key 113); `status runtime` was captured before another external command.
“First” is the first demo invocation in this emulator test session, not a physical
cold-storage benchmark. TickCount uses 60 ticks/second.

| Run | Read | Validate/checksum | Imports | Relocations | Prepare | Total ticks | Seconds |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| First | 39 | 199 | 10 | 36 | 29 | 313 | 5.217 |
| Repeat 1 | 33 | 198 | 10 | 37 | 1 | 279 | 4.650 |
| Repeat 2 | 34 | 198 | 10 | 36 | 0 | 278 | 4.633 |

Raw records: [first](tmoe.txt), [repeat 1](tmt.txt), [repeat 2](timing-third.txt).
Screenshots: [first](timing-one.png), [repeat 1](timing-two.png), [repeat 2](timing-three.png).
All five phase sums exactly equal their recorded total here. Loaded image 103,300
bytes, tracked allocation peak 40,526 bytes and tracked remainder zero on all three
runs; largest free block 400,880 bytes. The raw redirected status files show free
heap 405,306/405,308/405,306 bytes; unredirected screenshots differ slightly because
output capture itself holds memory. Total command `ticks` includes our wait before
Q and must not be treated as launch time.

Validation/checksum dominates: about 64% on first load and 71% on repeat loads.
This phase includes cooperative event handling, so it does not isolate pure FNV
cost. First preparation cost falls from 29 ticks to 1/0; reading falls from 39 to
33/34. These are observations, not an optimisation. All loads exceed 180 ticks.
The nano three-second fully drawn/keyboard-responsive target is not met or tested
by these demo-loader measurements.

## Limits

Acceptance covers the requested normal Plus workflows and metadata fields exposed
by hfsattr. Extended Finder information, non-empty resource-fork preservation,
physical media failure, forced power loss, and full/read-only-volume faults were
not guest-tested here. Existing host fault tests remain separate evidence for
failure boundaries; do not upgrade them to guest or hardware results. No source
or ABI changes were made in response to this validation.
