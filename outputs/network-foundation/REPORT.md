# Native networking foundation — 17 November 2026

Implemented independently in `Clarice-Network`, branch
`codex/native-network-foundation`, based on 0686839. The accepted 0.5 checkout,
Core, standard distribution, physical SE and its network settings are untouched.
No commit, push or release. This is an experimental optional utility collection.

## Implementation

- `ipconfig`: read-only local IPv4 and subnet mask through public `.IPP` GetMyIPAddr.
- Shared command-local `network/native.c`: DNR hostname resolution and ICMP echo,
  with caller-owned callback state and safe completion before unloading.
- `wget`: system DNR names as well as numeric IPv4; existing streamed safe staging,
  HTTP response handling, progress and destination protection retained.
- `ping [-c COUNT] [-W SECONDS] HOST`: native ICMP, defaults four probes and a
  two-second timeout. Tick-based reply timings, meaningful exit statuses and help.
- No new Core import, resident service, sockets, DNS protocol, routing or Ethernet
  driver dependency. Public ABI declarations have 68000 compile-time checks.
  The updater build links the shared object because it already embeds wget;
  the installed updater was not rebuilt or replaced during this milestone.

`ipconfig` does not claim link state or report invented gateway, DNS-server,
DHCP-mode, stack-version or Ethernet-driver details. Those settings remain with
MacTCP/MoofTCP and their control panels. `.ENET0` is below the stack and is not
opened or reconfigured by these commands. See [research](RESEARCH.md).

Build with `scripts/build-network.sh`; normal 68000 CLX outputs in `work/network`:

| Command | CLX file bytes |
|---|---:|
| ipconfig | 1,756 |
| ping | 3,944 |
| wget | 19,328 |

They use existing runtime 1.1 imports. Test them by explicit paths or install
through the existing appinstall workflow; these experiments are deliberately not
added to the release candidate's standard collection. No Core rebuild is needed.
`--help` describes supported options and platform limitations.

## Actual acceptance

Host ASan/UBSan adapter, command and HTTP tests pass (`tests/network/run-host.sh`).
Cover immediate/deferred DNS, absent stack/resource, resolver failure and resource
exhaustion, cancellation/deadline drain ordering, asynchronous ICMP submission,
reply/timeout/cancellation, options/status/output errors, repeated cleanup, and
existing HTTP binary/framing/status/truncation/write/close/rename/error recovery.
Mocks verify ordering; they are not evidence of Apple stack interoperability.

Snow 1.5.0-a921b10, Plus/68000/4 MB, **reported System 6.0.7**, with unchanged
Clarice Core 2026111550. Published MoofTCP 1.0 and MoofPort 1.0 installed using
their native installers in a disposable cloned HFS image. Snow NAT has no DHCP
service in the inspected implementation; a static test-fixture configuration
(10.0.0.2/24, gateway 10.0.0.1) supplied via the published MoofTCP prefs schema
resolved the initial native `-23004` unconfigured error. Production code neither
reads private configuration nor changes it. Functional tests used fixed maximum
acceleration, except keyboard cancellation setup; not physical timing acceptance.

Passed guest checks:

- Missing stack: errors and subsequent prompt recovery, three repeated runs.
- Correct address/mask; two native ICMP gateway replies (7 and 5 guest ticks).
- Unanswered ICMP: native -23035 and status 1, subsequent commands work.
- Numeric and hostname HTTP downloads; **all seven 4,096-byte files match the
  host fixture byte-for-byte**. DNS uses installed DNR, not a host-side substitute.
- Repeated named downloads, NXDOMAIN (-23045), HTTP 404 and shell recovery.
- Five named downloads: free heap 391,290–391,292 bytes; largest free block
  unchanged at 391,182; tracked remaining allocations 0. No progressive decline
  in this sample. This does not measure the stack's system-zone peak allocation.
- Outstanding 60-second native ICMP request cancelled with Command-period:
  profile reports interruption only after callback drain, then `ipconfig` succeeds.
  Post-command tracked remaining 0, heap 403,268, largest 399,322. Earlier raw
  Control-key automation was ambiguous, so do not count that as a guest Ctrl+C pass.
- Final ipconfig/ping rebuild is byte-identical to the guest-tested binaries.

Evidence: [guest transcript](snow-results.txt), [binary integrity](integrity.txt),
[host tests](host.txt), [HTTP server](http.txt), [recovery screen](recovery.png).
Guest download duration 114–258 ticks includes CLX loading of 52–72 ticks and
asynchronous host networking. Accelerated TickCount/network clock interaction
makes these unsuitable for asserting SE throughput or wall-clock latency.
No permanent Clarice allocation is introduced by design; command-local DNR and
existing TCP buffers are released. System-owned stack memory is separate.

## Compatibility and limitations

| Stack / Ethernet driver | Evidence |
|---|---|
| MoofTCP 1.0 + MoofPort 1.0 | Original Snow functional pass above |
| Apple MacTCP “2.1” + DaynaPORT SCSI/Link | Follow-up Snow pass: status, ping, DNR/HTTP, cancellation/cleanup |
| Apple MacTCP “2.1” + MoofPort 1.0 | Follow-up Snow pass: status, ping, DNR/HTTP, ICMP cancellation/recovery |
| MoofTCP 1.0 + DaynaPORT SCSI/Link | Follow-up Snow pass: status, ping, DNR/HTTP, cancellation/cleanup |

Follow-up evidence and reproducible fixtures: [matrix results](matrix/README.md).
Apple “2.1” is the BlueSCSI-distributed patched 2.0.6, **not** an acceptance claim
for every unmodified Apple MacTCP release. DaynaPORT is from its 7.5.3 installer,
custom SCSI/Link component only. No production code changes or rebuild were
needed in this validation follow-up.

The guest System file's `vers` resources identify **6.0.8**, although SysEnvirons
and Clarice's greeting report 6.0.7. The earlier report's inferred 6.0.7 fixture
label is corrected by that direct file evidence; no OS/version-detection patch
was made. System 7.1 and physical hardware remain unvalidated for this milestone.
Snow NAT supports TCP/UDP forwarding; ICMP was tested to its gateway, not routed
Internet hosts. Ping is IPv4, bounded-count, no TTL/raw socket emulation.

Public DNR has no portable per-query abort. On timeout/cancellation we record
failure, keep servicing events and wait for its callback before CloseResolver and
unloading. ICMP likewise drains the native requested timeout (two seconds by
default). A broken stack that never completes can prolong this indefinitely.
This is a documented lifetime limitation, not a hard deadline guarantee. Adding
resident callback infrastructure solely to hide it is outside this small design.
DNS cancellation/deadline and allocation failure have host fault-injection
coverage; a deliberately stuck resolver was not injected in the guest.
HTTP retains its existing restrictions: HTTP/1.0-style GET, Content-Length,
no TLS, redirects, chunking, credentials or resume; data forks only.

## Smallest next clients — recommendation only

Recommendations only; implementation remains a separate milestone:

1. FTP: one foreground CLX utility over the existing native TCP adapter, two
   connections (control and passive data), streamed TYPE I RETR/STOR, reply parser,
   cancellation and existing same-volume receive staging. Reuse DNR and progress;
   preserve complete Macintosh files using MacBinary. Passive mode avoids adding
   listeners merely for file transfer. Reference [RFC 959](https://www.rfc-editor.org/rfc/rfc959).
2. Telnet client: reuse serial's terminal input ownership/restoration, with a small
   independently tested NVT/IAC negotiation layer over native TCP. Begin TERM=dumb,
   explicit escape and defined echo/CR handling. Study existing Macintosh Telnet
   source before implementation; do not transplant licensing-incompatible code.
   Reference [RFC 854](https://www.rfc-editor.org/rfc/rfc854).
3. Telnet server: a separate, explicit one-session foreground utility using native
   passive TCP open. A diagnostic terminal/echo service is small; exposing Clarice's
   shell requires a reviewed command-stream/input-ownership boundary, not a fake
   Unix login/PTY or background daemon. Do not equate the client with a shell server.

No FTP, Telnet or SSH implementation was begun. Neither protocol provides
transport encryption; scope any initial experiments to the trusted local network.
