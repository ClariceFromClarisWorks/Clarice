# SEControl recovery — 17 November 2026

Contained helper fix; no Clarice production, RC, network configuration, ROM or
OS changes. Existing SEControl Fast changes preserved. Previous helper retained
in ignored work/se-control-recovery/SEControl-before.bin. Nothing pushed.

## Implemented and measured

Previously a lost socket, failed send, malformed input or five-minute idle limit
left the main loop and quit the application. This explains some apparent crashes,
not all reported failures. New helper releases its mouse action, discards the
unfinished command, closes only its own TCP stream and retries after five guest
seconds. Explicit local/remote Quit exits. No queued command replay.

Native asynchronous TCPActiveOpen services Macintosh events while waiting.
Cancellation uses TCPAbort and waits for the outstanding PB to complete before
releasing it. Close uses owned-stream abort/release, not a shared-driver reset.
Failed release preserves stream/ring ownership; another retry cannot allocate
over it. A genuinely wedged driver remains an explicit recovery limit.

- 16 existing Python host tests pass.
- Sanitized C transport fault tests:30 cycles; allocation/create/open failure,
  timeout, cancellation before/during open, repeated release failure and later
  recovery. No leaked mock allocations or premature disposal of driver storage.
- Snow Plus/4MB/System6.0.8 fixture with MoofTCP and SCSI Ethernet/NAT: actual
  network reconnect after partial keyboard input, disconnect during a drag,
  interrupted screenshot, eight further connection cycles, and listener shutdown
  for10s followed by restart. Complete responses/captures required, not merely a
  successful TCP open. No partial-key replay; active/down0 after recovery.
- All eight settled free-heap readings26810B. Interrupted snapshot recovered.
  System-heap totals were not independently instrumented; do not claim an
  unlimited endurance test. Capture8–9guest ticks. One raw initial heap reading
  before first capture was4282B; later post-capture readings remained26810B.
- Ordinary reconnect3.75–4.26host seconds at Snow's Video setting; guest retry
  interval is300ticks. Listener restart5.79host seconds after listen resumed,
  including rejection of one stale NAT candidate. These are emulator interaction
  times, not stock-SE performance numbers.
- Explicit network QUIT and local Command-Q during an offline connection attempt
  return to Finder. Guest then shut down cleanly.
- Earlier accelerated/NAT runs accumulated stale attempts and the first test
  listener wrongly treated a stale socket as the active session. Failures retained
  separately. Final run used a clean NAT instance, normal emulation and bounded
  handshake/PING checks. Do not fast-forward real network timeout tests.

MacBinary97920→98176B (+256). Partition remains256KiB, receive ring16KiB,
mouse plan314B; no new persistent buffer/static connection state. Asynchronous
open uses temporary stack PBs. Physical-SE deployment and reconnect acceptance are recorded below; broad
stack/driver-failure and crash acceptance remain outstanding.

## Ready build / install

`work/se-control-recovery/SEControl.bin`, or application-only BlueSCSI media
`work/se-control-recovery/SEControl-Recovery.hda`. Source archive/licences and
SHA256SUMS beside them. No private config included; keep the existing
`SE Control.cfg`. Quit the old helper, preserve its file, copy the new SEControl
beside that config, then launch. Existing host control commands/listener work.
No Clarice update or network-setting change required.

## Independent recovery: unresolved architecture boundary

The USB serial device is now present at /dev/cu.usbmodem11401. No serial control
service has been installed or claimed working. Outbound serial/Telnet clients
cannot operate Clarice's local shell, and Telnet over Wi-Fi cannot bypass Wi-Fi
failure. A raw serial maintenance console is the appropriate independent route.

Smallest recommended next implementation: optional Clarice-owned serial endpoint,
status/cancel/helper-launch only, using the already tested native LC sublaunch
under MultiFinder. Add cooperative helper stop requests; do not emulate Unix
signals or provide arbitrary-process force killing. Reuse the native Serial
Driver and restore all pending I/O on shutdown. It must explicitly arbitrate
port ownership with existing serial/Kermit, which currently open drivers and
replace receive buffers themselves. Silently running both is unsafe. A full
independent shell/session and generic process manager are unnecessary here.

Do not merge graphical SEControl into Clarice: that loses failure independence
and moves development-only mouse-state code into the production renderer.
If Clarice itself crashes, a separate recovery application would be required,
with a further application partition; neither approach rescues a globally
non-yielding/crashed System6. No safe shared-.IPP unload/restart was established.
This broader service/lifecycle change has not been improvised into the renderer
experiment or release candidate.

## References and existing evidence

- [Apple MacTCP Programmer's Guide](https://bitsavers.org/pdf/apple/mac/developer/Networking/MacTCP_Programmers_Guide_1989.pdf): asynchronous PB lifetime, stream abort/release.
- [Apple PLAT04](https://leopard-adc.pepas.com/qa/plat/plat04.html): KillIO does not cancel MacTCP; TCPAbort does.
- [Apple cooperative multitasking](https://dev.os9.ca/techpubs/mac/Processes/Processes-13.html): applications must yield; interrupt routines are not general application recovery.
- [Apple Process Manager Q&A](https://leopard-adc.pepas.com/technotes/ps/ps_510.html): System7 APIs versus earlier native launch controls.
- Existing native-bench driver/results and Serial-1/3 adapters reused as design evidence, not rerun or newly claimed accepted.

## Physical handoff and reconnect

User requested remote deployment rather than a manual swap. Downloaded through
Clarice wget into a new SEControl Recovery folder, with a separate private host
listener/config. StuffIt opened without extracting; quit it normally. Adapted
the existing absent-only native-bench installer, reusing production MacBinary
parser/extractor, to install this exact embedded, hash-recorded helper. It
refuses an existing destination and creates no general package mechanism.

Snow first: successful installation/status0; repeat refused/status2. Offline
fork comparison confirms all executable resources exact. Native File Manager
recovery scratch changed at38 offsets within48..127, as already documented by
update/fork-crc.h; Finder also set its initialized flag. Dates/type/creator and
both fork lengths preserved. This is not literal byte equality of that mutable
resource-fork scratch header.

Physical: wget transferred98176B package and the one-off installer. Native
installation produced APPL/SECT, data0/resource97945B. New helper opened through
Finder while the previous instance remained connected; verified new screenshot,
mouse movement/status, then explicitly quit the old instance (BYE). Both app
files retained. Two deliberate socket disconnects automatically reconnected in
5.94 and6.16host seconds, no manual relaunch. Mouse active/down0 and settled
heap26802B before/after both. Subsequent fast typing/screenshot/Clarice commands
work. Old physical heap27068B versus26802B new:266B less free, unchanged
partition/ring/bitmap policy. New listener lives at work/se-control-recovery/physical.sock.
No driver-wide reset, real helper crash, Wi-Fi-radio outage or serial recovery
claim. The serial endpoint and Telnet server remain future independent work.

## Physical disk archive close-out

Performed through Clarice filesystem commands, not host disk-image edits. Two
old recovery apps were moved in the first pass; 23 further old builds/update
packages were moved into `Macintosh HD:Clarice Archive 20261117`. The actual
running app `.cu-0000039b-78c4021b.old`, two newest recovery copies, both helper
applications, user data and System folders were excluded. No old build deleted.

The installed older ls rejects -d; that first pass was cancelled cleanly after
two moves. A batch script then completed the remaining moves but its final cmp
was unavailable (status1, not a passing script). Read both successfully written
normalized manifests with cat, checked each read status, and compared them with
the shell test builtin: ARCHIVE_VERIFIED. Unset the temporary shell variables.
The comparison covers all 25 archived files plus the pre-existing manifest,
matching file/fork lengths, type/creator and flags after removing only parent
path prefixes. This is not a content checksum or a fresh creation-date proof.
A verified-manifest.txt is retained in the archive. Final root listing returned
to a normal prompt; ambiguous/unselected names were left alone.

The follow-up archive5 draft was not executed; it was superseded by manifest
verification. Deployment/test scratch files remain in /tmp; no broad deletion
of unknown files. Missing cmp/older ls demonstrate that this physical machine's
independent utility set is not the freshly validated 0.5 collection. No utility
upgrade, renderer change, commit, push or release was performed during cleanup.
