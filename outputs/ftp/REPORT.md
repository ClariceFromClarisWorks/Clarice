# Small native FTP client — 17 November 2026

Separate `Clarice-Network` development checkout; no RC/Core/distribution changes.
Build: `sh scripts/build-ftp.sh`; output `work/ftp/ftp.clx`, 19,056 bytes,
SHA-256 `1b29cb7f4ddedbd0dd7ab923368486b124653ec0ab5a30d6e3d83b2b3c0ee802`.
68000 CLX runtime 1.1, using existing imports, native `.IPP` transport and DNR.

## Interface and safety

```
ftp --help
ftp get HOST REMOTE NEW_LOCAL
ftp put HOST LOCAL REMOTE
ftp -P 2121 get HOST file.bin /tmp/file.bin
ftp -u USER --password-stdin get HOST REMOTE NEW_LOCAL < /path/to/password
```

Anonymous by default; custom login reads one password line from stdin, not argv.
This is plain FTP: credentials/data are unencrypted. Binary TYPE I, EPSV then
PASV fallback. Data connection always targets the control peer rather than a
third-party address advertised in PASV. No active mode, interactive session,
resume, recursive transfers, TLS or remote filesystem management.

GET refuses existing files, writes an owned same-directory HFS temporary file,
checks SIZE when available, requires final server success and local close before
rename. Failure/cancellation deletes only the owned temporary file; deletion
failure reports its recoverable native path. PUT uses standard STOR and can
replace remote contents; failure warns that the remote file may be partial and
never blindly deletes it. Ordinary transfers preserve data forks only; use
MacBinary for forks/Finder metadata. TCP/server completion is not cryptographic
integrity verification. Interrupted-process/power-loss temporary cleanup is not
claimed. Existing HFS resolver and native operations are reused.

Status 0 success, 1 failure, 2 usage, 130 cancellation. Progress uses the shared
throttled tty-only row; redirected output has a completion summary. Reply/data
idle timeout 30 guest seconds; native connect/send/close and resolver drain can
delay cancellation. Password limit511, username500, remote argument900 bytes;
reply line1024-byte storage bound and256-line multiline response guard. HFS/path
and signed31-bit transfer lengths remain platform constraints; oversized values
fail explicitly. No whole-file buffering or new resident Core state.

## Actual validation

- ASan/UBSan `sh tests/ftp/run-host.sh`: binary/empty GET/PUT, fragmented/multiline
  responses, EPSV/PASV and peer-address restriction, missing SIZE, truncation,
  failed final reply, existing target, stalled transfer, download/upload cancel,
  control/data allocation failures, create/write/close/rename failures, failed
  cleanup diagnostic and20 repeated resource-balanced invocations. A malformed
  EPSV suffix exposed a bounds-check gap during review; fixed with a length
  check and long malformed-response regression.
- Existing native-network and wget host regressions pass unchanged.
- Snow Plus/4 MB/System6.0.8, DaynaPORT: MoofTCP1.0 numeric hosts and Apple
  MacTCP BlueSCSI-patched“2.1” hostname lookup each passed three64KiB GET/PUT
  cycles, empty GET/PUT, missing remote (550/status1), existing local(status1).
  Three uploads per stack and extracted HFS downloads compared byte-for-byte
  with `bytes(range(256))*256`; Apple all three downloads extracted, Moof one
  extracted plus all three round-tripped through uploads. Evidence:
  `moof-dayna.txt`, `apple-dayna.txt`, `server.log`.
- MoofTCP slow PASV fixture: accelerated guest reached idle timeout against a
  wall-clock-throttled server; no destination/temporary file remained and empty
  reconnection succeeded. This demonstrates why accelerated ticks cannot be
  treated as real network throughput. Normal-speed rerun interrupted active
  download with Command-period: `cancel.txt`, `cancel.png`; profile returned to
  prompt. Subsequent three64KiB uploads succeeded, terminal progress worked and
  script reached FTP_DONE (`recovery-final.png`). `cleanup-files.txt` shows no
  `ftp-cancel` destination or `.ftp-*` files. Guest upload cancellation not run;
  host coverage only. Guest cancellation status130 not captured independently
  (startup profile itself stops on cancellation); host status130 assertion passes.
- All downloads/uploads here use a disposable pyftpdlib2.2.0 server. It and the
  temporary HTTP server were stopped; Snow was closed before HFS extraction,
  and test startup profiles reset to idle. SEControl listener left available.

## Memory and timing limits

Final command runtime image13,160 bytes; two existing16KiB native transport
receive buffers during transfer plus small bounded command buffers/stack. No
permanent shell allocation. `status runtime` counts application allocations,
not the driver's system-heap buffers: zero remaining is not proof of system-heap
stability. Host adapter asserts close balance; native release code reused.
No system-heap peak/free measurement was added.

Three repeated GET/PUT pairs: Moof free heap387,296/387,294/387,296, largest386,964;
Apple387,120/387,118/387,120, largest386,760. Initial-to-warm reduction ~1.4KiB
includes shell/loader warmup; not attributed solely to FTP. No progressive loss
in this sample. Normal-speed recovery uploads: heap388,148/388,150, largest388,096.
Accelerated upload ticks: Moof229–243 (load49–53), Apple319–430(load51–60);
network timing and DNS differ, so these are diagnostics, not a stack performance
comparison or physical-SE throughput claim. Physical SE acceptance is recorded below. System7.1 FTP, authenticated servers
and broader interoperability remain pending.

## Research and reproduction

Independent implementation, no BusyBox source copied. Protocol references:
[RFC959](https://www.rfc-editor.org/rfc/rfc959) and
[RFC2428](https://www.rfc-editor.org/rfc/rfc2428).
BusyBox contributors' [ftpgetput design](https://github.com/mirror/busybox/blob/master/networking/ftpgetput.c)
informed the small one-shot client scope (GPL2+ source studied, not transplanted).
Host fixture uses the upstream [pyftpdlib tutorial/API](https://pyftpdlib.readthedocs.io/en/latest/tutorial.html).

Install pyftpdlib2.2.0 in an isolated host environment; create a disposable root
with binary.dat and empty.dat; run `tests/ftp/server.py ROOT --bind HOST_IP`.
`--slow --pasv --port 2122` enables the cancellation/fallback fixture. It grants
anonymous writes only inside ROOT: use a trusted local interface and disposable
files. Copy the CLX and `tests/ftp/guest-accept.sh` to guest /tmp while Snow is
stopped, adjust host/unique output paths, then run the script. Never mount a live
Snow disk. Existing network matrix instructions supply stack/driver fixtures.

## Physical SE follow-up — 17 November 2026

Existing Core1550, stock68000/4MB/System6.0.8 reference setup, same installed
network configuration; optional FTP ran only as `/tmp/ftp-se`. No Core, `/bin`,
settings or RC replacement. Native stack version/driver pairing was not changed
or independently identified during this run; this is acceptance of the installed
SE configuration, not a physical four-stack matrix.

Five64KiB files downloaded and uploaded: all returned bytes match the host
fixture exactly (`physical/integrity.txt`). Empty GET/PUT, missing remote550 and
existing local refusal pass. An initial control intervention interrupted the
first script; its successful first download was preserved. The recorded rerun
therefore reports get-1:1 (existing) while get-2..5:0 and all put-1..5:0. This is
not five fresh GETs in the captured second script. Server evidence retains both
attempts; no destination was overwritten to make the test pass.

Repeated application heap387,038–387,040bytes, largest386,986 constant, remaining0.
Full upload invocations320–371guest ticks (5.33–6.18s), including CLX load51–69
ticks (0.85–1.15s). These native60Hz readings are actual SE diagnostics; server
STOR duration covers only data reception and is not total command duration.
SEControl heartbeats paused for part of the run; not a perfectly isolated
throughput benchmark. System-heap/driver-private allocations remain unmeasured.

Slow PASV download interrupted with Ctrl+C after ~71KiB of1MiB: status130,
destination absent, `/tmp/.ftp-*` unmatched, prompt restored, remaining0
(`physical/download-cleanup.png`). Slow upload interrupted with Command-period:
status130 and explicit remote-partial warning (`physical/upload-status.png`).
All64KiB had already been transmitted; the server retained a complete byte-exact
file. Cancellation is not remote rollback and cannot retract queued bytes.

A fresh64KiB GET/PUT after both cancellations returned0/0 and matched the
host bytes. Recovery script heap394,952/largest394,784 before and after,
remaining0 (`physical/recovery.txt`). No demonstrated FTP production fix was
needed. Temporary servers stopped; owned test files remain under guest/tmp
for review, SEControl left connected, and no push/publication occurred.
