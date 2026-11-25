# Batch 4D — network performance and idle efficiency

Accepted normal 68000 build **2026111974**, product **f9b5f73**, local main;
compared with1973/27b8fbf.
Disposable Snow Plus,4 MiB,System6.0.8; no physical access or GitHub push.

## Retained changes

HTTP headers and FTP replies check events at refills and every256 buffered bytes,
instead of every byte. Empty foreground HTTP/FTP reads and idle Telnet iterations
use the existing one-tick cooperative wait. Available data and typed keys are
processed immediately. No transport ownership, protocol engine, remote queue,
buffer sizing, timeout or persistent-allocation changes.

## Matched evidence

Three repetitions per workload at Snow1x, with unchanged peers/data; maximum
acceleration only for functional installation. Host peers run in real time, so
accelerated guest ticks would distort network waits. Normal-client `time` includes
loading; separate instrumented client ticks exclude loading. Instrumentation adds
TickCount calls: do not present its larger gains as normal-build speedups.
Guest elapsed/yield time is not pure CPU time; native call counts measure avoided
work, not CPU utilisation. All raw results, including outliers, are retained.

| Workload | Baseline | Candidate |
|---|---:|---:|
| Normal HTTP1 KiB with4 KiB header, median total ticks |291|159|
| Header probe yield checks |4135|25|
| Header probe execution ticks, median |333|118|
| HTTP750ms idle: median TCPStatus calls |224|34|
| HTTP750ms idle: median normal total ticks |169|167|
| Normal HTTP64 KiB median total ticks |315|313|
| Normal FTP1 KiB warm total ticks (last two runs) |144,148|139,139|
| Normal FTP8 KiB median total ticks |156|154|
| Normal FTP64 KiB median total ticks |339|332|
| FTP1 KiB probe yield checks |275|35|
| Telnet delayed-close median TCPStatus calls |635|84|
| Telnet probe execution ticks, range |201–211|202–215|

Normal4 KiB-header transfer improves45%; ordinary throughput is broadly unchanged,
with modest FTP gains. HTTP8 KiB median121→123ticks is a small measured regression
within overlapping ranges (baseline116–122, candidate121–125), not hidden as a win. First invocation
of newly named probes/clients incurs receipt creation; do not compare that cold
cost with a previously invoked installed command. Warm1 KiB HTTP100,101→96,101ticks.
HTTP idle baseline includes a356tick outlier; other runs164/169, candidate164/172/167.

Cancellation-to-remote-prompt host times: baseline47–128ms, candidate54–153ms.
These include NAT, event polling and cleanup. Native pending operations retain
ownership through completion; no hard timing guarantee for arbitrary real drivers.
The host fixtures prove checks within256 buffered bytes and no sleeping while
bytes are available. Pauses service events/remote polling without reentrant commands.

## Remote editing and memory

Accepted4C service code remains unchanged. Ordinary character echo medians
152→154ms (max170→201ms) include host/NAT scheduling. Complete1000-byte mid-line
paste/Backspace/navigation/execute/prompt checks take3.72–3.79s before and3.82–3.87s
after, with exactly9058 received bytes each. This is not an isolated painting
benchmark. A separate echo-only check, after navigation drained, measures
1/30/300 inserted characters before a700-character suffix:1401/1430/3100 echo
bytes and0.497/0.457/0.984host seconds. The existing bounded batching avoids
per-character tail redraw for pasted runs, but still resends the suffix per batch. The existing six-tick shell idle wait and deferred send scheduling
remain relevant latency bounds; no queue redesign or idle-frequency increase here.

Observed system-heap peak delta:16392bytes for HTTP/Telnet,32784 for simultaneous
FTP control/data streams, with Telnetd already enabled. Every profiled call returns
the full delta. Existing Telnetd native allocation20700bytes is additional; a
concurrent FTP transfer therefore needs53484bytes of native storage in these
paths. Receive rings remain16 KiB: no evidence supports shrinking them across
Apple MacTCP/MoofTCP/OT. Six final repeated sessions report identical app
free/largest323204/321246 and system45602/45488; command allocations remaining0.

Files grow wget64, FTP220, Telnet44bytes; loaded images grow28/100/32bytes only
while invoked. No new BSS, permanent buffers or startup work. Core MacBinary+128
bytes from its embedded updater (+64bytes); Core CODE total remains285947bytes.

## Acceptance

ASan/UBSan existing HTTP/FTP/Telnet fault suites plus new polling-bound tests;
4B asynchronous native ownership/cancellation and4C remote input/queue regressions
pass. Includes fragmented/malformed headers/replies, timeout, ordinary EOF versus
TCP errors, allocation/disk/teardown failures, recovery and zero retained objects.

Normal guest binaries (byte-identical to the full sealed build) pass1/8/64 KiB
binary downloads, delayed/slow HTTP,30-second HTTP timeout, FTP upload/readback,
FTP cancellation during262144-byte download, remote Ctrl+C, local Command-period,
VT100 echo, Telnet Ctrl+C IP, terminal restoration and repeated heap checks.
Previous working distribution/recovery remains preserved. Physical stack timing,
throughput, native teardown and remote-client visual wrapping remain unaccepted.

The cloned fixture initially retained an old1951-era catalog after historical
direct test installs; update correctly refused it before activation. Existing
no-op adoption against the1973 repository reconciled the actual standard tools.
The1974 update then prepared recovery, replaced the three changed tools/Core and
relaunch succeeded. New embedded downloader no-op check passes; incomplete marker
absent. Readback verifies Core forks/Finder codes, client bytes and all49 standard
catalog entries against the sealed build. No updater changes were made.

HTTP/AFP verification covers51 entries/36 payloads, source/seal, offline tar/HDA
and unchanged optional service binaries; prior1973 index/payloads and recovery
are retained. Offline readback checks Core/Installer/Recover, both forks and all
49 utilities. See `verification.json`, `guest-readback.txt` and `offline.txt`.
Stop after4D.
