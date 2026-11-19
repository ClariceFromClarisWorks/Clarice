# BusyBox vi Telnet observations

Test service: unchanged Core 2026111823/telnetd 2026111814, own loopback
23241, 80x24 with Telnet BINARY in both directions. This is the older fixture
used for the vi/xvi comparison, not acceptance of the concurrent Telnet rework.

- Existing generic raw-session entry banner still says “Kermit ready; start the
  peer transfer now.” It appears when starting bbvi as it did for vi/xvi.
- Initial adapter bug: invalid options were checked after raw entry. Error exit
  requested raw abort and disconnected. Corrected by validating options before
  acquiring the terminal; errors should leave the authenticated shell usable.
- BusyBox upstream read_key uses a 50 ms escape-sequence deadline and discards
  unknown multi-byte sequences. An automated search/change/insert/Escape/:wq
  burst lost the trailing input despite a 150 ms host delay after sending Escape:
  the editor was still processing earlier input when that delay elapsed. Draining
  earlier output before Escape and waiting for its timeout fixes the harness.
  This is an editor/automation timing issue, not evidence that Telnet dropped
  bytes. Keep Escape distinct from subsequent commands in automated tests.
- Adapter currently fixes remote dimensions at 80x24, like the comparison ports.
  It does not consume newer NAWS services. This is an explicit port gap.
- No emergency unsaved-buffer recovery. Fatal raw transport failure exits and
  releases invocation allocations; recovery/reconnect results are recorded in
  lifecycle.json: first reconnect succeeds, subsequent edit works, original file
  unchanged and tracked remainder zero. Do not describe this prototype as recovery-safe.

Harness-only failures: malformed Snow forward setting panicked its private NAT
thread; missing required Core index row produced updater error -50. Both were
corrected without Core/Telnet changes or touching another guest.
