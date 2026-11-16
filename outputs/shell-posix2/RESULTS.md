# POSIX shell batch — build 2026111543

Accepted parser checkpoint c8ab885 remains intact. This batch changes the existing
mrsh adapter, small braced-parameter parsing and the external environment bridge;
it does not introduce a parser, process model or cache.

## Semantics and references

The empty `$@` discrepancy was loss of quote-group identity. A sole `"$@"` with no
parameters contributes no field; dash retains one empty field for `"$@$@"` within
one group. Separate groups `"$@""$@"` still contribute no fields. The implementation
now retains quoted groups except a sole ordinary `$@`, while emitting positional
field boundaries independently. Prefix/suffix and nested default/alternate cases
are covered. Bash differs on the repeated-empty case; dash is the chosen reference.

Research used [dash expansion](https://github.com/herbertx/dash/blob/master/src/expand.c),
[printf](https://github.com/herbertx/dash/blob/master/src/bltin/printf.c) and
[test](https://github.com/herbertx/dash/blob/master/src/bltin/test.c). Their quote
markers, conversion semantics and arity-sensitive tests guided this independent
adapter implementation. No code was transplanted or licensing removed.

Export flags distinguish local, exported and exported-but-unset names. Assignment
preserves export; snapshots preserve flags; a new sh invocation removes unexported
values and functions. Function/source changes persist, subshell changes do not.
Temporary prefix assignment restoration preserves unrelated function/builtin effects.
`export -p` quotes embedded apostrophes and round-trips unset exports. Native
cli_environment filters the same flags. Existing native HOME/PWD defaults remain.
Sourcing without arguments shares positional changes; sourcing with explicit
arguments retains Clarice's scoped-argument extension (dash ignores these operands).

Practical printf supports integer/string/character/binary-escape output, fixed
width/precision, integer flags, octal escapes, format reuse and missing operands.
It uses 64-byte stack chunks, not width-sized allocations. `%b` handles NUL and
`\c`; output errors and cancellation return nonzero. `test`/`[` distinguish empty
operands, operators, false and errors, and handle grouping/negation without recursive
expression allocation. Integer parsing rejects empty, malformed and overflow values.

## Actual evidence

- `host.json`: 54 complete dash comparisons under ASan/UBSan and allocation accounting.
- `failures.json`: eight whole-shell single/sticky exhaustion sweeps, 3,090 fault
  positions. Every trial restores streams, retains seeded state, runs a subsequent
  command and frees all tracked allocations.
- `regression-*.json`: retained 39 focused / 12 whole / 41 elastic comparisons,
  329 adapter failures, 9,874 parser and 8,268 AST-copy fault positions. Existing
  31 comparisons / three cancellation cases also pass. Historical reports unchanged.
- `host-extra.json`: host external awk export filtering and cancellable million-column
  printf with restored streams / zero retained bytes.
- `guest.json`: normal production build, Snow Macintosh Plus/4 MB/**reported 6.0.7**,
  fixed uncapped FFWD approximately 15–16x. All 54 cases and retained 39/12/41 cases
  pass. Binary stdout comparisons include embedded NUL; expected error diagnostics
  may differ from dash, but statuses/stdout match. Native CLX awk observes exports
  and excludes local variables. Existing utility packages were reused, not rebuilt.
- Actual guest Ctrl+C during billion-column printf returns 130, skips `echo BAD`,
  restores stdout/stderr and permits clean completion/unmount. One initial synthetic
  keyboard attempt could not type `%`; it was cancelled and retried after adding
  that key to the local helper. No acceptance is inferred from that failed attempt.
- Three guest repetitions: free heap **314612, 314612, 314608 B**, largest block
  **225530 B** each, portable remaining=0. Four-byte variation is recorded, not
  claimed as byte-identical. Measurements are inside the profile driver after awk,
  not comparable to the previous parser-only idle fixture.
- Small host echo: six allocations, 267-byte adapter peak, zero retained (1542:259).
  Quote position and per-variable export state are the only added state; no idle
  cache/reservation. Normal MacBinary **957568 B**, +4224 B over 1542.

No physical SE or System 6.0.8 acceptance. No new renderer/editor/filesystem policy.

## Reproduction and limits

Run `sh scripts/test-shell.sh`, `python3 tests/shell/posix2.py`, existing focused,
scenario and elastic runners; `sh tests/parser/build.sh`, then
`python3 tests/parser/shell-recovery.py --posix` and existing parser sweeps.
Use the existing pinned Retro68 normal build with CLARICE_PARSER_SAFETY_TEST=OFF,
then scripts/finish-application.py. `build/SHA256SUMS.txt` identifies the local
MacBinary artifact; disposable images and generated outputs are excluded from Git.

Guest uses existing work/parser-safety images and accepted /mnt/Audit/tools.
`tests/shell/posix2-prepare.py` creates staging commands and the profile; stage only
while Snow is stopped. Launch the existing patched Sys6Snow, toggle FFWD off/on to
fixed uncapped mode, and open Clarice. Wait for POSIX_READY. For cancellation, run
`printf '%1000000000s' x; echo BAD`, press Ctrl+C, then source the existing
/mnt/Audit/elastic/end.sh with stdout/stderr redirected to that directory's
cancel-out/cancel-err. Quit Clarice/Snow after completion. Generate export commands
with tests/shell/posix2-export.py, export with existing hfsutils, then run
`python3 tests/shell/posix2-guest.py`. Synthetic command entry briefly used normal
speed; all functional batches ran uncapped.

Remaining explicit gaps: printf floating formats/dynamic `*` width, test -a/-o and
Unix permissions, general shell options, complex vector operations on `$@`/`$*`,
function-definition redirections, and older awk/editor environment entry/buffer
limits. Native integer formatting uses 32-bit long; a 64-bit host is not a promise
of native numeric range. Large-source lexical cancellation remains unchanged. Some malformed-quote syntax
errors still report the parser’s generic out-of-memory diagnostic (observed in the
failed keyboard attempt); they return nonzero and recover, but wording needs future
correction. No new parser error-policy change was made here.
