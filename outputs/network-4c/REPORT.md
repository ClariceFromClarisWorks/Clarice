# Batch 4C — remote service responsiveness

Normal build **2026111973**, product **27b8fbf**, local main. Baseline1972/dbaca7c.
Host and disposable Snow Plus/4 MiB/System6.0.8 only; no physical access or push.

## Corrections

- Canonical `read` and editor keyboard waits call the existing non-executing
  service poll. Existing sleeps remain six and one guest ticks respectively;
  no scheduler, extra wakeups or reentrant command execution. Services retain
  independent shell state. Disabled services perform no callbacks/allocations.
- Retain one 256-byte native receive batch while echo drains. Coalesce printable
  paste runs and redraw the tail once per run. History replacement fits the
  existing4 KiB queue even with doubled Telnet IAC bytes. Prompts wait for room.
  TCP's existing receive window supplies backpressure;1023-byte line limit and
  explicit oversized-line rejection remain. Upstream libtelnet is unchanged.
- Optional status callback reports listening/connected/retrying/native error.
  Original service registration ABI still works and honestly reports unavailable
  listener status for older services. Failed retries wait one second; live native
  owners survive failure. New optional services require the matching Core export.

## Acceptance

`sh tests/network-4c/run-host.sh`: ASan/UBSan actual input loops and service/session
implementation; old paste negative control; bounded stalled sends, five1000-byte
mid-line edits, cursor/Backspace/Home/End/history,1023-byte IAC history, oversized
rejection, authentication, allocation failure, queued-read ownership, delayed
native teardown, reconnect and cleanup. Registry tests cover legacy ABI, failed
listener/retry status and10000 disabled polls with no callbacks. Added export-table
ordering regression after Snow caught the initial misplaced new export.

Normal68000 Snow: canonical read and vi/nano idle permit connect/type/disconnect/
reconnect. Submitted remote commands remain deferred until local command/editor
completion; a filesystem sentinel proves no overlap. Local read Command-period
and nano cancellation return130, vi exits0, prompts and subsequent commands work.
Remote Ctrl+C returns130. Five1000-byte mid-line paste/Backspace/navigation commands
produce exact output;1100-byte input is rejected and recovers. Local status shows
listening, disabled, restart and connected correctly. A separately labelled guest
probe injects TCPCreate -36 for ten seconds: reports retrying with the native
error, accepts local input and automatically recovers. Normal service restored.

At1x, tested connection-to-prompt host times0.224–0.377s during local waits.
At maximum acceleration, five large edit/execute/response checks take0.452–0.528s
(mean0.488s); baseline disconnects on just256 inserted bytes before a700-byte
suffix. These include host/NAT/client overhead, not CPU time or SE latency.
Twelve matched connect/command/disconnect cycles: app free/largest323564/321126,
system45622/45508 bytes, identical throughout; remaining command allocations0.
Stopping Telnetd returns all20700 native system bytes; restarting succeeds.

## Cost and limits

Core package+384bytes; CODE+406bytes; two status pointers8bytes. Optional Telnetd
file+1584bytes, loaded image+888bytes, including276bytes of state; Seriald file
+1532bytes. No new system-heap allocation or larger native ring; old256-byte
receive stack buffer becomes the retained optional-service batch. No networking
startup/idle allocation when disabled. Shared Seriald paths pass host regression;
no physical serial validation. Existing password verification waits for the
execution slot; network negotiation/password input still progress while busy.
Long-line byte accuracy is verified; inherited NVT backspace/visual auto-wrap
behaviour across different remote terminal clients was not exhaustively tested.
Physical MacTCP/MoofTCP/OT timing and failure behaviour remain unaccepted.

Guest Core forks and Telnetd readback match the built candidate. Offline HDA Core,
49 utilities, Release, Installer/Recover forks and complete tar match the sealed
kit (only documented mutable resource reserved bytes16..127 excluded for native
copies). Publication verification covers51 entries/36 payloads, HTTP/AFP identity,
offline packages, source/seal and optional service binaries. Prior1972 distribution
and recovery retained. See `verification.json`; stop after4C.
