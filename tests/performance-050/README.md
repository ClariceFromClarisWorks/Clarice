# Bounded 0.5.0 utility performance audit

Run from the repository root. `python3 tests/performance-050/fixtures.py`
generates deterministic inputs and small case scripts in ignored
`work/performance-050`. Stage on a disposable HFS volume named Audit using the
existing hfsutils workflow, **only while Snow is stopped**. Create ten/hundred
folders with respectively 10/100 distinct small files. Preserve baseline utility
images under Audit:before before replacing candidates. Do not use user storage.

Use the existing 4 MB Plus Snow fixture at maximum FastForward. This fixture
reports System 6.0.7: it is not System 6.0.8 or physical-SE acceptance. The audit
profile invokes baseline.sh with output redirected to Audit:baseline.txt. Each
small script runs three complete invocations with `time`, exit status and
`status runtime`. Run after.sh for the affected commands and additional utility
coverage, final.sh for production wc/tr and repeated-run heap checks. Retain
old and new find/du separately for alternating matched runs. `acceptance.sh`
requires a disposable tree with space/slash/percent names and a forked fixture;
run its setup once, omit the first mkdir/cp/ln lines on subsequent comparisons.

After scripts close their reports, explicitly flush/unmount the disposable
volume, quit the guest application and Snow, then export results. Never unmount
from inside a script/report held open on that volume. Missing/incomplete reports
are not passes. Parse with `python3 tests/performance-050/summarize.py REPORT`.
The module counters are stale for shell-only commands and cover only the final
pipeline stage; use the outer `time` line for those workloads.

Focused host checks:

```
sh tests/performance-050/utf-host.sh
python3 tests/performance-050/traversal.py
sh tests/ports/build-pipelines-host.sh
```

The UTF differential checks compare unchanged upstream helpers under ASan/UBSan,
including malformed input, every Unicode whitespace candidate and error returns.
Traversal tests compile the actual implementation with hostile native-catalog
mocks, explicit volumes, overwritten PB fields, allocation failure and cancellation.
Existing QOL-1 native namespace regression remains required. Build pipeline
utilities through scripts/build-pipelines.sh in the pinned Retro68 container;
inspect uses the existing inspect target in scripts/build.sh (no Core change).

For guest cancellation run cancel-tr.sh or cancel-find.sh, send Ctrl+C during
execution, record `$?` and `status runtime`, then run a successful small command.
Expected status130, no retained command allocations and usable subsequent I/O.
Compare exported UTF/binary output to unchanged upstream reference-wc/tr; compare
before/after traversal outputs and both forks/Finder metadata of the fixture.

clock.c is a disposable CLX CPU loop. Normal and maximum pacing both produced
66/132/264 guest ticks for 100k/200k/400k iterations. This checks timing scale,
not physical performance or asynchronous network timing. See the report for
limitations, measured results and the consolidated hardware plan.
