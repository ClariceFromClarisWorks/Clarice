# Pending physical Macintosh SE checks

Hardware is operational again. This is a checklist, not a claim that every item
is pending or has passed. Executed serial/update checks are linked from
[CURRENT-STATE.md](CURRENT-STATE.md). The 15 November 1514 session passed update/
relaunch, focused listing cancellation/paging and scrollbar redraw checks;
[measurements and limits](output-perf/physical-measurements.json). Scrolling
fluidity remains inadequate (~7.2 fps in the matched native test).
Target 4 MB, System 6.0.8, verified cable, 57600 8N1, no flow control.
Use a backup storage image and unused SCSI ID; never modify BlueSCSI firmware.

## Offline installer follow-up (Snow validated only)

When hardware acceptance resumes, use disposable storage with a backup. Open the
new distribution's **Install Clarice**, click Install, and verify automatic launch
from System Folder:Clarice with all 43 utilities. Cancel partway with Ctrl+C and
rerun; verify existing independent utilities and configuration are unchanged.
Upgrade an older disposable Core, check Clarice Previous and both forks/Finder
metadata, then rerun without replacing the equal/newer Core. Check the busy-app
refusal under MultiFinder and launch under single Finder. Record System 6.0.8,
free/largest heap, time to completion and native launch errors. No physical
acceptance is claimed for this launcher or build 1550 distribution.

## Earlier capability checks

1. Startup/install: follow `serial-3/build/READ-ME.txt`. Time launch to prompt and
   first-run installation. Run `serial --help`, `kermit --help`, `status runtime`.
   Confirm existing commands/configuration preserved, and explicit appinstall repair
   retains its backup. Record hardware, System version, build hashes and timings.
2. Echo: launch `scripts/serial-echo.py DEVICE -b 57600 --log echo.jsonl` on MacBook
   (pyserial in a venv). On SE: `serial -b 57600 /dev/tty.modem`. Type numbered lines
   continuously; expect one echo and ordered bytes. Save host hex log and video.
   Test printer with `serial -b 57600 /dev/tty.printer` only when AppleTalk is off.
3. Disconnect: Ctrl+] then q (Option+] on a keyboard without Control), then
   `echo $?` → 0. Reconnect; Command-period then `echo $?` → 130. Unplug/replug,
   restart host peer and reconnect. Three-wire cable loss can remain silent.
4. Binary: build host peer using `scripts/build-kermit-host.sh`. Generate fixture:
   `python3 -c 'from pathlib import Path; Path("source.bin").write_bytes(bytes(range(256))*256)'`.
   SE: `kermit receive /tmp/se.bin`; host:
   `work/kermit/kermit -d DEVICE send source.bin`. Expect success on both.
   Host: `work/kermit/kermit -d DEVICE receive returned.bin`; SE:
   `kermit send /tmp/se.bin`. `cmp source.bin returned.bin` must return 0.
   Repeat with empty file and a prepacked MacBinary container. Confirm both forks
   and Finder metadata after unpacking using an existing tool. Plain transfers
   carry only data forks. No automatic MacBinary pack/unpack is implemented.
5. Interrupt a larger receive with Ctrl+C, expect guest 130, peer failure/timeout,
   no completed partial destination, then retry the same name successfully.
   Existing file receives must fail without changing it. Compare hashes again.
6. Memory: `status runtime` after each of ten transfers/sessions. Expect remaining=0
   and no progressive free-heap decrease after warm-up. Save the values. Quit
   Clarice normally, shut down the Mac, then verify stored files again.
7. UI: type/edit during ordinary shell work; traverse history; select across lines,
   drag scrollback and page `ls -la /mnt`. Record latency/flicker and responsiveness.
8. Sustained transmission: repeat 64 KiB binary round trips and long interactive
   input, recording bytes/time/errors. Compare with Snow only after matching payload,
   settings and start/end timing. Snow does not establish physical throughput.
9. BlueSCSI: compare files after repeated normal shutdown/restart cycles; verify
   unchanged unrelated files and stable mounted volume identity. Preserve logs and
   image backups. No power-cut durability or firmware acceptance is claimed.

## Terminal/runtime completion (2026-11-14; pending hardware)

- Verify Command-C selection copy, idle Command-V and queued paste during an
  ordinary command. Pasted newlines must not execute until explicit Return.
  Overflow must cancel/block submission; interactive editor/serial ownership must
  not feed the shell behind it. Verify inverse pinwheel and frontmost launch.
- Cursor rendering was already confirmed on the SE; this task did not alter it.
- Record `cursesdemo`, exit with q, then `status runtime` three times: report read,
  validation, imports, relocations, preparation and included event ticks. Snow
  repeat load 229–230 ticks is not physical acceptance or nano's three-second goal.
- Verify repeated large portable-tool operations and cancellation return memory.
  New runtime leaves a 32 KiB free-heap margin; physical partition/fragmentation
  behavior remains untested. Installed commands are never silently replaced.
- Use disposable documents for experimental nano hardware acceptance. Safe-save
  host/Snow checks now pass; physical media failures remain unverified (see below).

## Safe HFS replacement (14 November; physical tests still pending)

Using disposable files, run nano on an existing file with a known resource fork,
type/creator, Finder flags and creation date. Edit, Control-O/Return, Control-X,
reopen and compare both forks and metadata. Plus fallback: Option-O/Option-X.
Repeat saves; compare `status runtime` after each. Test a new file, locked file,
read-only volume and insufficient-space failure. Original must remain intact or
retained under the reported recovery path. Inspect any .clarice-*.new/.old files
before retrying. Do not infer power-loss safety from host fault tests or Snow.
See outputs/safe-save/TEST-REPORT.md and IMPLEMENTATION.md for tested boundaries.

## Nano end-to-end performance (14 November)

When hardware is available, measure Return to fully drawn responsive nano for an
empty file and an 8192-byte ordinary text file, first after boot and repeated.
Type a short known prefix, scroll 40 lines with Down, search using Option-W,
save Option-O/Return and exit Option-X. Compare exact contents and resource/Finder
metadata; record `status runtime` after each normal exit and Command-period cancel.
Collect wall-clock video/timings and heap/owned-peak/remaining counters. Snow now
measures ~12 s repeated empty and ~17.5 s repeated 8 KiB; these are not physical
results, and they exclude the initial command-echo redraw. See nano-perf/REPORT.md.
Cursor rendering is already observed working on the SE; do not change it for Snow.

## Nano small-allocation pages (14 November; no physical result)

Use the nano-startup build and the same empty/8 KiB fixtures described above.
Record cold/repeated editor readiness, typing/search/save, cancellation and
`status runtime` on exit. Portable peak now includes complete 2 KiB allocator
pages, so compare peak as well as remaining allocations (expected zero). Confirm
repeated exits return to stable heap and test clean failure with a constrained
application partition using disposable files only. Snow repeat 8 KiB improved
17.68 -> 14.07 s at a ~21 KiB portable peak cost; no new idle arena. Do not infer
physical Plus/SE speed or storage durability from Snow. See nano-startup/REPORT.md.

## SE Control first Wi-Fi proof (14 November, physical result)

Two complete screen captures and one `x` delivered to the foreground helper
passed on the owner's SE; QUIT received BYE. Captures took 1.198/1.086 seconds.
See hardware-control/REPORT.md and screenshots. This does not accept Clarice
performance, mouse/background control, Finder launch/quit/reopen, reconnection,
network endurance or application updates. Next qualify those controls and measure
heap/driver cleanup across repeated sessions; snapshot-time free app heap was
only 4,432 bytes. No production runtime changes.


## SEControl bounded graphical backend (14 November 2026)

New helper awaits physical deployment; Snow lifecycle and release tests pass.
Do not confuse the earlier Wi-Fi screenshot/x proof with mouse acceptance.
Artifact and exact commands: tools/se-control/README.md and
outputs/hardware-control/graphical/REPORT.md. Listener: <MACBOOK_LAN_IP>:5968,
permitted SE peer <SE_LAN_IP>, private existing token.

- Capture foreground Finder, then Clarice; record frame seconds/copy ticks/heap.
- Click exposed Finder title bar and Clarice icon; key cmd+o; type pwd --enter.
  Confirm screenshot contains executed pwd and returned prompt.
- key cmd+q; click icon/key cmd+o; capture reopened Clarice.
- Drag title bar and select a menu using a bounded 180-tick gesture.
- Start drag --nowait then cancel; verify active=0/down=0/error=0.
- Start drag --nowait then disconnect; confirm physical button is released,
  relaunch helper, reconnect and capture. Do not retry prior input automatically.
- Verify the physical SE mouse moves/clicks normally after gestures and quit.
- Repeat captures/status and compare idle heap; measure physical latency.

No physical mouse/ADB acceptance yet. No unattended recovery or firmware changes.

### SEControl physical results — same-day follow-up

The new helper was installed on the real SE. PASS: background Finder and Clarice
captures, click/Command-O launch, pwd/Return, title dragging, menu Cancel command,
Command-Q and reopen, bounded gesture cancellation with released button.
Intentional host disconnect after queuing a drag: user confirmed helper exited
and real mouse worked; manual relaunch reconnected, captured and accepted x.
Evidence: outputs/hardware-control/graphical/physical-results.json and REPORT.md.
Normal screenshots 0.997–2.750 s, copy 1–3 ticks; startup-blocked captures
10.838/11.170 s. Idle heap stable around 26.2 KB after captures.
Still pending: long endurance/system-wide leak measurement, CPU percentage,
unplugged network/long outages, broader application/menu/keyboard layouts.
No unattended recovery. Earlier pending bullets above are now satisfied only
to this focused extent; no unrelated Clarice hardware milestones are claimed.

## Wi-Fi development follow-up (14 November 2026)

Physical proof and current limits: outputs/hardware-update/PROGRESS.md. 57600
short echo/reconnect/local cancellation passed; HTTP CLX download/check/install
and a 16 KiB binary round trip passed. Kermit host-to-SE bytes (including ACKs)
need --pace-ms 2 for this tested USB cable setup; unpaced transfers failed.
MiniPerformer+ was installed with hardware acceleration unverified. User is
resetting with newly fitted programmer switches: record CPU report again before
claiming stock SE timings. Original Clarice app is preserved; Clarice Dev was
installed side-by-side manually. No self-update/replacement acceptance yet.

Pending: deploy/test runtime 1.1 progress bars and detailed HTTP statuses on SE;
repeat serial/HTTP timings after accelerator reset, sustained bidirectional load,
HTTP cancellation and timeout recovery, system-heap stability over repeated TCP
connections. Snow progress/error tests used mock transport, real native HFS.

## Private HTTP self-update — accepted 15 November 2026

Full sequence passed first in Snow (HFS-backed HTTP fixture), then on the physical
SE over real MacTCP/Wi-Fi: check, download, verification, staging, explicit yes,
exit, independent replacement and automatic relaunch. Installed 2026111502;
--check and pwd work. Active copy Macintosh HD:Clarice Update; retained previous
Macintosh HD:.cu-00000a12-78c4020e.old (0/941423-byte forks). Original Clarice and
Clarice Dev preserved. See hardware-update/physical-update-acceptance.json and
PROGRESS.md. User reopened SEControl after a heartbeat timeout during handoff;
no manual Clarice relaunch was required. Exact relaunch timing and raw physical
fork export were not measured. CRC is not authentication; no atomic/power-loss
claim. This milestone is complete; do not expand updater work.

Outstanding separate work: physical performance profiling (next user priority),
long-term memory/network/storage endurance, and the unrelated checks above.
A MacBinary CLI utility was discussed but not authorised or implemented here.
