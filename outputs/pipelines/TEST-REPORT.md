# Everyday pipeline ports — first five

9 November 2026. Tested in Snow with Macintosh Plus ROM/model, 4,194,304
physical RAM bytes, System 6.0.8 and MultiFinder; no overclock or FPU.
These are emulator results, not real-hardware measurements.

## Delivered

Separate, PATH-resolved `/bin/wc`, `/bin/head`, `/bin/cut`, `/bin/tr` and
`/bin/uniq`, using pristine sbase sources at commit
`c546c3a5724c81cee9a11d816a38ccdf17472129`. Manuals, MIT author list and
per-file licence notices are retained. The staging script adds `--help` and
cooperative `tr` polling. The common runtime now routes `putchar` through the
command's stdout, essential for pipeline separators and newlines.

Packed disk sizes are 12,356 / 11,592 / 14,852 / 29,972 / 12,696 bytes,
respectively. The application partition remains 768 KiB. Each invocation has
96 KiB maximum tracked allocations (including its stream buffers), with a
16 KiB line limit for line-based tools. Code images, loader metadata and the
shell's shared stdio pool are additional memory. `wc` and `tr` stream without
retaining entire lines. There are 12 stream slots including standard streams.

## Validation

- 53 host checks: 26 upstream output/status comparisons, 5 additional file
  cases, 5 help cases, 5 cancellations, 5 write failures, 3 line bounds and 4
  invalid options. Port builds use AddressSanitizer and UndefinedBehaviorSanitizer;
  the harness checks allocation cap and zero retained owned memory on every exit.
- Full pinned offline Retro68 build with 68000 code generation passed.
  `build.log` and `build-sha256.txt` identify the tested artifacts.
- The Plus script verifies exact ordinary output, a four-stage
  `cat | cut | uniq | wc` pipeline, stdin/stdout redirection, help, invalid
  options (1), missing files (1) and an overlong line (2).
- Five repetitions of all five commands leave free heap at **431,800 bytes**
  and largest block at **431,112 bytes**, unchanged before/after. Last-command
  tracked peak is 2,466 bytes; remaining is zero. These are free-memory
  readings during the parsed test script, not total application footprint.
- Ctrl+C during `wc` on a 600,000-byte input restores the prompt and returns
  **130**, with zero retained invocation allocations. The host cancellation
  checks cover all five tools.
- Extracted `/tmp` listing has no `clarice-pipe-*` spool files after completion
  and interruption. Inputs/reports remain in `/tmp`; no test files were added
  to the shell root. Monaco 9 and the existing preferences are retained.

An initial combined run was interrupted while investigating its duration.
All isolated repeat checks passed; the second full run completed normally.
The sequence of many independently loaded commands is still noticeably slow:
its final `uniq` reports 62 ticks total, 45 loading (about 1.03/0.75 seconds
at 60 ticks per second). This is a tiny-input observation, not a throughput
benchmark. Loader/I/O optimisation remains future work; no speed improvement
or new flicker-free rendering guarantee is claimed by this tool-only change.

## Reproduction and limits

Run `tests/ports/build-pipelines-host.sh`, then
`python3 tests/ports/pipelines.py`. Generate inputs with
`python3 tests/ports/pipelines-fixtures.py`. Install the app and fixtures only
on the stopped disposable `work/system608/system.hda`; copy inputs plus
`tests/ports/pipelines-guest.sh` into `/tmp`. In the guest run
`sh /tmp/pipelines-test > /tmp/pipelines-result`, then the larger-input
Ctrl+C check. Quit before extracting evidence; validate using
`python3 tests/ports/validate-pipelines-guest.py`.

These are sbase options, not GNU option parity. `head` is line-count based,
`uniq` only combines adjacent lines and its `-s` skips bytes. UTF-8 processing
is not MacRoman screen conversion; UTF-8 comparisons were tested on the host,
not visually validated in the guest. Pipes remain sequential temporary-file
spools rather than concurrent Unix pipes. POSIX conformance is not claimed.

`tee`, disk-spilling `sort` and cancellable `tail -f` remain next. Earlier
awk integer-printf and serial validation gaps remain open; this report does
not certify those ports. No GitHub release was published by this update.
