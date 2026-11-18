# Telnet findings from original vi comparison

Isolated Plus/4 MiB/System 6.0.8 Snow; Core 2026111823, telnetd 2026111814,
loopback 23240. No changes to Core or the concurrent Telnet rework.

- Existing binary-session API emits **“Kermit ready; start the peer transfer now.”**
  on vi startup, as it does for xvi. It needs consumer-neutral wording.
- Raw session requires BINARY negotiation and offers no terminal type, NAWS size
  or resize contract. Both comparisons deliberately use the same 80x24 ANSI
  contract. This is not evidence of negotiated terminal sizing.
- Build 1862 returned a fatal raw read error to upstream vi, which retries input
  errors. After the failed screen assertion closed the socket, repeated reconnects
  got EOF before the password prompt. Command-period failed to unwind the loop;
  only this test guest was reset. This was a **vi adapter defect**, not proof of
  a Telnet server defect. Build 1864 terminates the invocation on fatal raw read/
  write errors, matching the xvi prototype. Lifecycle validation below is separate.
- Ending the invocation this way does not flush all unsaved editor changes to a
  recovery file. The runtime's cancellation/error state does not provide the
  bounded local-recovery phase discussed in [xvi findings](../xvi/TELNET-FINDINGS.md).
  This remains a production blocker; no weakening of cancellation or authentication.
- The existing `/dev/null` isatty defect and raw Ctrl-C semantics from the xvi
  investigation remain relevant; they were not changed or claimed fixed here.

Other porting findings (not Telnet bugs): CLX's relocation limit is reported as
“damaged or truncated”; HFS cannot seek past EOF or unlink an open file; the
minimal descriptor fstat has no write-mode information. These are documented
in [the comparison report](README.md).

## Final lifecycle result

Build 1864: the ANSI oracle observes the full 8 KiB screen and a visible UNSAVED
insertion before closing the socket. Invocation exits with `remaining=0`.
The first reconnect one second later authenticates. A subsequent vi invocation
succeeds and the original 8 KiB file still compares equal to its reference.
No recovery of the discarded unsaved insertion is claimed. See `lifecycle.json`.

An earlier check disconnected during initial rendering, before an inserted word
was observed. Its first reconnect received EOF before password; the second
attempt authenticated. This narrower startup/disconnect race is still worth
checking in the Telnet rework; it did not recur in the fully drawn-editor case.
See `startup-disconnect.json` for the earlier run's summary.
