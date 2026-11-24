# Q1-001: native ICMP ping

24 November 2026. Build2026111947; product commit ab119e6 on local main. No physical deployment.

The user clarified that the old output contained ticks, not an empty RTT.
Source inspection and Snow baseline confirm genuine asynchronous native ICMP;
the defects were displaying native ticks, no interval and a four-probe default.
The existing stack owns ICMP IDs/sequences; Clarice does not implement raw IP.

Changes: integer millisecond conversion (~16.7ms resolution; `<17 ms` for a
zero-tick sample), one-second start spacing, continuous default, finite `-c`,
explicit timeouts/loss, cancellation130. Probes remain sequential: a long timeout
can lengthen the interval. Checks match returned payload cookies and reject
missing/short/wrong data or implausible timestamps. Apple MacTCP reports24 bytes
while pointing at our16-byte payload; MoofTCP reports16. Only the bounded16-byte
prefix is read. Native callback disassembly contains no imported/library calls.

## Actual validation

- Host ASan/UBSan: native async DNS/echo/error/cancellation lifetimes, cookie and
  timestamp failures, tick wrap, Apple extra length, short data, repeated calls;
  CLI options, millisecond formatting, interval, default cancellation after7
  probes, packet loss including large counters, existing HTTP regressions pass.
- Snow Plus4MiB/System6.0.8, MoofTCP/MoofPort: final binary replies, two timeouts,
  hostname success/failure, six repeated runs, indefinite mode beyond8 replies,
  Ctrl+C130 and successful next command. Local Ctrl+C and Command-period also
  passed with the preceding finite-default candidate (same cancellation path).
- Apple MacTCP patched2.1/MoofPort Snow fixture: replies, timeouts, DNS success
  and failure, four repeated runs; final continuous binary reached168 replies,
  Ctrl+C130, prompt and one-probe recovery. Extracted guest output retained.
-37 request/reply pairs in filtered ICMP capture match source/destination,
  identifier, sequence and exact payload; all ICMP checksums valid. Request
  cookie guest timestamps show58–62 ticks between successive probes (driver
  submission overhead varies). Host PCAP times are not guest RTT measurements.
  Capture predates the continuous-default change; native packet code unchanged.
- Final Moof heap333234/largest331266 unchanged across six runs and recovery;
  owned peak0/remaining0. Apple prior repeated batch alternates354702/354704
  with largest354650 unchanged; no progressive decline. Final recovery remaining0.
- Final ping4684 bytes versus3944 before: +740 bytes. No new permanent Core
  allocation; command-local cookie replaces existing16-byte request data,
  adds4-byte limit and4-byte command-lifetime sequence state. Loaded image3336.
  Two-probe Moof totals90–93 ticks include deliberate60-tick pacing; load23–25
  ticks. These are guest elapsed times, not CPU benchmarks or physical-SE costs.

## Limits

No new physical or Open Transport acceptance; prior four-driver matrix is reused
as historical evidence, not a claim that all combinations were rerun. Apple’s
MacTCP compatibility does not establish ICMP support under OT. Native ICMP has
no public abort used here: cancellation safely drains an outstanding callback
(up to the requested timeout; DNS may take longer). A broken stack that never
completes still cannot safely unload callback code. No networking redesign.

Publication uses the existing sealed build and immutable-payload/index-last
HTTP/AFP mechanism; all51 records/36 unique payloads match sealed hashes through HTTP and AFP
readback (verification.json). Source, offline kit and provenance also published. Previous
payloads and recovery indices remain. No Git push or public release.
