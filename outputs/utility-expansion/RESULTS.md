# Utility expansion — build 2026111547

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
