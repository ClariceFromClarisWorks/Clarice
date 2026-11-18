# Telnet findings from the xvi investigation

18 November 2026. Isolated Snow Plus/4 MiB/System 6.0.8, Core 2026111823,
optional telnetd 2026111814. No physical SE or other agents' guests touched.
This report concerns that existing build; it does not assess the ongoing rework.

## Source-confirmed integration gaps

- `cli_remote_wire(1)` is a usable binary ownership boundary but unconditionally
  writes `Kermit ready; start the peer transfer now.` It is not transfer-specific
  in the ABI, and xvi is another consumer. A neutral caller-controlled readiness
  message would avoid misleading editor users.
- It requires negotiated Telnet BINARY in both directions. Non-BINARY clients
  cannot enter it, even for a seven-bit ANSI editor. xvi reports that requirement;
  the test client explicitly accepts BINARY. Distinguish raw terminal mode from
  file-transfer mode in any future API; retain IAC escaping and clear ownership.
- No terminal dimensions/type are exposed to applications. Prototype xvi uses
  an explicit 80x24 ANSI/VT100-compatible contract. NAWS and TERM propagation,
  including resize events, are appropriate for the Telnet rework.
- The normal remote read callback is canonical, so a full-screen editor cannot
  simply use the existing `read(0)` path. Local `app_screen_open()` refuses remote
  sessions. Reusing the already implemented raw-byte API avoids changing Core.
- Repository update is deliberately local-only in shell dispatch. Installation
  used `clarice update` in this task's isolated Snow window; help, file checks,
  editing, status and regression commands use Telnet where possible.

Guest-observed results will be recorded alongside the final xvi test report.
No Telnet implementation changes are included in this branch.

## Guest-confirmed redirected-input defect

`xvi < /dev/null` unexpectedly enters the remote full-screen editor and waits
for keyboard input. Source cause: `src/port-files.inc:cli_isatty()` returns true
for every negative descriptor except -2; virtual streams use negative tokens
(-5 etc.). Thus `/dev/null` and `/dev/zero` are misclassified as ttys. This is a
shared runtime bug, not xvi's parser. The adapter checks both standard descriptors
before opening, but cannot distinguish these tokens through that API. Correct
isatty to recognize only actual terminal sentinels, with virtual/closed/native-file
and duplicated-descriptor regression tests. No Core patch is included here.

## Other source-confirmed raw-mode constraints to review

The existing raw receiver consumes byte 0x03 as transport cancellation rather
than passing it to the foreground application. An editor cannot choose its own
Ctrl-C semantics. File-transfer cancellation and raw terminal input need distinct
policies. Raw-mode exit with nonzero status requests connection closure; xvi uses
that fail-closed path after abnormal termination to prevent pending editor input
from becoming shell commands. A general terminal API should own mode restoration
and input disposal on all normal/error/disconnect paths, preserving that protection.

## Guest-confirmed control/disconnect behaviour

Editing over authenticated Telnet works: insert, Escape, search/change,
substitution, arrows, save/reopen and normal quit all passed. Disconnecting during
an unsaved edit unwound the invocation; reconnect, zero tracked remainder and a
subsequent saved edit passed. The unsaved nonexistent target stayed absent.

Sending Ctrl-C in raw editor mode closed the connection rather than returning to
vi normal mode. The first immediate reconnect then closed before the password
prompt. Follow-up recovery results are recorded below; distinguish a reconnect
race from a persistent listener failure. Preserve fail-closed disposal of pending
editor input while providing foreground-program control-character policy.

A subsequent authenticated connection succeeded without restarting Clarice or
its listener, and six further editor measurements were started. This establishes
recovery, not that the immediate reconnect race is fixed. The first EOF is retained
in `lifecycle-reconnect-failure.txt`; `lifecycle.json` retains prior successful
unplug/reconnect and the observed Ctrl-C close.

A separate connection attempt while local xvi waited for keys did not receive the
password prompt within the client's 20-second deadline. Local `app_screen_read`
waits in its event loop rather than the usual `app_yield` service-poll path. Command
serialization is intentional, but accepting/authenticating connections during a
local editor's idle wait is worth testing in the rework. This observation does not
establish a general listener failure; remote operation otherwise recovered.

## Production blocker: orderly hangup preservation

Upstream `events.c` handles `Ev_disconnected` / `Ev_terminate` by calling
`exPreserveAllBuffers()` before exit. The prototype's remote read/write error
currently takes `port_exit(1)`, so **that emergency preservation is bypassed**.
Normal save/preserve paths are retained and tested; this is specifically unsaved
buffer recovery on transport failure. The disconnect test proved that the original
absent target stayed absent and cleanup/reconnect worked, not that its unsaved
text was recovered.

Simply returning the upstream event is insufficient: invocation `port_poll` /
`yield_now` and native safe replacement observe the same failed/cancelled session
and may abort recovery I/O. Review a bounded orderly-termination/recovery contract
that allows required local recovery writes while stopping remote input/output,
and still permits genuine user abort/error handling. Do not clear global
cancellation state from the port or report successful preservation without a file.
This branch deliberately does not introduce such a runtime mechanism. Resolve
this blocker before a production vi claim or merge recommendation.
