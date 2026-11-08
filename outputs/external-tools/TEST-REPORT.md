# External utility milestone — Plus / 4 MB / System 6.0.8

## Delivered

ls, df, cat, benchls and benchscan are native CLX1 command files under /bin. Their implementations are compiled independently from Clarice; the shell no longer contains or dispatches their algorithms. Each file contains the same 9,364-byte multicall image, selected by command basename. The actual code/data image is 5,212 bytes; relocation and import metadata accounts for the rest. The loader allocates 9,332 bytes for the payload per invocation, plus ordinary call-stack work, then frees it. This is not a peak total-memory measurement.

The build embeds an installation resource. Launch creates missing /bin files and updates files bearing CLRC/CLX1 ownership metadata when the packaged header changes. Unrelated files are preserved. Execution reads the HFS /bin file and honours PATH, including a preceding script that shadows ls. cd and shell-state commands remain builtins. Other utility migration, including the existing literal grep builtin, remains follow-up work.

Standard streams support file input, output, append, independent stderr, left-to-right descriptor duplication and closing for descriptors 0–2. Bindings restore after each simple command. Files duplicated onto multiple descriptors share offsets. Terminal input uses a fixed 1 KiB canonical queue, with Return, Backspace, Ctrl+U, Ctrl+D and Ctrl+C handling; EOF and interruption were exercised on the Plus. Option+D/Option+C are the original-keyboard substitutes used by the emulator's Control mapping.

## Validation

- Existing pinned Retro68 Docker build passes, targeting 68000. Provenance hashes and build log accompany this report.
- Native guest scripts: **29 output/status checks pass**, including copied bytes, three-stage pipes, append, stderr separation, redirection order, closed input/output, restoration, PATH override and direct /bin execution, df -h, statuses 0/1/2/126/127/130, and repeated module execution with fresh static data.
- Deliberately corrupted module checksum rejected with 126; missing command returns 127. The probe runs twice with its static counter reset on each load.
- Host sanitizer checks: 31 shell comparison cases, three cancellation cases, and 11 stream/redirection cases pass. The terminal writer matches its executable reference across 1,500 randomized cases.
- Native pixel check: **24 bursts + 20 history views, zero mismatches**. Scrollbar staging: **153 checks, zero premature screen changes**.
- Interactive ls pauses for paging; Space advances; Ctrl+C exits to the prompt. Recorded status is 130.
- Terminal cat accepts a typed line. Ctrl+D at empty input returns to the prompt; Ctrl+C cancels a waiting reader. No shell prompt is displayed while cat waits for input.
- Ten sequential cat loads leave reported free heap unchanged at **121 KB**; launch free heap was **126 KB**. The MultiFinder application partition remains **384 KB**. Reporting is rounded to KB and does not establish a byte-exact leak audit or peak stack use.
- Root catalog after tests contains no Clarice temporary spool files.

## Performance and limits

Three 1,000-entry rendered listings took **812, 791 and 796 guest ticks** (60 Hz): median **13.27 seconds**. First-entry times were 17, 32 and 37 ticks. The current test used the taller 80-column window and four-pixel text inset. The earlier 12.05-second result used different geometry/alignment; this is not a controlled estimate of module overhead. Timing begins inside benchls, excluding PATH lookup, module loading and final prompt repaint. These are emulator results, not physical Plus measurements. Further rendering/alignment optimization remains warranted.

Pipelines remain **sequential disk-backed spools**, not concurrent Unix pipes. Finite pipelines preserve bytes/EOF, stderr separation, final-stage status and cleanup. Infinite producers, SIGPIPE semantics, job control, raw terminal/termios, descriptors beyond 0–2, general Unix executable formats, protected processes and a complete portable libc stdio layer remain unsupported. CLX1 commands execute in the application's address space and must cooperate with cancellation. This milestone does not establish POSIX conformance or complete upstream-tool compatibility.

Guest fixtures are tests/commands/io.sh and path.sh. Native probe source is tests/commands/probe.c; it is compiled with the same freestanding m68000 options and packed with scripts/pack-command.py. A copy with its last payload byte inverted supplies the badcommand checksum fixture. These two test modules are not production commands.
