# Serial-3 — functional validation

14 November 2026. Normal 68000 build produced; no push, release or physical test.

## Result

Snow Macintosh Plus, 4 MB, System 6.0.8: bidirectional binary transfer, exact
round trips, cancellation/recovery, empty files, overwrite refusal and existing
interactive serial behaviour passed. [Verified bytes/status/memory](integrity-memory.json),
[peer timings](snow-results.json), [interactive regression](interactive-regression/result.json),
[final guest screen](guest-final.png). Physical SE acceptance remains pending.

## Changes and demonstrated fixes

- `/bin/kermit`, genuine E-Kermit 1.8 with separate bounded filename/parser and
  error-handling patches; upstream sources and BSD attribution retained.
- Binary-only send/receive, forced CRC-16, short packets, one file, explicit
  destination. Temporary sibling and no-overwrite commit; cancellation removes
  incomplete work. Existing native Serial Driver setup and cleanup pattern.
- Guest loader initially rejected `_impure_ptr` from libc diagnostics. Replaced
  those diagnostics with `vsnprintf` + existing `cli_write`; no runtime expansion.
  Protocol bytes use the serial device exclusively, never stdout or term_write.
- Native-to-host transfer initially delivered correct bytes but missed the final
  ACK after immediate host close. Host peer now drains output and allows 250 ms
  before restoring/closing the PTY. Both endpoints then returned success.
- No terminal redesign, editor changes or new permanent allocations.

Files: `src/commands/kermit.c`, `src/ports/ekermit/` (engine, adapters, upstream,
licence and adaptation notes), `src/command-install.inc`, `scripts/build-kermit*.sh`,
`scripts/build.sh`, `scripts/command-resource.py`, `tests/kermit/`, these reports.
Serial-1/2 and unpublished work are retained. [Adaptation details](../../src/ports/ekermit/README.md).

## Actual evidence

Host tests already completed before the final guest continuation, not rerun:
ASan/UBSan empty/4096 all-byte/4097 repeat/513 boundary files, injected checksum
error/retry, cancellation and recovery. [Results](host-tests.json).
The former filename overflow now returns X_ERROR under ASan; invalid-name and
failed-write checks pass ([bounds](bounds-fixed.log), [safety](safety.log)).

Guest fixture: 4,104 bytes, all byte values repeated plus NUL/CR/LF/FF/tail.
MacBook→guest and guest→MacBook pass, including retransmitting the received file
and the recovered file. Six byte comparisons pass, including a fresh received
file extracted after normal application exit. Empty file remains zero bytes.
Guest cancellation is 130; successful transfers 0; existing destination refusal 1.
A 128 KiB input was interrupted after two seconds; the peer failed after its
wait limit, the same guest destination was then received successfully, and no
`Clv kermit` temporary remained. The interrupted transfer did not complete.

Six captured Kermit runtime reports (normal, cancelled, recovered and empty):
heap **403,360 bytes**, largest block **398,876**, owned remaining **0**, each time.
Loaded image **17,552 bytes** (including invocation BSS). Initial startup baseline
was 404,662 bytes; this is not a claim that the shell's first-use allocations are
zero. Repeated Kermit runs showed no further retention. Final interactive session
screen: 403,554 free bytes, remaining 0, with no redirected-output stream open.

Recorded guest totals: normal send 312–313 ticks, recovery receive 521 ticks,
empty receive 115 ticks; loader 58–73 ticks. These include waiting and protocol
work, not pure transfer throughput. Host-clock fixtures include keyboard typing:
round trip 6.21 s, recovery receive 9.30 s, recovered send 6.42 s. No hardware
performance inference is made.

After transfers, the existing Snow shell fixture passed pwd/ls/echo/cat,
backspace, remote Ctrl+C, single echo, local disconnect and reconnection.

## Disk-inspection caveat

The first inspection began while Snow was still exiting, without first quitting
Clarice. That is not a valid persistence test. `recv.dat` listed correctly and
had already passed a serial round trip, but later offline open/restart access
failed; the final redirected metric was also missing. Do not attribute this to
a proven Kermit defect or claim crash durability. The affected extraction is
excluded. A fresh receive, normal Clarice quit (existing volume flush), confirmed
Snow process exit, then offline extraction **passed byte-for-byte** as `persist.dat`.
The recovered `cancel.dat` also extracted correctly. Never mount a live image.

## Build and usage

[Application media and install instructions](build/READ-ME.txt).
The normal production build is in `build/Clarice.bin`; separate `kermit.clx` and
`serial.clx` are supplied. HDA is clean application media, not a System boot disk.
Build logs: [command](build-kermit.log), [application](build-app.log).
Normal input/selection diagnostic modes remain off; no test command is bundled.

```
# Clarice, after exiting serial with Ctrl+] then q
kermit receive /tmp/new.bin
kermit send /tmp/new.bin
# macOS peer, built with scripts/build-kermit-host.sh
work/kermit/kermit -d /dev/ttysNNN send source.bin
work/kermit/kermit -d /dev/ttysNNN receive returned.bin
cmp source.bin returned.bin
```

Start the receiver first. No simultaneous terminal/transfer ownership. Reconnect
`serial -b 57600 /dev/tty.modem` explicitly afterward. Existing installed commands
are never silently replaced; use `appinstall repair kermit` when appropriate.

Limits: data forks only; complete Macintosh files require a prepacked MacBinary
container. No automatic packing/unpacking, resume, batches or server. Interoperability
validated with the supplied E-Kermit peer, not arbitrary Kermit clients. Both ends
require forced CRC-16. Final ACK loss may leave a complete file with overall failure;
never infer success merely from existence. Physical cable/SE tests remain unexecuted.

Stop after Serial-3. No later milestone begun.
