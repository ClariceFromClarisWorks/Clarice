# Approved Telnet peer compatibility fix — 18 November 2026

The user approved the native wildcard listener with immediate allowed-peer
rejection, including the tradeoff that a disallowed peer may complete TCP before
being closed. This resolves the prior security-review gate; it does not claim
physical acceptance while the SE is offline.

## Change and cost

Normal telnetd now requests both remote MacTCP endpoint fields as zero. The
completed native open's remoteHost must equal the configured allowed peer before
any connected indicator, Telnet engine, option negotiation, greeting, password
processing or shell session. Existing stream abort/release/rearm and password
verification are retained. The historical fixed-peer/port diagnostic is still
opt-in; normal operation uses the approved wildcard path.

This uses documented TCPPassiveOpen behavior and the existing peer check instead
of adding a transport abstraction or another validation pass. The physical
baseline evidence was partial wildcard refusing TCP while exact host+port worked;
see ../se-performance-20261118/TELNET-ISSUES.md and the cited Apple develop MacTCP
reference. No new allocation, resident cache, import or Core startup work.

Optional module revision2026111843 is40824B, versus40784B (+40B), including the
help text explaining rejection after TCP. Serial remains byte-identical at30060B.
Core/runtime are unchanged; the module requires runtime1.9 or newer. Package
hash, source hash and approval record are in package.json. Existing development
filesystem1842, nano, serial/Kermit and recovery work remains preserved.

## Validation

- Host ASan/UBSan: all session/authentication/service/native serial groups pass.
  Native passive-open submission asserts both remote fields zero. Twelve rejected
  completions (password-protected and explicit no-password modes, including zero
  returned address) create no session or engine, send no bytes, perform no auth,
  show no connected indicator and rearm with stable ownership. An allowed peer
  then connects. Existing password rejection, cancellation and delayed I/O cleanup
  tests pass. Raw host.log.
- Retro68 68000 compilation passes; existing serial signedness/unused warnings
  remain. Normal serial image hash is unchanged. Raw build.log.
- Isolated Snow Plus/4MiB/System6.0.8, normal Core1842: two initial authenticated
  connections pass. Replacing only the disposable guest's allowed address with
  192.0.2.1 makes all three real loopback-forward attempts close with zero bytes,
  in0.084–0.201 host seconds. Password settings otherwise remain identical.
  Source NAT peer is10.0.0.1. These are emulator observations, not SE latency.
  Raw snow-allow.json and snow-deny.json. Restoring the original allowed address
  then passes three authenticated reconnects, each executing a real grep pipeline
  and reporting remaining0 (snow-recovery.json).

The wire harness requires actual EOF/reset and zero bytes; a timeout cannot pass
rejection. Existing Snow NAT forwarding is loopback-only. Original emulator disks
are untouched; this test uses copies under work/telnet-peer-fix/guest. Private
password/verifier material stays in ignored work/ and is not included in reports.

## Physical deployment pending

SEControl5969 reports “SE not connected”; macOS exposes no USB serial adapter.
The user has been asked to restore the connection. No SE update or physical
compatibility/rejection pass is claimed. The private candidate is staged at:

    http://192.168.1.112:8086/telnet-peer-fix/index.txt

Once access is restored, stop remote services locally, update with `clarice
update -y --source` using that URL, then test allowed authentication and rejected
peers before greeting/authentication, followed by allowed reconnect. Preserve the
original saved configuration byte-for-byte if temporarily changing the allowed
address for the rejection test. Keep password protection enabled throughout.
The repository retains the prior Core/utility versions; updater comparisons
preserve any newer independent installations. No public release or push.

Physical native MacTCP acceptance is the next required step; Snow does not
reproduce or resolve the stock-SE compatibility question by itself.
