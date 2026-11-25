# Batch 4B — cooperative TCP and native ownership

Accepted normal build2026111972, productdbaca7c; baseline1971/30d7b1e. Local main, host and disposable
Snow Plus/4MiB/System6.0.8 evidence only. No physical access or GitHub push.

## Correction

- Shared foreground connect/send/receive/close now submit asynchronous MacTCP
  PBs and use the existing one-tick event wait. PBs, WDS and caller buffers remain
  alive until original completion. Cancellation/timeouts request TCPAbort with a
  separate PB; failed abort cannot authorize unloading. Native status survives.
- Foreground release retries retain the existing command owner and16KiB system
  receive ring. Successful release frees it; repeated cleanup is harmless. The
  first close/release error is reported even after cleanup succeeds. FTP/wget/
  Telnet map native cancellation to130 and no longer claim a released ring is
  retained. Core's embedded updater shares the same downloader correction.
- Telnetd startup/listen or registration failure drains its existing owner before
  returning to the loader. Registered services keep their existing retry path;
  Seriald is unchanged. No orphan registry or new resident service was added.
- The existing runtime pause now performs one event wait even when cancellation
  is already set, preventing teardown retries from becoming a busy spin.

## Acceptance

`sh tests/network-4b/run-host.sh`: actual transport ASan/UBSan delayed async calls,
refused/failed submissions, cancellation of each peer-dependent operation,
receive/send/close timeouts, repeated failed abort/release, pre-registration
service failure, dual streams and20 cleanup cycles pass. Baseline negative control
fails on its synchronous open. Client status130/error/EOF and FTP/HTTP/libtelnet,
VT, service authentication/session regression suites pass; no tracked leaks.

Normal68000 Snow: shipped wget binary download, FTP binary PUT/GET with exact
comparison, empty/unknown-size EOF, ten repeated GET/compare/remove cycles,
Telnet client echo/close/reconnect, local Telnet/FTP Command-period130, remote
Ctrl+C/Telnet IP130, staged-file removal and subsequent command recovery pass.
Probe-labelled native tests retain real .IPP calls: three injected failed releases
then success; pending receive with three failed aborts; receive timeout and dual
live streams. Each finishes with system-heap delta0 and no outstanding owner.
Guest readback matches the final normal Core forks and client/service binaries.

A non-listening endpoint at1x returns wget timeout-23016/status1 in6.01 host seconds,
leaves no target and restores the prompt. At15x Snow's host NAT connect timeout
outlasts the accelerated Telnetd send deadline and loses the remote test output;
trace confirms native abort/release/return complete. This is not counted as a
production hang or as hardware acceptance. Timing-sensitive refusal/local-key
checks use1x; other functional checks use fixed maximum acceleration.

## Cost and limitations

- Repeated-run free/largest app heap324888/323058; system45622/45508 bytes,
  identical before/after ten transfers. Runtime remaining allocations0.
- Two simultaneous TCP rings consume32784 system bytes including Memory Manager
  overhead: existing32768 payload bytes plus16 overhead. All return on close.
- Native probe wait-service gaps:4–5 guest ticks in tested cases; timeout309ticks;
  three failed releases54ticks. Remote cancellation prompt recovery21–26host ms
  at accelerated Snow (not physical latency). No throughput speedup claim.
- CLX growth: wget+732, FTP+828, Telnet+688, optional Telnetd+84bytes. Core package
  +768bytes; CODE resources-2bytes, on-demand updater resource+732bytes. No new
  persistent state, allocation, startup work or receive buffer. Extra PB storage
  is temporary stack storage; full stack high-water was not measured.
- A permanently broken driver that never completes/releases keeps its existing
  owner alive and cleanup waiting. Returning/unloading would be unsafe. Application
  crashes/forced quit are not made recoverable by this correction.
- Physical Apple MacTCP/MoofTCP/Open Transport failure timing and teardown remain
  untested. Native error injection is explicitly distinguished from peer faults.

Reference: Apple's [MacTCP Programmer's Guide](https://mirrors.apple2.org.za/www.bitsavers.org/pdf/apple/mac/developer/Networking/MacTCP_Programmers_Guide_1989.pdf),
TCP asynchronous calls, Abort and Release; pinned MoofTCP
[PB ownership/completion](https://github.com/matthewmrichter/mooftcp/blob/a72e90ae8dd65e99a7be4981f15da85e1795e1c7/mac/glue/mt_pb_tcp.c).
No upstream protocol/renderer replacement. HTTP/AFP publication verified:
51 manifest entries/36 payloads, source/seal and complete offline tar/HDA.
Optional updated Telnetd is a separate versioned artefact; it is not added to
the standard managed collection. See verification.json. Prior1971 distribution,
recovery and unrelated work retained.
