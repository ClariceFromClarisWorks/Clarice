# SEControl bounded mouse backend — progress

14 November 2026. Optional development utility only. No Clarice production edits,
OS/ROM patches, firmware changes, VNC implementation, pushes or publication.
Physical Finder/Clarice graphical control, disconnect/reconnect and local
mouse handback have passed this focused acceptance session.

## Implementation

- Native background framebuffer capture and foreground posted keyboard events.
- MiniVNC-inspired transient mouse globals, bounded prebuilt click/drag plans.
  Time Manager callback has no application globals/A5 dependency, allocation,
  UI, filesystem or networking. Register-preserving assembly entry. Task removed
  before release/free; no idle periodic timer and no indefinite button latch.
- Persistent local host controller: screenshot, status, type/key, move/click/drag,
  cancel, guest quit, disconnect, daemon stop. Peer IP/token restriction and
  owner-only Unix command socket; no commands for remote filesystem access.
- MacTCP status recognizes a peer close after buffered input drains. Five-minute
  idle bound covers silent loss; host heartbeat/deadlines detect failed sessions.
- Standalone Apple/File menus; local offline fixture commands only when a
  `HIL Script` is deliberately installed. Fixture files excluded from deployment.
- MiniVNC GPL-3.0-or-later attribution/licence, Macinclaude MIT notices preserved;
  corresponding helper source accompanies local build media.

Changed files are confined to `tools/se-control/` and hardware-control reports,
fixtures/artifacts. `mouse.c`, `mouse.h`, `timer.S`, `control.py`, host command/session
checks are new; control.c/CMake/Control.r/vendor adapter/readme extend the proof.

## Executed evidence

Host: 12 focused tests pass (`host-tests.txt`): frame pixels/framing/truncation,
deadlines, address policy, prevalidation, key mappings, finite gesture completion,
reported errors, cancellation, no partial-input replay, rejected pairing, socket
disconnect/reconnect and daemon cleanup. Loopback tests are not guest MacTCP tests.

Snow: Macintosh Plus, 4 MB, System 6.0.8 MultiFinder, existing disposable disk,
normal speed. `snow-script-5.txt` and HIL-5 PNG/PBM captures demonstrate:

1. Helper title-bar drag by 30 pixels; subsequent release.
2. Clicking exposed Finder title bar activates Finder; capture contains Finder.
3. Clicking Clarice icon and posting Command-O launches genuine existing Clarice.
4. Background helper captures Clarice and posts `pwd` + Return. `HIL-5-pwd.png`
   visibly contains `/ > pwd`, `/` and the next prompt.
5. Command-Q closes Clarice; `HIL-5-closed.png` contains Finder. Click/Command-O
   reopens it (`HIL-5-reopen.png`), then Command-Q closes it again.
6. Mouse reports after gestures: active=0, down=0, error=0. Native input still
   operates after helper exit (`snow-normal-input-after.png`, host mouse into Snow).
   This is emulator input-path evidence, not a physical SE mouse test.

The `bg` suspend/resume diagnostic remained zero even when the screenshots and
foreground key delivery proved background operation; do not use it as a focus
oracle. Screenshot observation is authoritative. No OS hook added to infer focus.

Earlier runs are retained, not counted as acceptance: bare desktop click failed
as a focus target; an exposed horizontal scrollbar changed icon positions; fixture
files cluttered the Finder; a missing leading colon made relative HFS captures
fail. Final fixture uses the Finder title bar and `:HIL Results:`. A diagnostic
build error was corrected; failed-build output was not treated as native success.

Final native release stress (`snow-release.txt`) passed with the updated build:

- Mid-action cancel: started=1, held=1; released at elapsed=15 ticks, active=0,
  down=0, actual MBState=128, error=0.
- No main-loop/host service: started=1, held=1; finite plan released at 180 ticks,
  active=0, down=0, MBState=128, error=0.
- Exit during a held action: held=1 before cleanup; after RmvTime/release,
  active=0, down=0, MBState=128, MBTicks−Ticks=0 before freeing state.

These deliberately withhold main-loop servicing, proving packet-independent
release. Actual network disconnect is covered by host socket tests only until
physical testing. They do not claim a frozen CPU can recover. Updated helper
build succeeds; packaged binary matches it. The earlier successful screenshot/
Clarice lifecycle uses the same mouse backend; final additions are the local
stress fixture and peer-close detection, not new rendering or input algorithms.

Production SHA-256 comparison (`production-preserved.json`) confirms the
existing Clarice source files are unchanged. Physical measurements remain pending.

## Costs / limits

224 KiB application partition (original proof 192 KiB), plus existing 16 KiB
system network ring, 314-byte mouse-plan record and driver overhead. The extra
32 KiB is partition reserve; it is not measured additional FreeMem. Screenshots
allocate/free 21,888 bytes. No screen history/cache. In the completed lifecycle
run, free heap after successive captures was stable at 24,806 bytes. Different
stdio/heap-growth phases can show lower FreeMem; this is not a whole-system
memory measurement. Callback object disassembly has no A5 global references.

Main loop requests six-tick sleep; mouse timer ~16 ms only while active. No
measured CPU percentage yet. Normal click ~6 ticks held; drag 30–180 ticks,
four-second callback overrun check. Cancellation may wait behind foreground
tracking; finite automatic release does not depend on a subsequent host packet.
A fully frozen system cannot execute the timer. Physical SE pointer behavior and intentional TCP disconnect/reconnect passed
the session below. Unplugged network/long outages remain untested. No unattended recovery.

A three-second helper File-menu drag selected Quit in the focused Snow run;
shorter background Finder menu gestures did not demonstrate a selected item.
The physical Clarice menu subsequently passed with a three-second gesture.
This is not a blanket claim for every application/menu or shorter gestures. US ASCII keyboard events only; no GetKeys held-modifier simulation.

## Physical reproduction procedure

Install `build/SE-Control.hda` or decode `build/SEControl.bin`; keep config beside
helper. Start control.py listener, then launch helper. See tools/se-control/README.md
for exact commands. Capture Finder, click Clarice, Command-O, type pwd, capture,
Command-Q/reopen; test bounded drag/cancel, TCP loss and physical mouse handback.
Manual recovery remains available; actual physical results follow below.


## Physical SE session — executed

The user installed and launched the new helper. Authenticated HIL2 connection
from <SE_LAN_IP> to MacBook <MACBOOK_LAN_IP>. Actual screenshots and measured
capture results are recorded in physical-results.json and SE-*.png.

Passed by remote operations and visual inspection:
- Background Finder screenshot after clicking its exposed content.
- Clarice icon click + Command-O; keyboard `pwd` + Return yielded `/` and prompt.
- Three-second title-bar drag moved Clarice down 15 pixels.
- File-menu drag selected Cancel command: visible ^C and new prompt.
- Command-Q closed Clarice; Finder redrew; click/Command-O reopened Clarice.
- Cancel after a queued three-second drag returned active=0, down=0, error=0.
  This does not establish instant cancellation during native tracking.

Ordinary frame round trips: 0.997–2.750 s; framebuffer copy 1–3 ticks
(~17–50 ms). Requests immediately after launching Clarice took 10.838 and
11.170 s, including foreground startup/scheduling; these are not pure network
latencies. Idle heap observations 26,176/26,210 bytes; snapshots 4,280/4,314
bytes free. No downward trend over these captures; not a long endurance test.
Application partition remains 224 KiB plus 16 KiB TCP ring and 314-byte plan.
CPU percentage was not measured. No physical performance claim based on Snow.

An additional 180-tick Finder title drag was queued without waiting, then the
host socket was intentionally disconnected. The user confirmed helper exit and normal physical mouse operation.
After the actual manual relaunch, a second authenticated connection arrived.
A screenshot completed in 1.410 s; a posted x was visually confirmed in the
helper (1.187 s capture). Final status: 26,210 free heap, active=0, down=0,
error=0. The temporary initial lack of reconnection was because the helper
was not running, not a demonstrated networking defect.
There is deliberately no unattended restart mechanism.

No implementation fixes or rebuilds were needed during physical acceptance.
The listener and reconnected helper remain running for subsequent authorised
work. No automatic replay, firmware changes, production changes or publication.
