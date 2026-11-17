# Unattended serial development progress

14 November 2026. No physical SE testing, commits, pushes or releases.

## 1. Serial-2 — passed host and Snow checks

- Native serial now forwards Ctrl+C to the remote tty; Command-period cancels
  locally, Ctrl+] then q disconnects. Existing legacy serial import retains its
  prior behaviour. New `app_serial_tty` import makes old runtimes reject new
  clients instead of silently using incompatible tty semantics.
- Remote CR and BS handling reuse `term_write`; native shell CR/LF behaviour is
  unchanged. Backspace transmits DEL for the host's canonical line discipline.
- `scripts/serial-shell.py`: local user-owned Snow PTY only, disposable scratch
  directory, restricted bash, controlled PATH, no profiles, TERM=dumb, two bounded
  4096-byte queues, deadline and owned-child cleanup. Not a security sandbox.
- Actual host PTY tests pass pwd/ls/echo/cat, erase, remote interrupt, single echo,
  restricted cd, TERM and exit. Max host-only prompt response 3 ms; not guest
  end-to-end latency. [Raw host results](serial-2/host-shell-results.json).
- Snow Plus/4 MB/System 6.0.8: exact ordered serial bytes verified; remote commands,
  correction, Ctrl+C during sleep, local exit status 0 and reconnect passed.
  Free app heap 394 KB after each session (launch 400 KB); no new permanent
  terminal allocation. [Commands](serial-2/guest-commands.png),
  [reconnect/memory](serial-2/guest-reconnect.png),
  [wire check](serial-2/guest-wire-check.txt).
- Host peer shutdown remains locally cancellable (Command-period); remote shell
  EOF is not a serial hangup signal. No carrier-loss claim. Press Return after
  connecting to get a fresh prompt if an earlier one was sent before SerReset.
- Focused ASan/UBSan client, queue and CR/BS rendering tests pass. Normal 68000
  build passes. No unrelated suite or editor rebuild.

Next milestone completed below.

## 2. Reusable serial test infrastructure — implemented

`scripts/serial-regress.py` runs the same JSON fixture through an owned host PTY
or the existing Snow CGEvent helper. It starts/stops the shell peer, captures
hex traffic, verifies ordered input and expected output, records time/exit status,
and fails on deadlines or bounded-capture overflow. It explicitly distinguishes
host-only evidence, submitted guest input and observed serial output.
[Usage](../tests/serial/README.md), [first Snow run](serial-2/harness-snow/result.json),
[negative checks](serial-2/harness-negative.txt). No large framework or emulator
changes. Local prompt/rendering assertions still use separately inspected images.

Second Snow run passed: `serial-2/harness-snow-repeat/result.json`.

## 3. Serial-3 — host and Snow functional validation complete

Upstream overflow reproduced and bounded; actual E-Kermit send/receive integrated
as /bin/kermit. Normal 68000 build, exact binary round trips, cancellation and
recovery, stable repeated heap and interactive serial regression passed.
See [complete evidence and disk-inspection caveat](serial-3/TEST-REPORT.md).
Data forks only; MacBinary containers needed for complete Macintosh files.
No physical acceptance, publication or subsequent milestone.

## Terminal compatibility and hardware status

Only Serial-2-demonstrated CR/BS and remote Ctrl+C changes were made. No full VT
rewrite. [Pending hardware checks](PENDING-HARDWARE-TESTS.md) remain unexecuted.
