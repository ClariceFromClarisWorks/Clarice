# libtelnet port — Snow acceptance, 17 November 2026

Genuine libtelnet now owns Telnet framing, RFC1143 negotiation and subnegotiation.
Upstream c/h source is unchanged at commit
`5f5ecee776b9bdaa4e981e5f807079a9c79d633e`; source, AUTHORS, public-domain
COPYING and upstream fixtures are retained in src/commands/vendor/libtelnet.
No zlib. No new Core services or idle allocations. Native TCP/DNR and the VT100
renderer are unchanged (renderer SHA256 matches the recovery copy).

## Integration

- network/telnet-engine.c is the thin allocator/state-observation boundary.
  It supplies overflow-checked cli_alloc/free-backed malloc/calloc/realloc, and
  observes upstream state for partial-control timeout/EOF and negotiated options.
  It does not maintain a second protocol parser or negotiation state machine.
- telnet.c maps upstream events to the existing transport and screen, retaining
  CR LF screen behaviour, CR NUL padding removal, TTYPE, NAWS, local echo,
  escaped data, local escape, IP and cancellation. Library warnings/errors end
  the session with a stable diagnostic, avoiding retention of callback-local text.
- Every engine/buffer allocation is invocation-owned and released on all exits.
  Ordinary passive client option policy remains: ECHO/SGA, TTYPE/NAWS; BINARY
  refused. The upstream engine supports general negotiation, but this task does
  not add new client controls, a server, character-set conversion or urgent SYNCH.
- Upstream subnegotiation storage is bounded at 16 KiB, growing only on demand;
  realloc temporarily holds old and new blocks. Upstream auxiliary decoders can
  allocate additional bounded working storage proportional to that input.
  No retained capacity after telnet_free. 30-second partial-control deadline
  and native cancellation/timeout policy remain.

## Build, memory and matched timings

| Quantity | Preserved client | libtelnet client |
|---|---:|---:|
| CLX file | 17,924 B | 24,920 B |
| Loaded image | 12,612 B | 18,260 B |
| Snow loader, three runs | 48 / 53 / 48 ticks | 53 / 52 / 53 ticks |
| Complete short session | 163 / 164 / 163 ticks | 180 / 161 / 165 ticks |

These are native guest TickCount measurements with normal-speed Snow, not host
elapsed time or physical performance. Same Plus/4 MB/actual System 6.0.8 image,
MoofTCP/DaynaPORT, numeric address, VT100, and telnetlib3 auto-close fixture.
Both use full verification of the disposable executable. Sessions include the
fixture's one-second negotiation deadline, TCP and terminal setup/restore;
therefore the small total-time difference is not a CPU-speed conclusion.
Median load grows five ticks (~83 ms); median whole session two ticks (~33 ms).
Three runs are a bounded regression comparison, not a statistically strong
benchmark. Physical SE performance remains pending.

ASan host ordinary connected allocation peak for model + engine is 11,744 B
(host pointer sizes), excluding native driver/Core renderer. On m68k, the normal
four-option engine has 42 B state + 8 B option capacity + 512 B suboption capacity
+ three 4 B adapter headers = 574 B, excluding Memory Manager overhead. VT model
remains 11,116 B; Core's on-demand screen and native ring are unchanged. This is
component accounting, not a measured all-system peak.

Repeated Snow libtelnet runs: free heap400166/400168/400166, largest399480.
Recovery series: heap400924/400922/400924/400922; largest399480 then399844.
No progressive loss observed. Values differ with shell loop/file state; do not
subtract unlike contexts to infer leaks. Runtime owned-peak/remaining counters
are zero but do not measure these direct cli_alloc calls; host allocator tracking
and native post-run heap observations are the relevant evidence.

## Actual validation

Host: existing fragmented NVT/IAC, duplicate ECHO transitions, refusal, TTYPE/NAWS,
CR LF/CR NUL, IP, malformed/oversized/unfinished input, timeout, local escape,
connect/send/output failure, input overflow, repeated terminal restoration;
VT dirty-cell/Core parser comparison and 200,000-byte model fuzz remain passing.
Added deterministic failure at protocol/model allocations and every upstream
suboption growth allocation, subsequent successful execution, zero retained
allocations/bytes, and queued negotiation reversal. All 11 upstream fixtures run
through the actual adapter under ASan/UBSan, including RFC1143 and TTYPE.
Run `sh tests/telnet/run-host.sh`; evidence: port/host-tests.txt.

Snow: negotiated VT100, 80x25; real host restricted Bash prompt and pwd/echo;
Vim opened, edited and saved exactly `lib-port remote fixture\n`; less searched
line110 with inverse END indicator. Remote IP interrupted sleep30 and accepted
a following command. Command-period exited from less with130; reconnect and
remote exit returned0; Ctrl+] then q returned0. All three PTY children reaped.
Six matched auto-close sessions and an additional --dumb session returned0.
Malformed SB, >16 KiB SB and EOF inside negotiation returned1/1/1, followed by
clean recovery0. Captured screens, fixture logs and guest statistics are in port/.
The old greeting's System6.0.7 report remains a known detection issue; the fixture
uses previously verified 6.0.8 System resources. No physical result is claimed.

## Recovery and limits

Build: `sh scripts/build-telnet.sh`, artifact work/telnet/telnet.clx, SHA256
`a7784016ed0975b2cf929a8157ef50a5a869ecf15cbdc098bc648bfb01ab6fca`.
Working recovery source/renderer/binary preserved in work/telnet-review/baseline;
old CLX SHA256 `8b13c31f4cc25672fc3ec2f876e1411895480c5837441c626e84e187df72ce16`.
Guest /tmp/telnet-vt2 retained; new candidate only /tmp/telnet-lib. Installed
/bin, Core, FTP, the SE and separate0.5 RC were not modified. No push/release.
Snow and owned test servers stopped after clean app exit; temporary PTYs reaped.

No full xterm/VT100 certification, BINARY client mode, line discipline, TLS,
urgent SYNCH, dynamic window resizing or server. Existing renderer limitations
remain. Native cancellation can wait for MacTCP operations. libtelnet's upstream
repository is archived: pinned source and regression coverage are maintained here.
Before promotion, perform consolidated physical-SE negotiation/full-screen/input,
cancellation/reconnect and memory/performance acceptance when hardware is available.
