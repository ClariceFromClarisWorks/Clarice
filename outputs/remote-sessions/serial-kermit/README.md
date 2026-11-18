# Supplementary serial shell and Kermit acceptance

18 November 2026. Snow Macintosh Plus / 4 MiB / System 6.0.8, fixed maximum
acceleration (~15×). No physical machine used. Core1812/runtime1.9,
telnetd1814/seriald1814 and Kermit1812 deployed through `clarice update`.

Enable **Preferences → Serial server** (or `seriald --preferences`), choose
`modem` or `printer`, 57600, set password, Enable, Apply. Saved verifier is in
`/etc/clarice/serial.conf`; bare `seriald` re-enables the saved configuration.
TCP settings remain independent. `remote off` stops both. Both disabled on restart.
The serial peer uses 57600 baud, 8N1, no flow control; press Return for login.
`exit` logs out. Only one authenticated remote shell is available across transports.
These are optional modules, not standard collection or Core startup dependencies.

## Interoperability

Genuine [E-Kermit 1.8](https://www.kermitproject.org/ek.html), existing BSD upstream
source and Macintosh adapters, is tested against independent upstream
[C-Kermit 9.0.302](https://www.kermitproject.org/ck90.html) (revised three-clause BSD).
Protocol engine unchanged. Native Macintosh Serial Driver and existing libtelnet
provide the transport. No Telnet framing is used on the serial cable.

In the remote shell, `kermit receive /tmp/new.bin` receives a new file;
`kermit send /tmp/new.bin` sends it. Wait for `Kermit ready` before starting the
peer. In C-Kermit select `SET FILE TYPE BINARY`, `SET BLOCK-CHECK FORCE-3`, and
`SET WINDOW 1`. TCP additionally requires Telnet binary mode in both directions.
After transfer, Return obtains a fresh prompt if the peer consumed the prior prompt
while draining its protocol input. Ctrl+C cancels; failed transfers close the login
so leftover packets cannot become shell commands. Log in again to recover.

Ordinary transfers preserve the data fork only. A complete Macintosh application
must first be placed in a fork-preserving container such as MacBinary. Acceptance
compares a 4,352-byte MacBinary fixture in both directions, including its resource
fork and Finder metadata bytes, but does not claim extraction into native forks.
Existing destination files are never overwritten by receive.

## Actual results

- `kermit-guest.py`: independent C-Kermit exact round trip over both transports.
  Host elapsed totals 4.38 s TCP / 3.98 s serial at accelerated Snow, including
  authentication and commands; these are **not physical throughput measurements**.
- `serial-guest.py`: login, variables, Ctrl+C/status130, three interrupted partial
  receives, no destination/temp remnants, logout/reconnect. Free heap/largest block
  exactly 293336/290326 each cycle; command remaining allocation zero.
- Existing interaction/lifecycle scripts: local typing/scrolling, queued local
  command, separate cwd, stdin, input overflow, repeats, TCP disconnect and recovery.
- Host tests: service/session allocation failures, delayed native I/O cancellation
  retains buffers until safe release, raw bytes and text-newline separation.
  Existing E-Kermit host suite: empty/all-bytes/repeat/boundary/checksum retry/cancel/
  recovery, all passed. JSON and host log alongside this report.

The first interoperability attempt used C-Kermit's default negotiated CRC and
correctly failed against our existing FORCE-3 configuration. A host-only C-Kermit
9.0.302 `zchko` crash on modern macOS is avoided by pre-creating its disposable
receive destination; no upstream or Clarice source was changed for that workaround.
Tests require a disposable password through `CLARICE_TEST_PASSWORD`; secrets,
C-Kermit build and raw packet logs remain in ignored `work/`.

One real guest defect was found and fixed: libtelnet stops newline conversion in
BINARY mode. Terminal text now explicitly remains CRLF while binary transfer bytes
bypass conversion. Tests verify both paths and repeated guest transfers.

Limitations: no reliable three-wire cable-unplug detection, no GUI editor on the
remote shell, no concurrent command execution, no encryption, and no physical SE
or System7 acceptance. The local native serial client/Kermit port-ownership guard
currently blocks either local port while seriald is enabled. Disable the service
before a local outbound serial command. Native I/O teardown failure retains the
service/storage for a safe retry rather than freeing pending asynchronous buffers.
