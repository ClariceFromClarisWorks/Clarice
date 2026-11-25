# Batch 3C focused traversal measurements

Use only disposable, stopped Snow images. `fixtures.py` clones the preceding
Batch3B fixture and writes an hfsutils setup script for a 32MiB volume with
10/100/1000 files (1KiB data files, plus a valid resource-bearing file per folder),
split across2/10/10 subfolders. It does not start or access physical hardware.

`profile.py before|after` wraps the actual utility source; baseline is0f82946.
It records command-body guest ticks, yield intervals/time, allocation counts and
frame payload bytes. Enumeration catalog counts include native_nth's two known
PBGetCatInfo calls per invocation; volume counts are *volume scans*, not individual
PBHGetVInfo traps. These scoped counters exclude resolver/native-copy internals.
No instrumentation is added to the shipped Core or utilities.

`build-candidate.sh` builds only the two normal68000 utility images. Preserve the
accepted baseline images as `before/cp,rm,find,du` on the Perf volume, and candidate
images under `after/`. Keep the same command basename for multicall dispatch.
`guest.py before|after` records counters; `pairs.py` alternates the normal commands
three times on identical fixtures/destination names. It separates runtime load
from execution ticks and also records outer `time` output and native heap.
Run with macOS `/usr/bin/python3` (existing Telnet client) against the owned Snow
NAT forward23260. Never point these destructive fixture commands at user volumes.

`acceptance.py` compares find/du output/options, symlink safety, encoded names,
fork/metadata preservation with the existing fsprobe, cancellation and repeated
cleanup. Host coverage reuses performance-050/traversal.py, utility-correctness/
file-depth.py and the Batch3A/B fault suites. `enumeration.py` checks the production
native enumerator with HFS overwriting result fields, errors and leaf boundaries.
