# Real SE control — first proof

14 November 2026: first physical SE Wi-Fi capture/key proof **passed** at about
20:33 Adelaide time. SE <SE_LAN_IP> connected to MacBook <MACBOOK_LAN_IP> after the
owner updated the configuration. Source/build, host and Snow evidence below is
retained. Broader background control and deployment are not yet accepted.

## Physical Wi-Fi evidence

The authenticated session captured two complete 21,888-byte frames, posted one
`x` key, and received BYE after QUIT. The second screenshot visibly contains
`Last received character: x`, confirming delivery beyond the QUEUED response.
Evidence: [session JSON](physical-session.jsonl), [before](SE-before.png),
[after](SE-after.png). The host exited normally after the acknowledgement;
post-exit guest memory/cleanup was not measured.

- First capture request-to-complete: **1.198 s**; second: **1.086 s**.
- Guest allocation+screen-copy: **2 ticks (~33 ms)** each.
- Free application heap while each snapshot was allocated: **4,432 bytes**,
  identical for these two samples. This is limited headroom, not a whole-system
  memory figure or endurance/leak test.
- First screenshot caught the helper window before its content was painted;
  second is fully drawn. Both show actual guest framebuffer contents. A later
  automation client must wait for expected UI state rather than equate a frame
  response with UI readiness.
- OS/RAM/MultiFinder configuration is the user's specified SE/4 MB/6.0.8 setup;
  this protocol does not independently interrogate those settings.

Only foreground-helper keyboard input and two Wi-Fi captures are proven. Mouse,
background app targeting, Finder lifecycle, reconnection and long-term cleanup
remain pending. No Clarice production change or publication.

## Selection

| Candidate | Evidence and decision |
| --- | --- |
| [MiniVNC](https://github.com/marciot/mac-minivnc) | Genuine compact-68000 remote desktop support, but published compatibility table lists System 7 even for 0.9. Current source includes Gestalt/folder discovery and Script Manager keyboard translation; compact monochrome clients have encoding constraints. CodeWarrior-era C++/assembly and a broader protocol make this a larger System 6 qualification/port, not a ready 6.0.8 binary. GPL-3.0-or-later. No claim that System 6 is fundamentally impossible. |
| [Macinclaude](https://github.com/bdecrem/Macinclaude) | Explicit Plus/SE/System 6 and Retro68 target. Small shared C MacTCP client, framebuffer screenshot code and host receiver provide the best reusable pieces. Its screenshot service is part of a serial/Wi-Fi multiplexer, not a complete ready-made DaynaPORT remote-control server. Reuse the MacTCP client and screen-capture approach, avoiding the broader services/INIT. MIT, credited in source and package. |
| [AppleBridge](https://github.com/LoetLuemmel/AppleBridge) | Useful separation of host commands, guest execution and evidence. Relies on System 7 Apple Events/ToolServer and targets later systems; unsuitable as a drop-in System 6 control plane. Architectural reference only; no code copied. MIT. |

Pinned examined commits are in [upstream.json](upstream.json). Source copies used
for research are under work/hil-review; production code is untouched.

## Minimal implementation

Separate `tools/se-control` APPL and standard-library Python host. Helper connects
outward through existing .IPP/MacTCP to a configured private address. One bounded
frame response contains the true 512×342 1-bit screen; host creates PNG/PBM and
JSON timings. Key injection uses the documented native PPostEvent trap and queued
modifier field, without patches. [Apple API](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-66.html).
Commands are GRAB, KEY, PING and QUIT. No arbitrary remote execution/file access.
Host checks peer IP and token and enforces header, length and response deadlines.

The guest is manually launched and removable by quitting/deleting its folder.
No new Clarice dependency or persistent OS service. No full VNC implementation,
HTTP server, framework, firmware or network-driver changes.

## Actual evidence

- Normal pinned Retro68 68000 build succeeds: [build log](build.log).
- Five focused host test methods pass: fragmented frame delivery, truncated EOF
  with no output file, invalid header, response timeout, private-address policy
  and exact PNG pixel/CRC decoding. [Host results](host-tests.txt).
  These use sockets with synthetic peers, not the physical network.
- Snow Plus, 4 MB, 6.0.8/MultiFinder: native app copies screen, posts `x`, receives
  character 120 and redraws it, saves before/after screen data and exits to Finder.
  [Raw result](HIL-result.txt), [before](HIL-before.png), [after](HIL-after.png).
  Both PBMs are 21,899 bytes including header; pixel inversion/orientation was
  visually checked. Captures are guest framebuffer data, not host window grabs.
- Capture allocation+copy: **1 guest tick (~16.7 ms)**. Posted key to processed
  event/redraw: **3 ticks (~50 ms)**. Single samples, not network latency or
  typing-throughput claims. These earlier local figures exclude network transport; see the physical results above.
- Early test exposed a reversed register convention in the small trap glue;
  corrected from Apple's documentation, then the successful guest test above.
  No OS trap replacement. The final bounded-send change affects only the
  untested network path; capture/key primitives are unchanged from the passing run.
- Clean helper exit/volume flush and confirmed Snow process termination preceded
  offline extraction. A separate disposable copy was used; the prior loader
  acceptance Snow process/disk was left untouched.

## Memory and performance boundary

Final SIZE minimum/preferred is **192 KiB**, verified in the MacBinary resource
fork (not a claimed resident-byte measurement). Connected MacTCP adds a **16 KiB
system-heap receive ring**, plus unmeasured driver state. Snapshot is **21,888 bytes
temporary inside the app partition**, disposed after capture. No retained frame.
Initial 128 KiB trial left little measured heap headroom; 192 KiB was selected
for the proof rather than claiming a minimum safe partition.

Guest FreeMem sample was 3,830 before the first capture and 26,358 during result
writing after capture. Native heap expansion/purge and stdio make those samples
unsuitable as a total memory or leak measurement; do not subtract them and claim
negative allocation. Only snapshot disposal is established by the code, not
long-running network cleanup/endurance. TCPRelease failure intentionally retains
the system ring rather than allowing use-after-free; a failed release or crash
can leak until reboot. Physical connected/repeated-run memory still needs testing.

Synchronous MacTCP can pause UI during sends/open/close. Five-second per-operation
timeouts and an overall capture loop limit are configured, not measured on SE.
The helper is a bounded first proof, not a qualified background control daemon.

## Delivery and remaining gate

[Installation and exact host commands](../../tools/se-control/README.md).
Local build media: `build/SE-Control.hda` and `build/SEControl.bin`; config and MIT
notice included. Media uses the existing packaging tool's driver/map template
and clean HFS blocks, with no System/ROM or old user files copied. Pairing config
is installation-specific and excluded from version control/publication.

The initial physical bootstrap was pending at report creation and is now complete
within the narrow scope above. Snow supports DaynaPORT
but this fixture has no attached adapter/configured MacTCP/DaynaPORT installation;
therefore the offline native proof is explicitly separate from Wi-Fi acceptance.
A one-packet ping to the SE got no reply; this does not establish MacTCP failure
or absence because ICMP support is not assumed. No USB serial device is currently
enumerated, so automated serial bootstrap was unavailable. The owner subsequently installed the helper and completed the live test above.

Next: background input/mouse and
Finder launch/close/reopen, and timeout/reconnect/memory endurance. These are not
implemented or claimed passed. For subsequent Wi-Fi deployment, reuse reviewed
network transport and fork-preserving MacBinary staging. Macinclaude Bridge's
delete-existing-then-rename path is **not atomic** and does not meet Clarice's
recoverable-update policy; do not reuse it unchanged. Use a separate updater
after Clarice quits, preserve the previous version, and retain Task 2 command
compatibility/update rules. Serial Kermit remains a fallback; Kermit over TCP is
an option to assess, not a prerequisite for Wi-Fi file delivery.

No production Clarice changes, pushes or publication. Stop after the first physical capture/key proof; no broader control framework
or updater built.
