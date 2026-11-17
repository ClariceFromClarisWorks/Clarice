# Elastic shell, batch one — build 2026111541

Accepted 1540 checkpoint: `2115cd8`, local on
`codex/utility-correctness-2026111539`. This batch leaves the parser's AST shape,
terminal input limit, HFS implementation and external executable ABI unchanged.
No physical tests, push, tag or release.

Implemented: demand-grown variables, words, argument vectors and command
substitutions; multi-variable read; correct function `$0`; positional isolation
across subshell/substitution execution. Values are staged before replacing old
state, and restoration does not allocate. Snapshot and expansion lifetimes were
adjusted together to avoid dangling variable pointers. Size arithmetic is checked.
Small commands no longer allocate fixed 4 KiB word and argument arrays.

## Evidence

- Host production adapter with ASan/UBSan: 31 original comparisons/3 cancellation,
  39 focused cases and 12 complete scripts preserved; 41 new dash comparisons.
  `elastic-failures.py`: **329** injected adapter allocation failures, no retained
  tracked allocation, no sanitizer failure. Existing 11 compound failure/control
  cases and 18 compound allocation-position trials remain passing.
- Snow Macintosh Plus, 4 MB, **reported System 6.0.7**, fixed uncapped FFWD about
  15–16×: 41/41 elastic outputs/statuses match dash; 39 focused cases and outputs
  of all 12 utility scripts match. `$0` comparisons use the actual guest script
  filename as dash's argument zero. Includes 64 KiB read/expansion, 8 KiB command
  substitution, 80 variables, long names, and 100 arguments.
- Native 512 KiB record exhausts available memory explicitly: `2:3:old` confirms
  read status 2 and unchanged old variable. This is genuine available-heap failure,
  not a new fixed record cap. A failed read can leave unread input; check status.
- Actual guest Ctrl+C cancels `x=$(while true; do :; done)` with 130, does not
  execute its following command, and restores stdout/stderr. Final helper flushes
  and unmounts only the disposable Audit volume before host export.
- Five matched repeats: free heap 409058, then **409068 B** on all four subsequent
  runs; largest block **233372 B** throughout. Portable remaining=0 after cancel.
- Host-only adapter peak: `:` 8472→250 B, `echo hello` 8472→259 B,
  small assignment/echo 8472→283 B, short read/echo 8472→375 B. These exclude
  parser/libc allocations and use 64-bit host sizes. More small allocations replace
  the old oversized frames; no physical speed claim. Native fixed global arrays
  shrink by about 2.6 KiB; live values/positional parameters now consume heap.
- Initial Snow 64 KiB failure exposed unnecessary doubling peaks. Record growth
  now uses 1.5× capacity and large word appends reserve at least their actual need,
  without rounding a single large append up to the next power of two. Final run
  preserves all 65536 bytes. No permanent allocation was added.
- Normal MacBinary: 948736 B, **+1920 B** over 1540. Optional activity/nano
  diagnostics remain off. Bundled external utilities are unchanged.

The crowded earlier Audit fixture triggered Finder's known memory dialog before
Clarice launched. A fresh disposable driver-bearing disk with grouped fixtures
resolved that test-environment issue. The accepted disk and physical SE were not
altered. Synthetic keyboard entry briefly used normal speed; all functional runs
and Ctrl+C execution used uncapped FFWD.

## Remaining work and parser boundary

The 8 KiB source, 256-token and 16-function restrictions remain, as do explicit
source/execution recursion safeguards. Embedded quoted `$@`, complete export/
locality and further printf/test semantics remain incomplete. Old awk/editor
runtime environment consumers still have independent 18-entry/buffer restrictions;
expanding shell storage does not make those consumers fully elastic.

`parser-oom-probe.py` demonstrates a **pre-existing unsafe failure**, not a passed
OOM test: failing the parser's first calloc causes a null-pointer dereference.
AST constructors have the same unchecked-allocation pattern; some parser/copy
callers ignore failed array growth. Source-size/free-memory preflight checks are
not a proof of allocation safety. The existing AST-copy headroom check is retained
both before and after elastic snapshot allocation.

Before removing the remaining parser guards, the required next batch is coordinated
allocation-failure propagation through parser construction, AST construction/copy,
and their callers, with failure-position tests. Prefer retaining the current AST
representation and explicit cleanup, rather than introducing an allocation arena,
nonlocal unwind scheme or a replacement parser. That is broader than another
constant removal and must be treated as a distinct parser-hardening change.

Reproduce host evidence with `scripts/test-shell.sh`, then
`tests/shell/{posix-focused,scenarios,compound-failures,compound-oom,elastic,elastic-failures,read-overflow}.py`.
The parser probe intentionally demonstrates the remaining unsafe allocation.
Guest captures and summaries are under this directory; large disposable fixture
outputs/build media remain local. The compatibility matrix and CURRENT-STATE
explicitly distinguish this completed batch from the unfinished overall task.
