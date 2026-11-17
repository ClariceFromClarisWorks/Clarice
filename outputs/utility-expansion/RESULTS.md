# Utility expansion and help — completed through build 2026111549

## Second batch: actual results

Pinned, unmodified sbase sources/manuals add independent `/bin/dd`, `sleep`,
`printenv`, `mktemp`. Adaptations remain in the preparation script/runtime.
`env` and `time` are deliberately shell wrappers using existing scope and
execution services; an external implementation would require a new nested command
execution API. No resident code cache, new process model or filesystem layer.

- dd: checked decimal/b/k/x sizes before allocation/destructive open; output-block
  seek correction; native HFS zero-filled extension, including notrunc; correct
  32-bit record counts (initial guest exposed the intmax formatting mismatch).
  Runtime 1.5's resize import is linked only where needed. grep/sed and other
  older commands retain their prior runtime requirements.
- mktemp: native exclusive create, collision retries, caller-owned file; removes
  an unreported file after output failure. -u rejected, -d deferred. Predictable
  names do not confer Unix permissions or security guarantees on System 6.
- env/printenv: elastic export snapshot; explicit HOME/PWD overrides honoured;
  env -i/-u suppress synthetic defaults within saved scope. Use an absolute
  external path or supply PATH with env -i. The initial fixture incorrectly
  expected stripped PATH to locate Audit commands; corrected fixtures pass.
- sleep yields while waiting in native 60 Hz ticks. time prints elapsed wall
  time to stderr and returns the command status; no fake user/system CPU figures.

Host ASan/UBSan: eight dd copies match /bin/dd; size overflow and allocation
failure preserve an existing destination; cancellation/write failure return
nonzero and release ownership. mktemp: 32 deterministic collisions, existing
contents untouched, output-failure cleanup, invalid/quiet options. printenv:
64 entries of 2,048 bytes, allocation failures; sleep zero/one-second/cancel.
Nine dash environment/scope comparisons, time/exit isolation, 149 recovery trials
(41 injected failures). Preserved 39 focused shell +54 POSIX cases, 1,268 parser
recovery trials, 65 loader/installer cases, and editor environment tests (80 x
4,096 bytes, 88 failure points, cancellation and 20 repeats).

Snow Plus/4 MB, **reported System 6.0.7**, fixed maximum speed ~15–16x:
37 final utility cases passed. Eight exported HFS files compared byte-for-byte,
including 128 KiB all-byte-values copying, zero gaps, empty files and originals
preserved after native OOM/locked-file failure. CRCs match the host utility.
2,049-byte export reaches printenv, env isolation reaches awk, pipelines/statuses
work. Actual Ctrl+C interrupted sleep and unbounded zero-to-null dd with status
130 and returned to a usable prompt. That cancellation run precedes the final
record-format-only dd correction; it was not needlessly repeated.

Six repeated dd/printenv/mktemp/remove cycles: free heap 290,492 then 290,482 bytes;
largest block alternates 289,034/289,164, without progressive decline. Each
record reports remaining=0. Measured owned peaks: dd 5,744 bytes, printenv/mktemp
5,716 bytes for these fixtures; scopes/script memory are separate. Final CLX
sizes: dd 18,264, sleep 12,688, printenv 13,168, mktemp 16,412 bytes. No new
permanent buffer/cache; shell default-environment mask adds one integer.

## Help coverage

Final `help-inventory-final.json`: **43 bundled names +32 builtin topics +nano**.
All 76 actual guest help calls succeed on stdout with no stderr; Clarice help
fits 72 columns, upstream nano 80. All external option sets were compared with
current parsing/source, and `help-content.py` checks 46 capability contracts;
`builtin-help.py` checks the builtin entries. Initial errors corrected include
awk option handling, native usage stderr, builtin semantics, upstream adaptations
and misleading generic allocation/encoding wording. No production command lacks
help. Exhaustive option combinations and new serial/network/editor operation
acceptance are not claimed by a help sweep.

Exact guest script/cases: `second.sh`, `second-cases.json`. Consolidated actual
outputs, memory and verified binary hashes: `second-guest.json`. Normal build
fingerprints: `build1549.json`. Validators replay assertions on saved evidence,
not the emulator itself. Build with existing `scripts/build.sh`; nano was not
rebuilt. Guest fixture/test outputs are disposable; no OS images or packages are
committed. Physical SE/System 6.0.8 acceptance remains pending.

Remaining deliberate limits: dd uses signed 32-bit HFS offsets, seek requires
of=, no conv=noerror/raw disks/fork-copy/safe-editor-save; partial output remains
on failure. mktemp directory mode is not supplied. time covers one command,
not an enclosing pipeline, and does not support -p. These are documented in help.

## First-batch evidence (builds 1547–1548)


Host and fixed maximum-speed Snow (~15–16x), Macintosh Plus, 4 MB, guest reports
System 6.0.7. No physical-SE or System 6.0.8 acceptance claimed.

## Verified changes

Pinned sbase c546c3a adds cmp and cksum through the existing portable runtime.
Source and manuals are unchanged; preparation adds help and cooperative polls.
CLX sizes: cmp 17,660 bytes; cksum 18,664 bytes. No idle allocation or cache.
File streams contain data forks, not resource forks/Finder metadata.

Host: ASan/UBSan binary 1 MiB/reference comparisons, invalid options, empty and
missing files; 16 actual-runtime allocation-budget/cancellation/output-error
trials plus recovery, all report zero tracked allocations. Snow: 16 functional
cases and 12 repeat pairs; raw results consolidated in guest-results.json. CRC of all byte
values repeated to 65,536 bytes: 3547434670, matching the host POSIX utility.

All actual guest help invocations returned zero with stdout and empty stderr.
Native invalid-usage stream corrections and awk unknown-option rejection also
passed guest checks. Some help still needs deeper examples/interactions review;
basic availability and formatting must not be confused with complete coverage.
Experimental nano was not rebuilt: only its existing --help was exercised.

## Reproduction

```
python3 tests/commands/builtin-help.py
python3 tests/commands/compare-checksum.py
CLARICE_TEST_COMMANDS='cmp cksum' sh tests/ports/build-pipelines-host.sh
python3 tests/commands/compare-cleanup.py
python3 tests/utility-correctness/help-inventory.py
python3 tests/utility-correctness/validate-help.py
```

Last validator checks saved actual guest evidence; it does not rerun Snow.
help-inventory.py generates the guest commands. Stage current CLX files into
/mnt/Audit/tools and run its help.sh on the disposable existing Plus fixture.
For new-command cases see cases.json and tools.sh. Never mount guest images for
host writes while Snow is using them. Build uses the existing pinned Retro68
scripts/build.sh; the measured build was completed incrementally.

## Complete basic help coverage

Every row passed status/stdout/stderr and width checks. Comprehensive advertised
option execution remains partial outside the focused cases above.

| Command | Kind | Bytes | Maximum columns |
|---|---|---:|---:|
| `ls` | external | 936 | 62 |
| `df` | external | 225 | 63 |
| `cat` | external | 229 | 67 |
| `benchls` | external | 142 | 71 |
| `benchscan` | external | 142 | 71 |
| `grep` | external | 657 | 71 |
| `sed` | external | 672 | 71 |
| `awk` | external | 889 | 72 |
| `serial` | external | 608 | 72 |
| `wc` | external | 590 | 72 |
| `head` | external | 577 | 72 |
| `cut` | external | 725 | 72 |
| `tr` | external | 555 | 72 |
| `uniq` | external | 752 | 72 |
| `tee` | external | 505 | 72 |
| `tail` | external | 817 | 72 |
| `sort` | external | 1032 | 72 |
| `cp` | external | 429 | 71 |
| `mv` | external | 237 | 66 |
| `touch` | external | 246 | 69 |
| `rmdir` | external | 115 | 58 |
| `mkdir` | external | 150 | 60 |
| `rm` | external | 247 | 70 |
| `ln` | external | 152 | 61 |
| `readlink` | external | 141 | 58 |
| `hfsattr` | external | 309 | 70 |
| `find` | external | 502 | 72 |
| `du` | external | 372 | 72 |
| `file` | external | 240 | 69 |
| `od` | external | 388 | 70 |
| `more` | external | 302 | 72 |
| `basename` | external | 422 | 72 |
| `dirname` | external | 386 | 72 |
| `kermit` | external | 584 | 72 |
| `wget` | external | 468 | 72 |
| `clarice` | external | 448 | 70 |
| `cmp` | external | 722 | 72 |
| `cksum` | external | 728 | 72 |
| `appinstall` | builtin | 721 | 72 |
| `cd` | builtin | 88 | 72 |
| `pwd` | builtin | 39 | 34 |
| `echo` | builtin | 89 | 72 |
| `printf` | builtin | 611 | 70 |
| `export` | builtin | 428 | 72 |
| `unset` | builtin | 220 | 64 |
| `set` | builtin | 373 | 71 |
| `shift` | builtin | 54 | 39 |
| `read` | builtin | 639 | 71 |
| `sh` | builtin | 608 | 70 |
| `.` | builtin | 307 | 71 |
| `test` | builtin | 587 | 71 |
| `[` | builtin | 139 | 62 |
| `true` | builtin | 36 | 19 |
| `false` | builtin | 38 | 20 |
| `:` | builtin | 47 | 29 |
| `break` | builtin | 58 | 51 |
| `continue` | builtin | 63 | 53 |
| `return` | builtin | 50 | 33 |
| `exit` | builtin | 74 | 59 |
| `history` | builtin | 167 | 62 |
| `prefs` | builtin | 813 | 72 |
| `status` | builtin | 367 | 72 |
| `clear` | builtin | 34 | 27 |
| `selftest` | builtin | 46 | 36 |
| `checks` | builtin | 37 | 29 |
| `scrollbench` | builtin | 290 | 66 |
| `help` | builtin | 231 | 71 |
| `nano` | experimental | 1009 | 80 |

## Outstanding authorised work

Finish help depth/option regressions; null/zero streams, command -v and free;
validate that first batch before dd, time, cooperative sleep, env/printenv and
safe mktemp. No caching or process-architecture changes made.

## First batch completion — 1548

23 actual guest device/resolution/memory cases, plus corrected native memory
query, six repeat pairs and cancellation/recovery are recorded in
`devices-guest.json`. The original free result is retained and labelled pre-fix.
No physical acceptance. CLX runtime 1.4 adds the small native memory snapshot;
existing commands retain their earlier requirements. The virtual stream tokens
need no native file, handle, descriptor table or permanent allocation.

Limitations: virtual nodes are not HFS catalog entries; native catalog/stat tools
and direct-HFS file operands do not gain virtual-node support. Portable stdio
operands and shell redirection work. Seeking and file-size queries fail rather
than pretending an infinite stream is an ordinary file. command -v accepts one
name; no command-execution/-p/-V forms. free reports current heap extent, not
whole application partition capacity or total machine free RAM.

Native memory research: Apple [Inside Macintosh: Memory, PurgeSpace](https://www.vintageapple.org/inside_r/pdf/Memory_1992.pdf)
defines total in D0 and contiguous capacity in A0. The pinned Multiversal binding
emits A062; disassembly showed A0 was preserved by trap dispatch, giving a
3,163,188-byte address as the result. Correct A162 result-return glue, with no OS
patch or actual purge, yields coherent figures. [Macintosh Revealed](https://vintageapple.org/macprogramming/pdf/Macintosh_Revealed_2nd_Edition_Volume_One_1987.pdf)
also lists A162. This is a correctness fix, not an assembly optimisation.

Reproduce focused host checks with tests/commands/virtual-streams.py, free.py,
imports.py, tests/commands/updates.py and host-streams.py. Device guest scripts
are retained beside this report. Use a *single* FFWD toggle to return temporarily
to video speed while injecting keys, then restore maximum speed for execution;
the older double-click helper toggles twice and can cause guest autorepeat.
The first malformed cancellation command was excluded, then correctly repeated.
