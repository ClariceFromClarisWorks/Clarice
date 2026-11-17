# Parser safety and elastic capacity — build 2026111542

## Cause and changes

The accepted 1541 probe reproduced a NULL dereference immediately after parser
allocation. The same assumption appeared in AST constructors, token buffers,
heredoc delimiters, arithmetic parsing and function-copy paths. Array append and
partial-copy failures also lacked consistent ownership cleanup.

The existing mrsh representation and destructors are retained. Constructors report
failure; callers unwind owned children; only completed nodes enter arrays or replace
function definitions. Parser errors remain available under sustained exhaustion.
Syntax errors no longer assert when a reader encounters the end marker. Buffer and
array size arithmetic is checked. Whitespace token scanning is iterative.

Source size, total token count, function count/name and program-chunk ceilings are
removed. Functions use individually owned nodes and snapshots clone only existing
entries. Source/execution recursion limits (8/16), parser recursion (32), and deep
AND-OR/arithmetic chains (64) protect the Macintosh stack. These are explicit
resource safeguards, not POSIX limits. Interactive input remains 1023 bytes.

Research: [upstream mrsh parser](https://github.com/emersion/mrsh/blob/master/parser/parser.c)
shows the unchecked construction pattern; [dash memory management](https://github.com/herbertx/dash/blob/master/src/memalloc.c)
uses checked allocation and stack-region unwinding. Importing dash's allocation
model would replace ownership conventions unnecessarily. This change instead uses
mrsh's existing object ownership and destruction. Upstream MIT attribution remains.

## Executed checks

- Host ASan/UBSan and explicit accounting: 68 parse fixtures / 9,874 injected
  allocation positions; 59 valid AST-copy fixtures / 8,268 positions; 660 arithmetic
  failure/recovery trials. Single-failure and sustained-exhaustion modes. Every
  trial tears down to zero tracked bytes and parses another command successfully.
- Five whole-shell fault sweeps cover 1,268 positions, function retention,
  substitutions, pipelines, redirections and subsequent execution. Legacy 18
  compound fault-position trials now hit six current sites; the full copy sweep
  covers the replacement copy implementation. The original constructor probe now
  exits with status 2 and an error, rather than crashing.
- Existing 31 host comparisons / 3 cancellation cases, 39 focused cases, 12 complete
  scripts, 41 elastic cases and 329 adapter failure positions pass.
- Snow diagnostic build, Plus / 4 MB, **reported System 6.0.7**, uncapped FFWD about
  15–16×: all 39 focused, 12 complete and 41 elastic cases pass. New 32 KiB source,
  600-command, 48 long-named function and 200-argument cases pass. Invalid syntax
  and excessive nesting/chains return status 2.
- Guest diagnostic script executes 441 trials, 282 of which actually inject a
  failure. All return to the caller with **zero retained parser bytes** and a
  callable pre-existing function. A script can finish with status zero after an
  earlier failed command, as with ordinary sh without `set -e`; this is distinct
  from an ignored parser failure. Diagnostics are absent from normal builds.
- Diagnostic repeat heap varies by four bytes (391166–391170), largest block
  227664 throughout; portable command allocations remaining=0. These measurements
  include the diagnostic application and its driver, not the normal idle shell.

Normal build: a 40 KiB / 20,000-command source exhausts the actual guest heap.
Three runs return status 2 / `sh: out of memory`, execute no partial program, and
successfully run subsequent commands. Free heap: 411350, 411360, 411360 bytes;
largest blocks: 410220, 411336, 411272. No downward retained-memory trend.
The 32 KiB, 600-command and 48-function cases pass again. Using the existing patched
Snow keyboard backend, actual Ctrl+C cancels substitution with status 130, skips
the following command and restores stdout/stderr. Portable remaining=0.
No physical Macintosh or System 6.0.8 acceptance is claimed.

## Costs and limits

Small `echo hello` retains the 1541 host adapter peak of 259 bytes, six allocations,
and zero retained bytes (excludes parser/libc). Host 10,000 parse/destroy samples
are recorded separately: this is correctness overhead evidence, not SE timings.
No permanent bitmap/cache/reservation was added. Removing the static function
slots also removes their empty-slot cost from execution snapshots.

Large input still incurs the inherited reader's repeated buffer shifting; parsing
very large sources can be slow and does not yet poll cancellation inside lexical
scanning. Cancellation during execution remains a separate acceptance check.
No wholesale parser rewrite or expanded shell semantics is claimed here. Embedded
quoted `$@`, export visibility, printf and test remain the next compatibility batch.

## Reproduction

`sh tests/parser/build.sh`, then `python3 tests/parser/sweep.py`,
`python3 tests/parser/sweep.py 0 copy`, `python3 tests/parser/arithmetic.py`,
`python3 tests/parser/shell-recovery.py`, and `python3 tests/parser/capacity.py`.
Use the existing shell scripts for the retained regression groups.

`CLARICE_PARSER_SAFETY_TEST=ON` enables only the diagnostic build's `parsercheck`.
`tests/parser/prepare-guest.py` stages copied existing disposable images and fixtures;
ROM/config paths stay local. Never mount an image in hfsutils while Snow uses it.
Export after the native finish helper flushes/unmounts Audit, then run
`python3 tests/parser/guest-results.py`. Build normally with the diagnostic option
OFF before delivering a development application.
