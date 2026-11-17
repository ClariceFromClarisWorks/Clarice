# Serial host and Snow checks

The PTY-backed shell runs **on the MacBook** and displays `MACBOOK> `.
Clarice's shell remains on the guest and displays its directory prompt.
Connecting outward does not let the host issue Clarice commands. Snow keyboard
input is the only guest-control mechanism used here.

The shell peer has no TCP listener. It accepts only an existing `/dev/ttys...`
character device owned by the current account, uses a temporary working directory,
TERM=dumb, no profiles, restricted bash and a small command PATH. Restricted bash
is **not a security sandbox**; run trusted fixtures only. The host account retains
its normal filesystem privileges. The temporary shell terminates on peer shutdown.
Queues are bounded to 4096 bytes per direction; logs go to disk. No credentials,
login service, automatic shell startup or network exposure is installed.

## Host-only checks (no emulator)

```
python3 tests/serial/shell-host.py
python3 scripts/serial-regress.py --fixture tests/serial/shell-fixture.json --output work/serial-host-run
python3 tests/serial/tty-host.py
python3 tests/serial/queue-host.py
```

The regression tool creates its own pseudo-terminal in host-only mode. `result.json`
labels it host-only; this is not evidence of guest execution. The same fixture
then exercises actual guest input in Snow mode.

## Snow session

Use the existing project Snow build, a **disposable** Plus/4 MB/System 6.0.8
workspace and its own boot disk. Never install into an image mounted by a running
emulator. For this task the stopped clone is `work/serial-2/system.hda`, workspace
`work/serial-2/plus.snoww`, with serial_bridges `["Pty", "Pty"]`. The app and
`/bin/serial` must both be the new build (app_serial_tty is a new required import).

Start the workspace using the existing emulator:

```
work/emulator/Sys6Snow.app/Contents/MacOS/Snow work/serial-2/plus.snoww > work/serial-2/snow.log 2>&1 &
```

Open Clarice in Finder and leave it idle at its own prompt. Use the **PID of this
Snow process** and the modem PTY path from its log. Do not guess a different
session's PID or PTY. Keep this Snow window foreground during input automation.

```
swiftc tests/serial/snow-keys.swift -o work/serial-2/keys
python3 scripts/serial-regress.py --fixture tests/serial/shell-fixture.json --output work/serial-snow-run --snow-pid PID --device /dev/ttysNNN
```

The harness starts the bounded shell peer, submits the connection command through
Snow keyboard events, sends Return to refresh the remote prompt, runs commands,
checks the exact forwarded bytes and expected remote output, disconnects locally,
reconnects and stops the peer. `result.json` records each check, elapsed times and
peer exit status. Timing includes automated typing; it is not display latency.
`wire.jsonl` records timestamped hex bytes to the shell and back to serial.
The helper's test-only byte 0x1c requests Command-period for failure cleanup.

A failed expectation returns 1 with a deadline; the harness terminates its owned
peer and submits local Command-period to the specified Snow PID. It does not type
shell commands as cleanup. Check the guest screenshot/prompt after an interrupted
automation run before restarting. A successful remote-output check does not by
itself prove correct screen rendering or local prompt restoration; inspect those
with a guest screenshot (as recorded in outputs/serial-2).

## Manual virtual connection, when useful

```
python3 scripts/serial-shell.py /dev/ttysNNN --log work/shell-wire.jsonl --seconds 600
```

In Clarice: `serial -b 57600 /dev/tty.modem`, then Return. Ctrl+C interrupts the
**remote** command. Ctrl+] then q exits locally; Option+] is the Plus equivalent.
Command-period cancels locally. Remote `exit` ends the host shell but cannot
signal physical carrier loss: disconnect locally afterward. Restart the host
peer for a new disposable shell. No physical hardware test is implied.
