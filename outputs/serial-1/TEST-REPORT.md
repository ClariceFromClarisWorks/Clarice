# Serial-1 validation

14 November 2026. Unpublished working-tree build; no subsequent roadmap work.

## Implementation and focused fixes

`src/commands/serial.c` reuses the native Serial Driver and existing terminal
output. `/dev/tty.modem` selects `.AIn/.AOut`; `/dev/tty.printer` selects
`.BIn/.BOut`. No general device filesystem was added. Default/requested 57600,
8N1, no flow control; also 4800/9600/19200/38400 and legacy aliases.

Bounded asynchronous transmit (64-byte batch), available-byte receive reads,
4096-byte invocation-owned receive buffer, five-second pending-write timeout,
receive-error detection, explicit usage/error/status handling. Ctrl+] then q
exits locally (0); Ctrl+C/Option+C/Command-period cancels (130). Option+] is
supported on the Plus keyboard. No VT100 or remote editing implementation.

`src/app.c` replaces the 32-byte drop-and-beep serial queue with ownership of the
existing 1024-byte typeahead storage. Overflow is latched, reported and requires
an explicit interrupt before shell input resumes. Session bytes never replay as
shell commands. Ordinary command cleanup preserves normal typeahead. The Plus
Option+] physical-key translation is normalised only during serial ownership.

Final validation corrected cleanup ordering: restore the receive buffer before
closing either half of the shared serial port. The earlier order produced
Mac error -17. KillIO is required only while an asynchronous request is pending;
there are no pending asynchronous receive requests. No completion callback is
installed. `src/module.inc` exports SerStatus; `src/sys6.h` documents the overflow
result. `scripts/build.sh` packages serial with the existing CLX runtime 1.0
contract. Its required import list causes older runtimes lacking SerStatus to
reject it rather than execute against an unsupported interface.

## Host results

- [Production client with mocked Device Manager](host-client.txt), ASan/UBSan:
  4096 ordered transmitted / 2048 received bytes with overlapping activity,
  both ports, escapes, 12 API failure positions, cancellation of a held request,
  transmit/probe timeouts, receive errors, short write, keyboard overflow,
  invalid path and non-terminal input/output. Checks require pending I/O cleared
  and receive buffer restored before close.
- [Actual queue helpers](host-queue.txt), ASan/UBSan: 10,240 ordered bytes over
  ring wraps, ownership transfer, unrelated typeahead preservation, overflow
  latch and no remote suffix replay.
- [Real Python/pyserial echo service over host PTYs](host-echo.txt): two sessions
  of 8192 exact bytes each (all byte values), disconnect error, reconnect and
  Ctrl+C shutdown. This is host transport evidence, not Macintosh hardware.

Run from repository root:

```
cc -std=c99 -Wall -Wextra -Wno-misleading-indentation -fsanitize=address,undefined tests/serial/client-host.c -o work/serial-1/client-host
work/serial-1/client-host
python3 tests/serial/queue-host.py
work/serial-1/venv/bin/python tests/serial/echo-host.py
```

## Normal Plus guest results

Existing keyboard-patched Snow 1.5.0, Macintosh Plus, 4 MB, System 6.0.8,
MultiFinder; disposable boot-disk clone and separate PTY bridges for A/B.
System API reports 0607 for this previously verified 6.0.8 installation; see
[existing OS identification](../system608/TEST-REPORT.md).

- Normal production build: all input/selection diagnostics OFF. Only the changed
  serial command and application were rebuilt; unrelated utility/editor products
  were retained. [Build](build-production.log).
- Original 315-byte bidirectional keyboard/echo test passed. Repeated on the final
  normal application at 20 ms requested key intervals: all 315 bytes arrived in
  order and were echoed byte-for-byte. [Exact comparisons](byte-checks.json),
  [wire log](echo-final.jsonl), [screen and local exit](guest-escape.png).
- Measured input rate: 40.31 bytes/sec over 7.814 host seconds (about 484 WPM at
  five bytes/word). This is automated input rate, **not maximum serial throughput
  or end-to-end keyboard latency**. Snow does not faithfully pace SCC output at
  the selected hardware baud; [raw measurement](throughput.txt).
- Local escape restored the prompt with status 0. Host echo process stopped and
  restarted; a new connection echoed all 23 bytes exactly. Ctrl+C restored the
  prompt with status 130 and subsequent `status` executed normally.
  [Screenshot](guest-cancel.png), [reconnect log](echo-reconnect.jsonl).
- Printer-port probe and subsequent modem probe both exchanged CLARICE-PING/PONG.
  Invalid device path returned 2. Test fixture iterations and final results below.

## Test fixture iterations

`tests/serial/cancel-guest.c` includes the production serial client and substitutes
only test input/yield and write/kill observers. Its first attempt found Snow had
already completed every write before cancellation; it correctly failed the
pending-request assertion. Its transmitted x characters contaminated the host
line responder's first PING, causing an expected timeout before the next clean
probe passed. [Attempt 1](guest-attempt1.txt), [screen](guest-probes-attempt1.png).
A second attempt set the native XOFF flag without enabling output flow control;
it still did not hold a request. [Attempt 2](guest-attempt2.txt). These are not
reported as passed pending-I/O tests. Test-only flow control is used to exercise
cancellation; the production command remains configured with no flow control.

## Final pending-I/O and recovery acceptance

The final test-only fixture enables native output flow control, then sets XOFF
(control 21) before submitting the real asynchronous write. All three cycles
observed `pending=1`, hold status 0, KillIO status 0 and request `ioResult=-27`
(abortErr), returning 130 with **zero free-heap delta** each time. Modem, printer
and modem-again probes then passed with status 0; invalid path returned 2.
[Guest screen](guest-acceptance.png), [extracted report](guest-results.txt).
No fixture or flow-control override is included in the production application.

The clean distribution HDA mounted at SCSI ID 6. `appinstall check` accepted its
serial.clx (status 0). Application/resource forks and serial command are checked
against the packaged originals by round-trip extraction. [Media verification](media-verification.txt).
`appinstall repair serial` installed the bundled new command, retained its previous
version and `serial --help` displayed the new interface; [installation screen](guest-install.png).

## Cost and hardware boundary

CLX loaded code/data/BSS: 7156 bytes, including the 4096-byte receive buffer;
loader relocation/import metadata and the small local stack frame are additional.
No new persistent input/cache allocation: the old 32-byte serial array is removed,
and one four-byte overflow latch is added. The application's existing 768 KiB
partition request is unchanged. No sustained physical SE line-rate or hardware
memory measurement is claimed.

No USB serial adapter was visible to this session. Cable wiring, real SCC timing,
USB driver behaviour and physical SE acceptance remain to be tested using the
[exact installation, connection and acceptance instructions](physical-se/READ-ME.txt).
No-flow-control cables do not reliably signal disconnection; silence remains
cancellable. Sustained receive traffic can overrun finite buffers; errors are
reported and terminate the session rather than silently continuing. The printer
port must be free of AppleTalk/other applications. Native driver access is not
an exclusive multi-application port broker.

Reference: Apple's [Serial Driver usage](https://dev.os9.ca/techpubs/mac/Devices/Devices-315.html)
and [PBKillIO contract](https://leopard-adc.pepas.com/documentation/mac/Devices/Devices-41.html).

## Deliverables

[Physical SE disk and instructions](physical-se/READ-ME.txt). Whole-device HDA
uses the existing Apple_Driver43 template with a newly formatted HFS partition;
no template filesystem blocks copied, no System/Finder/ROM distributed.
[Provenance](hda-provenance.json), [media listing](media-build.txt).
The existing safe-update policy remains: install the new application, then
explicitly `appinstall repair serial` to replace an already-installed command.
QOL-1/2/3A, Task 2 and unpublished editor work are preserved. No push or publication.
