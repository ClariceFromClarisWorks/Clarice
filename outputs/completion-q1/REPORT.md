# Q1-006 — responsive completion (Snow acceptance)

Build2026111953, local main. Normal Core and sealed collection rebuilt; no hardware
access or GitHub push. Production confirmation/input tests pass on Snow Plus4MB,
System6.0.8/MultiFinder at maximum FFWD. No renderer algorithm changes.

## Cause and correction

Old completion enumerated every entry even after displaying50 suggestions; its
event yield was skipped for nonmatching entries. New completion yields every8
scanned entries, aborts on queued typing or cancellation, and never applies a
partially calculated prefix. Existing input replay handles the triggering keys.
First Tab preserves single-match/LCP behaviour and stops early only when further
matches cannot extend the prefix. Only consecutive Tabs offer suggestions.
Second Tab streams at most16 names or4 terminal rows of characters, then asks
`More matches? [y/N]`; y continues, n/Return/Escape stops, other typing returns to
the original command. No match-count prepass, stored directory list or cache.
Case-insensitive byte matching, quotes/escaped spaces and volume paths are covered.

GNU Readline's established query/page convention informed the UI; its complete
match-array approach was not imported into the bounded native HFS scan.
[Reference](https://sourceware.org/gdb/download/onlinedocs/gdb.html/Readline-Init-File-Syntax.html).

## Measurements

Matched diagnostic completion-call medians, guest ticks (60ticks/s), three runs.
These include native I/O and event servicing, not pure CPU time; final outer-loop
presentation is measured separately by the injected-input tests. Snow results are
comparative evidence, not physical-SE acceptance.

| Entries / operation | Before | After |
|---|---:|---:|
| 1, first Tab |17|17|
| 10, first Tab |19|34|
| 100, first Tab |65|58|
| 1000, first Tab, LCP extends `e` to `e0` |345|365|
| 1000, second Tab / first suggestion page |365|37|

The full catalog scan alone costs249–250ticks; output-only8/16/32/64 names costs
7/8/10/13ticks.16 is a conservative measured page bound, further reduced for long
names/narrow windows. Second Tab improves about90%; first-Tab full LCP is still
linear and may take~6guestseconds. Cooperative event handling adds some cost to
small multi-entry scans; this change targets responsiveness, not a faster HFS API.

A repeat run confirms second-Tab41–42ticks. Input injected just after a poll is
handled in2–3ticks and visibly restored in5–6ticks (maximum observed100ms), including
with only32,760bytes contiguous free. This is a tested bound, not a guarantee
against a slow native catalog call. Retained heap is zero in all warmed runs;
free heap361,646 before/after, largest358,084; constrained free36,322 before/after.
The cold24,328byte renderer cache allocation also occurs in the old build and is
not introduced by completion.

## Correctness and cost

ASan/UBSan actual completion code: single/LCP,1000entries, case/quotes/apostrophes,
escaped/volume spaces, bounded pages, editing reset, overflow without truncation,
nonmatch and partial-match cancellation, native memory-error recovery and1000
repeated calls. Existing input/control, viewport and8,505geometry checks pass.
An obsolete input-layout assertion was corrected: its old first+prompt coordinates
were inconsistent with current_line and the existing wrapped-input geometry.
Normal1953: quoted HFS file executes successfully, first Tab does not list,
y/n/Return/other typing, Ctrl+C130/recovery and both themes pass. No command-owned
allocations remain. Diagnostic hooks are excluded from the normal build.

68000 binary: text235,236 ->236,204 (+968); data34,848 unchanged;
BSS41,436 ->41,444 (+8); shell_complete stack frame2060 ->2592 (+532).
MacBinary430,848 ->431,872 (+1024). No new heap allocation or persistent bitmap.

Remaining: physical-SE acceptance; full-scan LCP latency and native catalog-call
latency remain. Non-ASCII case behaviour follows the existing byte/locale model,
not a new Unicode or international HFS collation implementation. No index/cache
or native filesystem changes were introduced.
