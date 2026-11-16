# Focused utility close-out — build 2026111546

Baseline ee2faf8 / 1545 preserved. Host and Snow Plus / 4 MB / reported **6.0.7**,
fixed maximum acceleration (roughly 14–16x observed, briefly normal for synthetic
key entry). No physical-SE or System 6.0.8 acceptance, release or main changes.

## Changes and evidence

- Profile wrapper unconditionally erased status. Separately, an interrupted loop
  condition could leave source return status zero while cancellation flow was set.
  Source completion now preserves return/exit/cancel status; profile retains it and
  distinguishes interruption. Production wrapper host test: success, false, return
  inside loop condition, exit versus dash, first allocation failure, 80 cancel
  points, subsequent command execution and zero retained references/allocations.
  Snow `closeout.out` starts `PROFILE_STATUS:130`; `cancel.out` records ordinary
  prompt 130 and successful subsequent find. Neither path executes its trailing
  BAD marker (also inspected on screen). Startup fixture was then restored to a
  normal PATH-only profile.
- `find` previously overwrote repeated -name/-type tests. Validated argv is now
  borrowed for conjunctive filtering. Multiple roots and quoted -path patterns
  make practical tree filtering possible without an expression parser or extra
  allocation. * and ? only; no -o/-exec/bracket expressions. -print remains an
  idempotent request for the final matching rows, not an ordered side-effect action.
  Host depth 24, maxdepth, both-fork du accounting, 26 allocation failures,
  cancellation, missing roots, conflicting predicates, -path and multiple roots
  pass with ASan/UBSan. Existing HFS traversal and metadata operations unchanged.
- `ls`/`df` wrote operands unchecked into 32-pointer arrays. Baseline host execution
  reproduces index-32 / stack-buffer-overflow. Revisit validated argv rather than
  collect pointers: removes each 128-byte 68k operand table, adds a few scalar
  cursors, no heap allocation or argv mutation. Host 130 operands, default operands,
  --, usage errors, file failures and cancellation pass. Production Snow 100 ls
  operands / 100 df rows pass, with an empty invalid-option stdout file.

Initial guest operand results accidentally used stale core.clx: the focused
compile omitted the existing build script's misleading-indentation warning flag,
then subsequent commands packaged the old object. These results (`operands.out`)
are explicitly **not acceptance**. Rebuilt using normal flags and fail-fast command
execution, compared staged df byte-for-byte with core.clx, and reran: `final.out`
has LS:0 / DF:0, `many-ls` has 100 records, `many-df` has 100 volume rows.
No HFS change was justified. A diagnostic build was not retained or packaged.

Three final find invocations: free heap 396578 / 396576 / 396578 bytes, largest
block 393318 throughout, tracked remaining=0. New code adds no permanent allocation.
No speed claim from accelerated guest timing counters. Artifact sizes/hashes and
host result excerpts are in `host-and-build.json`; normal MacBinary remains in
`work/build/Clarice.bin`, core/inspect CLX in that same build directory.

## Scope decisions from the existing inventory

| Restriction | Actual consumer / decision |
|---|---|
| 255-byte canonical terminal input | Existing canonical adapter API; nano uses raw input. No active nano record restriction; unchanged. |
| 510-byte converted printf format | Shared runtime format text, not formatted record/value length. Explicit error; no new ordinary-consumer failure demonstrated. |
| 20 temporary slots / 96-byte names | Active sort and nano adapter tracking. Sort has 16 binary merge levels and at most 18 simultaneous run files; table does not bind first. Explicit capacity failure, existing cleanup. No speculative allocator rewrite. |
| Sort 16 merge levels | Genuine remaining artificial limit for very large spill counts, with explicit failure. Defer until a practical failing workload warrants it. |
| Env count/value limits | Awk/editor fixed in 1544/1545; no regression or reason to restore fixed limits. |
| Native 256-byte paths, HFS component/catalog widths | Existing platform/adapter bounds, errors remain explicit. No unrelated filesystem change. |
| Parent-component globbing | Still rejected; final-component expansion works in the guest pipeline. New find -path/multiple roots covers tree-filter workflows without broad shell changes. |
| ls order | Streaming catalog order retained to avoid gathering/sorting whole directories. `ls -1P | sort` is available; no native-order claim of POSIX alphabetical output. |

## Reproduction and validation

Run `python3 tests/shell/profile.py`, `tests/commands/listing-operands.py`,
`listing-host.py`, `listing-format.py`, and `tests/utility-correctness/traversal.py`.
Existing host 31 basic / 3 cancel, 54 POSIX, 39 focused, 41 elastic, twelve complete
scripts, six upstream-script variants and 3090 scope/parser allocation-failure
trials pass. The parser implementation was not changed; its previous exhaustive
construction sweep remains evidence rather than needlessly rerunning it.
Editor environment 80 x 4096 bytes / 88 fault points / 20 repeats also passes.

For Snow, stage current core.clx as tools/ls and tools/df, inspect.clx as tools/find,
and the normal application. Reuse the Audit fixture and maximum-speed automation.
A temporary profile exports PATH and runs `while :; do :; done` after a readiness
marker. Ctrl+C, then source `tests/utility-correctness/closeout.sh` immediately with
stdout/stderr capture. Restore normal profile afterwards. Source `operands.sh` for
100-path checks and memory repeats. Separately interrupt a loop running find,
capture `$?` before another command, and verify subsequent find succeeds.
Flush/unmount with existing Audit/finish **outside** output redirection before
quitting the guest and exporting evidence. Do not mount live guest images.

`closeout-reference.py` compares the portable workflow against dash/macOS tools;
only implementation-specific usage text/nonzero usage status and uniq count
padding are normalized. `closeout-verify.py` validates the saved guest results.
The volume API cross-check used Apple's [File Manager reference](https://vintageapple.org/inside_r/pdf/Files_1992.pdf),
PBHGetVInfo description; no change to volume resolution was needed.
