# Physical Telnet findings — 18 November 2026

Update: the user approved the peer-policy change. Candidate1843 is implemented
and host/Snow tested; physical deployment awaits restored SE access. See
[approved fix report](../telnet-peer-fix/README.md). Pending-approval statements
below describe the historical test session.

Core2026111823 diagnostic, telnetd2026111814 (39280-byte CLX), stock reported
68000/4MB/System6.0.8. Native IP interface reports installed Macintosh TCP/IP,
192.168.1.252/24. MacBook source192.168.1.112; SEControl outbound/HTTP work.

## T1 — Enabled message but inbound connection refused (open)

Reproduction: telnetd --preferences; peer192.168.1.112, port2323, fresh disposable
24-character password, Enable checked, Apply. Terminal eventually reports
"telnetd enabled for the configured endpoint" and returns a prompt. Repeated
Python TCP connections from the configured MacBook address to SE:2323 return
ECONNREFUSED(61), including explicitly bound source192.168.1.112. No password
prompt/authentication/command acceptance yet. Swapped port4873 also refused;
not evidence of a particular cause. GUI helper and ordinary local commands work.

Do not conflate configured service registration with a reachable listener.
The fixed-peer/fixed-source-port diagnostic below connects successfully; the
normal fixed-peer/zero-source-port passive open remains incompatible on this SE. Current service automatically retries failed opens but does not expose
the driver error or retry state. This obscures diagnosis and merits a focused
status improvement, not a fake connected/success claim.

## T2 — Slow, silent settings apply (observed; attribution pending)

Initial55-second readiness wait expired. Later screenshot shows enabled/prompt,
but the port remained refused, so polling duration is NOT KDF execution timing.
A snapshot shortly after Apply had no prompt, later snapshot had one. Exact
Apply→completion and password derivation costs require a properly bracketed
measurement. Do not label the full listener wait as hashing time. No protection
or PBKDF round count was weakened. Existing service uses1000-round PBKDF2-SHA256.

## Diagnostic limitations

Host tcpdump cannot open BPF due to OS permissions; no packet capture collected.
No request to bypass that restriction. Successful Snow acceptance does not cover
this native network path. Remote full-screen editors, concurrent execution and
transport encryption remain documented feature limits, not newly found defects.

## Host regression fixture correction

The service-host binary-negotiation assertion initially failed because its text
feed helper used strlen on IAC DO BINARY (ff fd 00), truncating the required option
byte. Added a byte-counted feed for this packet. All existing ASan/UBSan remote
session, authentication, native service and serial host groups now pass. This was
a test defect, not evidence that the physical connection refusal is fixed.

Diagnostic optional telnet module adds bounded passive-submit/result/completion
logs only under SE_NETWORK_DIAGNOSTICS. Size39692B versus39280B (+412B), no new
normal-build allocation/import. Reuses existing native PB completion state;
maximum12 log records, no credentials. Physical execution results follow.

Physical diagnostic result: passive-submit=0, passive-result=1 at128380 guest
ticks; no completion/error callback observed before stopping. MacBook connection
still refused. Thus an accepted pending native passive open is confirmed, not
an observed async-error retry loop. Apple develop Spring1991 TCPPassiveOpen
reference describes specific remote host/port and zero wildcard endpoints:
https://preserve.mactech.com/articles/develop/issue_06/p46-69_Falkenburg_text_.html
No wildcard/peer-policy change made. The subsequent fixed-port comparison narrows this to native peer/port matching.
Normal module retained; exact MacTCP implementation cause remains unproven.

Shared authentication measurement: after physical USB unplug/replug and explicit
local service stop/start, serial Return→authenticated prompt took23.177s on the
SE (paced24-character password, no password echo). This includes prompt delivery,
password input and authentication, not an isolated KDF microbenchmark. It provides
a concrete usability cost for the common password path; TCP login remains blocked.

Exact-peer/port diagnostic: adding remotePort23230 while retaining the same
peer restriction gave a successful connection from192.168.1.112:23230 in1.037s,
with expected IAC negotiation and Password prompt. Native completion0 observed.
Same SE/destination2323; prior peer+port0 refused. This isolates a native partial
wildcard compatibility issue. Proposed wildcard listen + immediate existing peer
check requires user review because TCP handshake acceptance broadens before
rejection. Authentication/greeting remain behind the peer check. Approval pending.

## Combined1841 retest

Normal Core1841 and telnetd1841 (40784B) installed through the verified repository
update. Enabled message observed, then explicit-source192.168.1.112:63547 to
SE2323 still returns ECONNREFUSED61 (0.344s host elapsed). Evidence telnet1841.json.
Thus the new cursor-editing integration does not resolve the native listener issue.
The shared cursor handler DOES pass physical serial insertion (`AC`→`ABC` via
Left/Left/B), and remote help/local-input diagnostics pass. This is serial evidence,
not a claim of working ordinary Telnet access. No peer-policy approval received;
proposed fix remains unimplemented. Remote services are disabled at handback.
