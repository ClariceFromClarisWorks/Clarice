# Real SE development workflow

15 November 2026: **self-update acceptance complete in Snow and on the physical SE**.
The physical `clarice update` installed build 2026111502 and automatically
relaunched the same application. `clarice update --check` reports 2026111502;
`pwd` returns `/`. Recovery application remains at
`Macintosh HD:.cu-00000a12-78c4020e.old` (data 0, resource 941423 bytes).
Original `Clarice` and `Clarice Dev` also remain untouched. Active updated copy:
`Macintosh HD:Clarice Update`. Stop this milestone; no push/publication.

Evidence: [physical acceptance](physical-update-acceptance.json),
[relaunch screen](se-update-relaunch.png), [backup](se-update-backup.png),
[Snow integrity](snow-full-integrity.json). Tested normal package/manifest and
external updater command are retained in [accepted-build](accepted-build/).
Build 2026111502 uses the normal WD-launch binary with its CLUP identity stamped
forward from the bootstrap's 2026111402; no diagnostic application was deployed.

Physical download: 919.50 KB / 58 s / 15.85 KB/s. Staged readback: 2113 ticks
(35.22 s). Snow readback: 1644 ticks (27.40 s); Snow transport was an HFS fixture.
Independent helper requests a temporary 160 KiB partition; normal app remains
768 KiB. No new end-to-end peak-memory measurement was taken in this acceptance.

SEControl timed out during the physical helper handoff. User reopened SEControl
only; Clarice had relaunched successfully and was then verified remotely. This
qualifies the updater but leaves an interruption in remote observation: exact
handoff-to-ready latency was not measured. No further infrastructure work added.

For subsequent private-LAN delivery, retain the configured
`/etc/clarice/update.conf` URL and serve a newer compatible package/manifest via
`scripts/dev-http.py` on <MACBOOK_LAN_IP>:8080, restricted to <SE_LAN_IP>. Run
`clarice update --check` or `clarice update` in the active updated copy; the latter
requires explicit `yes`. `--source URL` overrides the configured source.
No wget/Finder/manual relaunch is required for subsequent updates. Preserve `.old`
recovery files; with Clarice quit they are native applications that Finder can
open. Rollback is manual recovery, not an additional automated feature.

Limitations: private numeric-IP HTTP only, CRC-32 corruption detection without
authentication, no HTTPS/public-source support, no atomic/power-loss guarantee.
Physical backup presence/sizes were checked; raw physical forks were not exported
for byte comparison. Snow extracted old/new fork contents and metadata passed.
The one-time binUnpk bootstrap omitted its bundle flag; the documented disposable
probe restored it. No MacBinary CLI feature was added.

The dated sections below retain the development evidence and superseded blockers.

## Stage 1: installed build baseline

Physical Macintosh SE, user-reported 4 MB/System 6.0.8/MultiFinder/BlueSCSI.
MiniPerformer+ accelerator installed; its software is disabled but hardware
acceleration state is **unverified**. Do not label these stock 8 MHz 68000 results.
Clarice reports CPU code 3 and System API 0607; banner says Plus/68000. Raw
reports preserved, not silently interpreted as a verified processor or OS version.

SEControl confirms Wi-Fi screenshots and foreground input remain functional.
Installed app reports launch-to-ready 387 ticks (~6.45 s), launch free app heap
429 KiB, idle 424 KiB. Screenshot round trips include scheduling/network delay,
not pure application latency. `/bin` listing and `serial --help` execute. Their
output can finish after the first capture: a QUEUED reply/frame is not completion.
No storage error observed in these read-only probes; not a storage endurance test.

Installed serial is older than Serial-1: advertised maximum 19200 baud, no Kermit
in /bin. At 19200 8N1 over the user-confirmed Freetronics Generic CDC adapter
/dev/cu.usbmodem1401 to modem port, `SE19200-abc123` was received and echoed
byte-for-byte on the host and appeared in Clarice. Command-period restored the
shell. `status runtime`: heap=433568, largest=428922, image=3068, ticks=2510,
load=29; owned-peak=0, remaining=0. Loader read=10, validate=9, imports=8,
relocations=1, prepare=1 ticks. This is the old serial image, not Serial-3.

Evidence: status.png, bin.png, help.png, serial-help.png, serial-open.png,
serial-echo.png, after-serial.png, serial-19200.jsonl. Remote key pacing is too
slow to count this as a sustained serial line-rate/typing benchmark.

## Bootstrap preparation

Normal current-checkout application compiled in isolated work/hardware-update-build
with nano performance diagnostics OFF. Existing work/build diagnostic cache and
all unpublished source preserved. Build log: bootstrap/build.log.
The user copied Clarice Dev beside the original app and launched it. The original
application is preserved. Focused Snow bootstrap passed; its pre-existing serial
backup correctly blocked redundant repair (not counted as a repair success).
On the physical SE, `appinstall repair serial` succeeded and retained
`/bin/.clx-serial.old`. New serial help confirms 57600 support.

Physical 57600 8N1/no-flow-control echo and reconnection passed: exact host RX/TX
`SE57600-abc123RECONNECT57600`. Ctrl+] then q and Command-period each restored
Clarice's prompt. Evidence: serial-57600-integrity.json, serial-57600.jsonl and
serial-57600-*.png. After cancellation runtime reported heap 424800, largest
424418, image 7232, load 40 ticks, owned-peak 0, remaining 0. Read 10, validate 5,
imports 9, relocations 0, prepare 4, events 3 ticks; receipt validation includes
record 12, metadata 5, structure 0, header 0, payload 0. These nested phases must
not be summed as independent totals. No sustained line-rate or leak endurance
claim; helper input pacing limits this short interactive test.

## Stage 2: downloader prototype

External wget prototype built separately (10,100-byte file; 6,884-byte loaded image). Native
MacTCP transport adapted from the attributed MIT Macinclaude code. No idle shell
network state. Uses existing HFS services, same-volume temporary file and rename,
refuses existing destinations. Numeric IPv4 HTTP/1.0 GET with Content-Length;
no DNS/TLS/chunking/redirects. 2 KiB stream buffer and temporary 16 KiB system TCP
receive ring. Failed TCP release reports a retained ring instead of freeing memory
still owned by the driver. No claim of power-loss-safe commit.

Focused ASan/UBSan host tests pass binary/fragmented response, zero length,
truncation, malformed lengths/headers, HTTP errors, existing-file protection,
write/close/rename/create failure, cancellation, timeout and cleanup recovery.
These mock host tests do not establish native HFS or MacTCP guest correctness.
Snow Plus/4 MB compatibility test passed: appinstall check/replace, help, and
missing MacTCP returned error -43 cleanly; runtime owned remainder 0, load 34
ticks, heap 395274. Evidence: snow-wget.png and snow-wget-script.txt. This Snow
fixture has no MacTCP, so no guest networking success is claimed. Physical
networking validation is pending Kermit delivery. Two direct Kermit attempts
failed without committing the destination. A wire trace confirmed negotiation,
filename and first data packet ACK, then silence on the second data packet.
Host byte-pacing experiment in progress; no cause asserted yet.
Read-only peer-restricted development server added at scripts/dev-http.py.

Stage 3 self-update has not been implemented or tested. No installed application
replacement has been authorised or performed.

### Kermit delivery defect isolated and corrected

Paced physical transfer negotiated and acknowledged many packets, then repeatedly
NAKed a valid LEN=94 packet. Both adapter collection and local engine validation
incorrectly capped the buffer at 94 including LEN; valid payload needs 95 bytes.
Existing P_PKTLEN+8 allocation suffices; no added resident memory. Both limits
now allow P_PKTLEN+1. Captured packet regression (ASan/UBSan) passes collection,
CRC, ACK and 52 decoded bytes. Exact wget image failed host transfer before and
passed afterward; standard focused host transfer/cancel/retry regressions pass.
Earlier fixtures did not exercise this boundary. Sources retain upstream credits.

A temporary host peer using smaller packets and 2 ms byte pacing delivered the
10,100-byte wget image to the old SE receiver in 36.69 s including handshake.
appinstall check then replace succeeded. The initial unpaced failure may also
involve burst reception: pacing advanced much farther, but its separate cause
is not established and no sustained-rate acceptance is claimed.

### Physical HTTP and integrity results

The SE downloaded the corrected 22,948-byte Kermit command using wget, passed
appinstall check and installed it with the old Kermit retained at
/bin/.clx-kermit.old. The original Clarice application is still untouched.
A 16,384-byte all-values binary download completed in 192 ticks (3.2 seconds,
including 45 loader ticks); runtime heap/largest 411028, owned remainder 0.
Existing destination refused (-48), HTTP missing file returned failure, and
success/failure shell statuses were 0/1/1. Subsequent error command returned the
same heap/largest 411028; not a system-heap endurance test.

The downloaded file was returned over Kermit and compared byte-for-byte:
http-binary-integrity-paced.json records exact=true, SHA-256
 a1f259d4365ed4320c377ce26f5c8c56dcdc9a89e7b641bfd8eabfbbeac86654.
Return took 21.36 seconds with host ACK bytes paced at 2 ms. Unpaced return failed
after several acknowledged packets. Pacing addresses this observed transport
limitation but does not establish its electrical/driver/accelerator root cause.
The host Kermit peer now exposes --pace-ms 2 (including ACK pacing); no firmware
or OS changes. Progress and detailed HTTP-status changes are undergoing Snow
validation; they are not yet running on the physical SE.

### Requested progress bars and HTTP status reporting

Added shared app_progress service: overwrite the current terminal row through
existing drawing, cap at current width, no input-ownership changes and no new
permanent allocation. wget/Kermit update no more than once per second and only
when stderr is a terminal. Known totals show percentage; Kermit receive uses an
indeterminate bar because this minimal protocol build does not exchange lengths.
Bytes/rate/elapsed are local status, never protocol data. Sender counts include
buffered bytes, so only the final success message establishes completion.
Runtime 1.1 declares the new import; 1.0 commands remain compatible. New tools
requiring progress are rejected by old runtimes instead of executing partially.

wget now reports HTTP status/reason (including 404 and 503), malformed status,
connection-close/truncation and input timeout distinctly. It still intentionally
requires HTTP 200 with Content-Length; no redirects/chunking/TLS added.

Snow Plus/4 MB/System 6.0.8 test used a test-only deterministic HTTP transport,
production parser and native HFS. It does NOT constitute a Snow MacTCP test.
Intermediate screenshot: snow-progress.png (45%, 7510 bytes, 2s); final and error
screens: snow-http-errors.png. Exact extracted 16 KiB data passes; no failed
404/503/truncated destinations or owned temporary files remain. Free heap/largest
394928/394330 after both success and failed download, tracked remainder zero.
Artificial pacing means its 375-tick download is not a network speed benchmark.
Host sanitizer tests cover progress pacing, redirected stderr, unknown lengths,
large-size arithmetic, actual HTTP errors and existing failure cases.

User requested a physical SE reset after fitting programmer switches. Physical
operations paused with no active transfer/replacement. Accelerator state must
be re-measured after reconnection, not compared silently with the earlier run.
Self-update is not implemented: a read-only System 6 open-file guard probe is now
being qualified before designing the native replacement step.

Runtime 1.1 focused installer/loader sanitizer regression passes, including legacy
requirements, accepting the current minor version and rejecting a future minor.
Existing independent-update, recovery, corruption and strict-verification checks
in the same focused harness remain passing (runtime-1.1-regression.txt).

### Physical SE after accelerator bypass

User reports Performer extensions disabled and interrupt/reset bypass of the
68030 board. Physical `status` now reports CPU code 1 (consistent with 68000),
System API 0607, free app heap 416 KiB / launch 420 KiB and 360 ticks to ready.
Clock frequency was not independently measured. Keep these results separate
from the earlier CPU-code-3 runs.

Wi-Fi GET of 16,384-byte binary: command 158 ticks (2.63 s), loader 44 ticks.
Loader subcounters: read 11, validation 6, imports 7, relocation 1, preparation 9,
events 2; counters overlap as documented. Heap 424808, largest 424416, tracked
remaining allocation zero. Evidence: post-reset-status.png, post-reset-http.png.
Returned that download through native Kermit using the new host --pace-ms 2:
exit 0, 18.63 s, exactly 16,384 bytes and matching SHA-256. See
post-reset-kermit-integrity.json and post-reset-transfer-complete.png.

Experimental host-only batched typing was not accepted: screenshots differ from
sent text. The one-at-a-time control also shows a missing character, so this is
not proven to be solely rate-dependent. No experimental test line was submitted;
Ctrl+C cleared them. User requested retaining one-character delivery and moving
on. Continue screenshot verification before submitting important commands.
The physical screen also reported Preferences save failed (-43), previous file
retained, during window movement. This is recorded, not diagnosed or dismissed.

Snow read-only updater guard: current Clarice reports busy=1, closed command
busy=0, nonexistent path error=-43/busy=1 (fail closed); guard-before.png.
The standalone after-exit probe has not yet produced its result file; Finder
launch automation is being corrected. No application replacement has occurred.

### Self-update foundation follow-up

Standalone native guard subsequently executed via Finder in Snow after Clarice
exited: guard-after.txt is `error=0 busy=0`. Earlier absent result was a launch
automation failure, not a guard result. Running/closed/missing-path guard cases
now have guest evidence. No physical application replacement performed.

The updater transaction core follows appinstall's sibling-stage/retained-backup
policy, refuses pre-existing backup files and performs no deletion. 482 focused
host ASan/UBSan scenarios pass, including every operation failing, failures
within rollback, busy files and invalid candidates. This is host evidence only;
native transaction integration and guest fault tests remain pending.
Strict MacBinary inspection plus unmodified sbase public-domain SHA-256 now pass
host tests against the normal application package and corrupt/truncated inputs.
No whole-package RAM allocation is required by the design.

User clarified the mandatory normal workflow: `clarice update` checks configured
source, verifies a newer compatible build, asks confirmation, stages, transfers
to an independent updater after cleanup, and automatically relaunches. Also
`--check` and explicit development-source override. Codex deployment must use
this same path; no normal manual wget/Finder/file manipulation. Private HTTP
only initially. Physical trials must use disposable application copies. The
native transfer-launch prerequisite is currently being tested in Snow.

Native transfer launch now has Snow evidence: LaunchProbe resolved its own full
HFS path, transferred to GuardAfter and did not return; the target wrote its
successful result. A second test selects the target directory and passes a leaf
name, avoiding the full-path display-name artifact. Evidence: launch-leaf-*.txt.

Implementation in progress: external /bin/clarice checks a configured private
HTTP source, supports --check/--source, rejects downgrades, streams/verifies a
MacBinary package, stages both forks, asks for explicit `yes`, and requests a
post-cleanup helper handoff. A native helper revalidates the candidate and target
identity, refuses open applications, commits with retained previous version and
transfers back to Clarice. These compile but the combined workflow is NOT yet
accepted. Runtime 1.2 declares the handoff import; older runtimes reject it.
The physical bootstrap currently lacks this hook and update identity.

Focused host tests also cover strict manifests, update-plan corruption and
cross-volume/path rejection, both-fork extraction failures and bounded HTTP
responses. Current integration tests use a test-only HFS-backed HTTP fixture in
Snow with the production parser and native filesystem. They do not establish
Snow MacTCP networking. Initial physical update trials remain pending and will
use disposable application copies only.

First integrated Snow `update --check --source ...` passes: running build
2026111402, source fixture 2026111403, normal prompt restored. Native resource
identity lookup leaves the running application's resource file open. Cold test
command: 283 ticks total, 210 loader; image 84,768 bytes; heap/largest after exit
393552/392784, tracked remainder zero. Full-check loader subcounters read24,
validate126, imports12, relocations2, prepare42, events12 (overlap as documented).
Evidence update-check.png and update-check-script-result.txt. HTTP is the test
transport reading local HFS fixtures; this is not a network benchmark.

## System 6 trap diagnosis and private-LAN integrity policy

The previous full disposable Snow update stopped at an **unimplemented trap**,
not a demonstrated cryptographic failure. Both original application forks were
extracted and remain byte-identical (update-crash-original-integrity.json).
The actual Plus/System 6 guest trap-table probe reports HOpenResFile A81A and
Unimplemented at the same address 00400806; OpenRFPerm A9C4 is available at
0041381a. Replacing the call with OpenRFPerm on the explicitly resolved HFS volume,
with resource preloading disabled while opening another application, reads the
CLUP identity successfully. Probe free heap stays 3250 bytes before/after.
Evidence: resource-probe.txt and update-unimplemented-trap.png.

HashBench executed in the same Plus/4 MB Snow configuration, normal video speed:
64 KB SHA-256 = 549 ticks (9.15 s), CRC-32 = 88 ticks (1.47 s), approximately
6.24x faster. These isolate computation, not disk I/O or total update time.
The incremental CRC result b11de6a1 matches the host known-vector test; SHA output
is also recorded in hash-bench.txt. Each CRC context is 1028 bytes of temporary
working storage, with no permanent table in Clarice's idle shell.

The private update manifest/plan now explicitly use version 2 and CRC-32.
The package CRC is computed during the existing fork extraction, including header
and padding, rather than a separate full-file pass. Both expected fork CRCs come
from source bytes before writes. Staged readback and the independent updater's
precommit readback remain. After rename, the helper checks catalog identity,
lengths, metadata and compatibility without another complete content scan.
CRC detects accidental corruption; it does **not** authenticate the HTTP server
or package. Public/internet updating remains future work requiring trusted
cryptographic verification. BlueSCSI offload is future optional infrastructure;
no firmware changes or assumption of an available coprocessor. A future common
crypto interface should allow software and optional hardware implementations.

Native disposable transaction fixture: successful replacement retains exact old
and new data/resource forks; injected second-rename failure returns -36 and rolls
back with both versions exact (native-txn.txt). Host CRC/manifest/plan, streaming
extraction and 482 transaction fault scenarios pass. Full integrated CRC workflow
is still under guest validation; do not infer relaunch acceptance from these probes.

UI transfer/status sizes now use B below 1024, then KB/MB/etc with two decimal
places; rates follow the same rule. Traditional Unix tool -h behavior is unchanged.

Follow-up full CRC run rejected a staged resource-fork mismatch (expected
f5ad9f4d, read 15058a9b), before confirmation/replacement. Original both forks
still match the pretest extraction. Staged verification took 1816/1828 ticks
(~30.3 s) including I/O/yields; this failed-run timing is not update acceptance.
A separate native probe hashes a disposable candidate before and after resource
identity inspection: both f5ad9f4d, with zero extracted byte differences.
Thus inspection itself has not been shown to change the fork. Further diagnosis
retains only the disposable failed stage under a test-only compile definition.
The production cleanup policy is unchanged. Evidence: resource-crc-probe.txt,
resource-crc-differences.txt, update-crc-mismatch.png.

The test-only HFS HTTP transport now supplies 2048-byte chunks matching the
production receive buffer, instead of an artificial 512-byte cap. Its 33-second
package transfer versus 91 seconds with the cap is a fixture difference, not
MacTCP or physical-network performance evidence.

Resolved the staged mismatch: the retained fork differs in 44 bytes, all within
16..127, containing the staged filename/type/creator/recovery fields. Every byte
outside this OS-reserved area is exact. Apple explicitly documents File Manager
scavenging writes and non-identical native copies in [More Macintosh Toolbox,
p. 1-53](https://developer.apple.com/library/archive/documentation/mac/pdf/MoreMacintoshToolbox.pdf#page=83).
The bounded staged-fork CRC now omits only these 112 bytes; the complete downloaded
MacBinary CRC still covers every byte. The resource structural header, bytes128
onward, all resources and map are checked; catalog Finder metadata is checked
separately. Resource data/map offsets must be >=256 and in bounds/nonoverlapping,
so ignored scratch cannot be treated as executable resource data. Exhaustive
host chunk-boundary/mutation tests confirm only the intended range is excluded.
No OS scratch bytes are rewritten by Clarice. See update-stage-differences.json,
update-stage-explanation.json and fork-crc-host.txt. Full workflow retest underway.

### 15 November: complete Snow update acceptance

Normal production application, Plus/4 MB/System 6.0.8: configured-source
`/mnt/sys6unix/ucheck.clx update` downloaded 941,568 bytes, verified/staged,
accepted explicit `yes`, exited, launched the independent helper from
System Folder:Clarice Data:etc:clarice, replaced the application and automatically
relaunched. Follow-up `--check` reports installed/source 2026111502 and no
newer version; pwd returns /. Previous build was 2026111402.
The existing ucheck fixture uses production HTTP parsing/extraction/update code
with HFS-backed transport, not real Snow MacTCP network evidence.

Staged readback was 1644 ticks (27.40 s); fixture transfer display 31 s is not a
Wi-Fi benchmark. Retained Sys6Unix:.cu-00000801-78c4020e.old has exact previous
data/resource content (excluding only documented OS resource scratch 16..127).
New forks match the candidate under the same documented comparison. Type/creator,
Finder flags and creation date match; plan consumed and no .new stage remains.
Evidence: snow-full-confirm.png, snow-full-relaunch.png, snow-full-integrity.json
and snow-{before,after,retained}-normal.bin.

The earlier launch stalls were confounded by timed automation selecting a retained
backup and host keystroke synthesis dropping punctuation/modifiers. Manual-paced
keycode input plus configured source avoided those issues. Diagnostic
handoff-trace.txt confirms cleanup completion and native Launch trap entry;
no additional production fix was needed in this resumed validation run.
Native launch uses PBOpenWD + SetVol with the actual FCB parent and a leaf name.
Physical acceptance follows next; no physical self-update success claimed yet.


### Physical confirmation reached; control reconnection pending

The existing binUnpk decoded bootstrap-wd.bin to Macintosh HD:Clarice Update,
but omitted its bundle flag. Its version stamp (2026111402), fork sizes
(0/941423), APPL/CLRC type/creator and closed-file guard passed a disposable
runtime-1.0 diagnostic. Restored only the package bundle flag and cleared Finder's
cached-initialisation flag; Finder Info then recognises/launches the application.
This is a one-time bootstrap correction, not an added production command or
MacBinary feature. Diagnostic source: bootstrap-check.c; evidence
se-bootstrap-bundle.png. Earlier probe builds were rejected for missing old-runtime
imports (EqualString, HOpenRF); the final probe uses the existing native service.

Real MacTCP HTTP --check against the explicit private source reports installed
2026111402 / available 2026111502. Created previously absent
/etc/clarice/update.conf with http://<MACBOOK_LAN_IP>:8080/update.txt.
The exact command `clarice update` downloaded 941568 bytes (919.50 KB), display
58 s / 15.85 KB/s. Both forks staged and readback passed at 2113 ticks (35.22 s).
Explicit yes was sent. Evidence: se-update-download.png, se-update-staging.png,
se-update-confirm.png, http-server.log.

The host SEControl heartbeat then exceeded its response deadline and disconnected.
This is not evidence of a guest crash or of successful relaunch. User asked to
reopen SEControl, leaving Clarice/error state untouched. Installed Clarice and
Clarice Dev were not replacement targets and remain recovery options. Need final
build check, responsive prompt and retained previous-target catalog verification.


### Physical final acceptance after reconnect

SEControl reconnection restored screenshots. Clarice was already running with a
fresh prompt; no manual Clarice launch occurred between confirmation and this
observation. `clarice update --check` reports installed/source 2026111502 and no
newer update; pwd returns /. `ls -l "/mnt/Macintosh HD"/.cu*` confirms retained
`.cu-00000a12-78c4020e.old`, data 0/resource 941423 bytes. The updater milestone is
complete; no additional feature/optimisation work was started. See concise result
and evidence at the top of this report. No push or publication.
