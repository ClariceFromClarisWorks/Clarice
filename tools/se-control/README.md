# SEControl graphical development helper

Separate from Clarice. System 6.0.8 / MultiFinder, 68000 Plus/SE, 4 MB,
Dynamic main-screen capture (up to 1024×768), delivered as monochrome.
PowerBook 180/System 7.1 is a new test target, not hardware-accepted yet. No extensions, trap/ROM patches, startup installation,
Clarice changes, file transfer or updater. New helper speaks HIL2; the original
one-shot `host.py` HIL1 entry point is retained only for the earlier proof.

Focused physical-SE acceptance passed: background Finder/Clarice capture, keys,
click/drag/menu, Clarice lifecycle, disconnect, manual mouse handback and
reconnection. See `outputs/hardware-control/graphical/REPORT.md` for actual
evidence and limits. This is not an unattended endurance qualification.

## Install / connect

Prepared media: `outputs/hardware-control/graphical/build/SE-Control.hda`.
Application-only disk (no System or ROM). Use a spare BlueSCSI image slot, never
replace the boot disk. Or decode `SEControl.bin` with a MacBinary-aware utility.
Quit the old helper before replacing it. Copy **SEControl** and **SE Control.cfg**
together into an ordinary folder. Existing config/token can be reused if its
host address is still correct. Do not copy emulator `HIL Script` / `HIL Selftest`
markers. The packaged config uses MacBook **<MACBOOK_LAN_IP>**, port 5968 and the
existing private token. SE **<SE_LAN_IP>**. No network-setting changes.

Set `SECONTROL_BIND` and `SECONTROL_PEER` to your private LAN addresses.
Tokens and guest config files are local-only and are not included in Git.
From this checkout on the MacBook, start the listener, then launch SEControl:

```sh
python3 tools/se-control/control.py serve \
  --bind "$SECONTROL_BIND" --peer "$SECONTROL_PEER" \
  --token-file work/se-control/token
```

Keep that process running. It uses a private local Unix socket for other commands,
accepts only the named peer and token, and maintains the connection with heartbeats.
The recovery build retries a dropped session after five guest seconds. The
listener accepts a new session; it never replays earlier input. An explicit
`quit` still exits; it does not reconnect. If macOS asks, allow this listener on the
trusted LAN. No public bind, router forwarding or firewall disabling.

## Host controls

```sh
python3 tools/se-control/control.py status
python3 tools/se-control/control.py shot /tmp/se-screen.png
python3 tools/se-control/control.py move 200 100
python3 tools/se-control/control.py click 200 100
python3 tools/se-control/control.py click 200 100 --count 2
python3 tools/se-control/control.py drag 200 100 300 150 --ticks 90
python3 tools/se-control/control.py type 'pwd' --enter
python3 tools/se-control/control.py key cmd+o
python3 tools/se-control/control.py key ctrl+c
python3 tools/se-control/control.py cancel
python3 tools/se-control/control.py quit
python3 tools/se-control/control.py stop
```

Capture a screenshot first to establish the connected screen dimensions.
Coordinates are actual Macintosh screen pixels, not scaled MacBook display coordinates.
Inspect a fresh screenshot before clicking. An exposed Finder title bar is a
reliable focus target; bare desktop clicks do not always activate Finder.
Select Clarice's icon, `key cmd+o`, wait for its prompt, then type. `key cmd+q`
targets the foreground app; `quit` targets SEControl regardless of foreground.
`shot` records capture time, snapshot heap and a frame checksum. Parent directory
must exist. PNG/PBM files are written only after a complete frame arrives.

`key` supports return, tab, escape, backspace, arrows and cmd/shift/option/ctrl
modifiers. Text is prevalidated US ASCII, max 1024 characters/request, paced at
no more than ten keys/second in the legacy single-key mode.
`type 'echo ready' --fast` uses eight-key bursts with one acknowledgement each,
no per-character sleep, and native event-queue backpressure. It requires the new
helper; old helpers fail clearly. Fast mode accepts only printable ASCII, never
Return, Tab or control shortcuts. Inspect a screenshot before `key return`.
BUSY is retried for at most ten seconds; partial/error responses are never replayed. No hardware GetKeys state, held modifiers or repeat.
`QUEUED` means posted, not accepted by the target app: verify the screenshot.
A partial-send error reports how many keys were queued; never blindly retry.
No automatic window recognition or background command execution in Clarice.

## Mouse lifetime and limits

MiniVNC-inspired transient MouseTemp/RawMouseLocation/CursorNew/MBState/MBTicks
writes. No patches. A 314-byte system-heap plan is prepared before execution;
a Time Manager callback advances it without heap, UI, filesystem, network or
application-global accesses. Registers are preserved. Its task is removed before
state/code can be freed. It is not scheduled when idle.

Clicks release in ~0.1 s; finite drags last 30–180 guest ticks (0.5–3 s).
The plan always ends in release, with a four-second overrun check if callbacks
are delayed. There is **no open-ended remote button-down command**. Host loss
cannot leave a normally running plan waiting for a release packet. If the entire
Mac freezes, this is not a real-time watchdog or crash recovery mechanism.
`cancel` requests early release when the main loop can service it; native menu/
drag tracking may postpone that request until the bounded plan has already ended.
Do not operate the local mouse simultaneously with a remote gesture. Hardware
control resumes afterward; final pointer position is retained, not rewound.

`drag --nowait` queues without waiting, useful before `cancel` or `disconnect`
for testing. Default waits for completion/release or an explicit error.
Screenshot acquisition or other commands may wait while foreground code refuses
to yield. Host deadlines distinguish a timeout from success, not a freeze from
network loss. The recovery build reconnects after connection loss, releasing its own mouse
action and discarding the unfinished command. This is not crash recovery.

## Footprint / network boundaries

256 KiB application partition (32 KiB more reserved than the previous helper),
16 KiB system TCP ring plus 314-byte mouse state and MacTCP's own overhead.
A snapshot exists only during capture inside the app partition: 21,888 bytes
on the SE, 32,000 on the PowerBook 180, 38,400 at 640×480. Colour QuickDraw
uses native CopyBits into a temporary monochrome bitmap, with a full-screen
clip independent of the background helper window. Low memory fails capture
explicitly. No claim of colour-fidelity screenshots.
No history/frame cache. FreeMem is current free heap, not total partition headroom.
Main loop sleeps six ticks; mouse timer runs ~60 times/sec only during a gesture.
CPU percentage and physical input latency require hardware measurement.

Plaintext trusted-LAN pairing, not encryption against hostile LAN participants.
Host private Unix socket is owner-only. Reads/sends retain five-second synchronous MacTCP timeouts;
connection attempts yield to Macintosh events through native asynchronous I/O; capture stops new chunks after 15 seconds. TCPStatus detects
peer closure after draining buffered data. Silent network loss can take longer
(up to the five-minute guest command-idle timeout); host heartbeat detects failure.
If TCPRelease fails, the receive ring is deliberately retained rather than freed
under a live driver. This exceptional condition/crashes can leak 16 KiB until
reboot. No claim of unattended crash safety.

## Build / focused tests

```sh
sh tools/se-control/build.sh
python3 -m unittest discover -s tools/se-control -p 'test_*.py' -v
```

Pinned offline Retro68 image; only SEControl builds. Output:
`work/se-control/build/SEControl.bin`. GPL-3.0-or-later helper; MiniVNC technique
credited in mouse.h and vendor/README.md. Macinclaude adapter retains its MIT
licence. Both licences and corresponding helper source accompany the package.

Snow fixture `work/se-control-graphical/plus.snoww`: disposable Plus/4 MB disk.
`HIL Script` executes the same command parser offline, with WAIT/SAVE and bounded
release checks. Output `HIL-script-result.txt` and PBM screenshots are extracted
only after normal helper exit and clean Snow shutdown. This validates native
input/capture and Time Manager behavior, **not MacTCP networking in Snow**.
Never mount a running emulator disk. Do not install fixture markers on the SE.

### Quiet performance measurements

`control.py quiet 120` suppresses automatic host heartbeat traffic for 120 seconds
(maximum 180, below the guest's five-minute idle limit). It sends nothing to the
SE. Explicit screenshot, typing and status requests still communicate; avoid
those during a timed workload. `quiet 0` resumes normal heartbeats immediately.
For long foreground workloads, start the listener with `--response-timeout 120`
(default 20; bounded 1–120 seconds). This changes only the MacBook listener,
not SEControl's resident memory or the guest's own network timeouts. The recovery build reconnects after a listener restart; no earlier command is
replayed. A genuinely crashed helper still needs independent relaunch.

### System 7.1 / PowerBook preparation

Use the same 68000 build; no CPU-specific binary or ROM patch is needed. Install
MacTCP and configure a working network interface separately. A PowerBook does
not inherit the SE's BlueSCSI networking configuration. Use a distinct peer IP;
stop the SE listener or run a second listener with a different port and --socket.
Keep the old helper as recovery. Initial testing is pending on System 7.1 and
the PowerBook LCD/trackball; Snow Plus and Mac II checks are separate evidence.

New protocol: CAPS reports keys=8 and screen dimensions; KEYS takes 1–8
hex-encoded (character,keycode,16-bit modifiers) records. Validate every record
before posting. BUSY posts none; ERR keys posted=N reports a partial native
posting failure. No keyboard queue is retained by the helper. The Macintosh's
existing EventQueue is inspected read-only through the toolchain's low-memory
accessor because its GetEvQHdr declaration has no linked implementation.

### Recovery development boundary

See [measured recovery checks](../../outputs/se-control-recovery/REPORT.md).
Only owned TCP streams are aborted/released; the shared .IPP driver is never
closed/unloaded. Failed release retains the existing system ring and prevents
allocating a replacement ring over it. A permanently wedged driver may prevent
safe completion. MacTCP is not an independently killable Unix service.

A serial maintenance endpoint inside Clarice is proposed for independent helper
relaunch. It is not implemented by this reconnect build. Existing Serial/Kermit
clients own their ports directly; an always-listening endpoint first needs an
explicit shared ownership/interlock so it cannot steal their receive buffer.
Generic process killing, OS reset, full shell service and startup hooks are not
provided. No recovery mechanism can run ordinary application code while the
cooperative Macintosh system is completely wedged.
