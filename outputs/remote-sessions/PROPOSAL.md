# Remote sessions: implementation proposal and first protocol proof

Updater checkpoint: **0891878**, local `codex/coherent-deployment`; no push.
Remote investigation/test changes are separate. No physical-SE access.

## Reuse decision

Use Clarice's genuine **libtelnet** port for the Telnet server role; the existing
**mrsh** parser, Clarice executor and upstream **sbase/GNU/awk** commands remain the
command environment. Do not port a second shell or create a second Telnet parser.

Inspected [GNU inetutils telnetd](https://github.com/guillemj/inetutils/blob/master/telnetd/telnetd.c):
`startslave`, PTY packet mode/nonblocking `ioctl`, `select`, login/child lifecycle.
Those are Unix OS dependencies, not transport-independent library services. Porting
the complete daemon would require the Unix process/PTY subsystem excluded by the
brief. Reuse its separation of protocol/network, terminal session and command
execution, including backpressure; retain native Macintosh transport adapters.
GNU source was studied, not copied. It is GPL-3.0-or-later.

[sbase](https://core.suckless.org/sbase/) supplies portable Unix commands, not a
Telnet server/session implementation. Its utilities should run unchanged through
the existing runtime. Existing libtelnet is public-domain, with AUTHORS/COPYING
preserved; existing mrsh is MIT. No new production dependency introduced.

## Demonstrated boundaries that must change

- `shell.c` has global cwd, standard streams, status, environment and history;
  `sh.inc` additionally owns functions, arguments and execution-flow state.
  `sh_save` already isolates subshell variables/functions/cwd, but it is destructive
  restore, not a persistent session switch, and omits terminal/stream ownership.
- `streams.inc` sends terminal output directly to the local screen and reads the
  local keyboard. Interactive application screen services also target local UI.
  Merely calling `shell_run` from a TCP callback would hijack the local session.
- CLX executes synchronously and disposes its image at return. A service cannot
  register callbacks into a returned/unloaded CLX image. It needs explicit enabled-
  service ownership, separate from ordinary command lifetime, or a native helper.
- Current NetSend is synchronous with a five-second driver timeout. Reusing it
  unchanged in a background poll could freeze the UI. Native async parameter blocks
  and their buffers must outlive I/O and complete/abort before service release.
- Existing serial/E-Kermit opens the port exclusively and installs its own receive
  ring. Session transfers must use the already-owned transport, not reopen it.

## Smallest proposed implementation

1. One optional service, disabled by default, retaining its code only while enabled.
   Small Core hooks: enable/stop/poll, session dispatch, connection indication.
   Use the existing CLX validator/loader; do not alter its executable format.
   Ownership of its live code and outstanding I/O must be explicit and tested.
2. An independent shell context with cwd, environment/functions/arguments/status,
   history/editing and stream endpoints. Swap only at complete-command boundaries.
   Initially one command executes at a time; local editing/scrolling continues,
   and submitted local/remote commands wait their turn. No nested/reentrant CLX
   command execution from `app_yield`. True simultaneous commands require a larger
   execution architecture and remain a decision, not an implemented claim.
3. Native MacTCP passive open + asynchronous bounded send, and Serial Driver input/
   output using the same session. libtelnet handles only TCP framing/negotiation.
   Test transport teardown, cancellation and reconnect before Preferences wiring.
4. Disabled-by-default Preferences, connection indicator, salted non-plaintext
   password verifier from a vetted upstream password-KDF implementation, delay/
   limit repeated failures. Choose/calibrate the KDF separately; existing SHA-256
   alone is not a password-storage design. Telnet remains unencrypted.
5. E-Kermit over that transport last. Existing forced CRC-16 peer configuration is
   not yet proof of interoperability with ordinary clients. Test standard client
   negotiation; preserve full Mac files through MacBinary. No proprietary transfer.

Estimated enabled TCP working storage: existing 16 KiB driver receive ring,
roughly 0.6 KiB ordinary libtelnet state, 4–8 KiB bounded queues/editing, ~2 KiB
session base, plus demand-grown variables/functions and service code. Roughly
24–32 KiB fixed working storage, **not measured total**; libtelnet subnegotiation
can grow to its existing 16 KiB bound. Disabled service: no driver stream/ring or
session allocation. Physical measurements remain pending by instruction.

## First proof and next acceptance gates

`tests/remote-sessions/protocol.c` uses the unchanged engine in server and client
roles. One-byte fragmented delivery, 1,000 duplicate negotiations, both directions
of all 256 binary values including escaped IAC, Telnet IP, NAWS and 20 creation/
teardown repetitions. Host ASan/UBSan: **PASS**, two-endpoint allocator peak 712 B,
retained 0 B. This is protocol machinery, **not a remote shell or network test**.
The native 68000 test CLX is 10,272 B. **Snow Plus/4 MiB/System 6.0.8 at
maximum acceleration also passed all 20 repeats**; see [screen evidence](protocol-snow.png).
This is queued in-process protocol testing, not an actual inbound TCP session.
Rebuild only this fixture with `sh tests/remote-sessions/build-proof.sh`.

Before exposing a listener: isolated cwd/export/function/stdin/stdout tests;
local typing and scrollback while remote commands run; session-only interrupt;
failed allocation and disconnect restore the other context; no retained heap.
Then serial prompt, TCP prompt/authentication, Preferences/lifecycle, transfers.

Snow's current Ethernet NAT configuration/source exposes outbound translation,
not a configured host-to-guest listener mapping. Its available link modes include
raw bridge (and Linux TAP); no incoming reachability has been claimed. Prove a
supported isolated inbound path or guest-to-guest client before TCP acceptance;
do not silently expose a development listener on the physical LAN.

The two runtime decisions above (persistent session context and enabled service
code lifetime) are necessary prerequisites. The user approved these prerequisites and serialized execution;
no new executable architecture, worker process or retained-code cache is implied.


## Implementation status

The approved serialized-session approach is now implemented and tested over real
inbound Snow TCP connections. See [current acceptance and limitations](README.md).
The original five-step proposal above remains context; authentication/Preferences,
serial sessions and transfers are not declared implemented by this first batch.
