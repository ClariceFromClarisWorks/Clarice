# Upstream grep/sed and quiet-command indicator

9 November 2026. Target: Snow Macintosh Plus, 4 MB, System 6.0.8, MultiFinder. These are emulator measurements, not real-hardware results. Application partition remains 384 KiB.

## Implemented

Separate `/bin/grep` and `/bin/sed` CLX1 images built from pinned, preserved sbase source with musl/TRE regex. No shell-dispatch wrappers and no GNU compatibility claim. The complete imported source, notices and staged adaptations are included; see THIRD_PARTY.md. PATH, standard streams, finite sequential HFS spool pipes, redirection and statuses use the existing shell integration.

The owned runtime ceiling is 96 KiB, allocated on demand, including tracking headers, 512-byte stream buffers and at most 8 KiB of reusable scratch blocks. All owned blocks are released on exit/error/cancel. Maximum twelve streams, including standard streams; maximum input line 16 KiB. Application fragmentation and free space can impose lower limits. These are cooperative in-process tools, not separately partitioned applications.

## Validation

- Host sanitizers: 26 grep option comparisons and 40 buffered-boundary comparisons against pristine sbase, plus explicit empty-pattern, word-boundary, memory/line-limit, cancel and write-failure cases.
- Host sanitizers: 19 sed behaviour comparisons against pristine sbase, including ranges, hold/pattern space, branches, substitutions, files, and errors; explicit infinite-loop cancel, line-limit and write-failure checks.
- Existing shell: 31 behaviour cases, three cancellation cases, eleven stream/redirection cases pass. Terminal writer: 1,500 randomized equivalence cases pass.
- Actual 68000 checksum implementation: 2,028 executable-reference cases pass, including odd addresses, large buffers, guards and preserved registers/stack.
- Guest grep: matches, BRE/ERE, no-match=1, malformed regex/missing file/closed stdout=2, quiet, redirection, `cat | grep | cat`, and ten repeated invocations pass. A 17 KiB input fails explicitly with allocation error; the 16 KiB ceiling is not a guarantee that every such line fits the available heap.
- Guest sed: substitution, `cat | sed | grep`, file writing, invalid program status=1, and five repeated invocations pass. A looping `:x;b x` program stops on interrupt with status 130.
- Twenty additional grep invocations retain exactly the same free heap and largest block before/after: 75,456 and 75,422 bytes in that scripted context. All reported owned remaining counts are zero. Script AST and other live allocations make this different from idle-terminal free space.
- Guest native scrolling regression: 24 bursts plus 20 history views, zero pixel mismatches; 153 scrollbar staging checks, zero premature screen changes (native-scroll.txt).
- Quiet-command overlay: visually checked at top right, cleared on completion and cancellation, suppressed by interactive ownership. See ACTIVITY-TEST.md.

Raw guest logs: grep-final.txt, sed-final.txt, perf-final.txt, sed-cancel.txt. Initial/earlier logs deliberately retain failures found during development; they are not passing evidence. In particular, the original loader/stdio fragmentation was fixed by freeing metadata before execution and preparing a bounded reusable FILE pool before loading images.

## Performance

Three runs per case, 1,000 lines / 16,000 bytes (`alpha beta gamma\n`), output redirected. Times include loading and are measured in guest 60 Hz ticks.

| Case | Earlier median | Optimized median | Change |
| --- | ---: | ---: | ---: |
| Fixed-string count | 295 ticks / 4.92 s | 201 ticks / 3.35 s | 32% lower |
| Regex count | 928 ticks / 15.47 s | 353 ticks / 5.88 s | 62% lower |
| Load portion | about 190 ticks / 3.17 s | about 95 ticks / 1.58 s | about half |

The retained optimizations are 68000 checksum shifts/adds and per-invocation reuse of short-lived regex allocations. Regex execution excluding loading fell from about 738 to 257 ticks. Both benchmark builds already used bulk buffered getline; the older 20,000-line byte-at-a-time result is not directly used for these percentages. The optimized image grew slightly; grep image was 26,680 bytes and sed 39,888 bytes before the final diagnostic-only errno-preservation fix. Timing is workload-specific, not a general throughput claim.

## Limits and next work

LF-delimited ASCII/C-locale text is the tested baseline. Native classic-Mac CR files are not automatically converted; resource forks are not text inputs. No full Unicode/locale, termios, concurrent pipes, signals, process protection, GNU options or POSIX-conformance claim. Cancellation depends on inserted yield points; not every arbitrary third-party C routine is bounded. Terminal-output-heavy workloads still need separate measurement from redirected scans.

One True Awk's pinned source compiled to a 68000 relocatable object of 90,775 bytes before runtime, libm and working memory. That is a feasibility probe only, not a runnable or installed awk. Its imports include subprocess, locale, seeking and floating-point services beyond this runtime. A separate MultiFinder worker partition and bounded stream/cancellation protocol is a plausible next design, but is not implemented. No silent enlargement of Clarice's partition or temporary-memory assumption was made.

Final diagnostic correction preserves the original errno across diagnostic printing; the Plus now reports a missing file as “No such file or directory” rather than “Success”. Final grep/sed image sizes are 26,816/40,024 bytes, with on-disk CLX sizes 38,012/57,288 bytes.
