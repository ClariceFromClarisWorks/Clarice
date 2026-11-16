# Native heap ownership and elastic editor environment — build 2026111545

Snow Macintosh Plus, 4 MB, **reported System 6.0.7**, fixed uncapped FFWD
(~15–16×). No physical SE or System 6.0.8 acceptance. Normal application build;
experimental nano relinked from existing upstream/ncurses objects with only the
terminal adapter recompiled. No editor optimisation or allocator redesign.

## Root cause and fix

Upstream awk advances its local argv pointer and assigns `argv[0] = cmdname`
(`src/vendor/awk/main.c:241`). That overwrote the shell's program-argument pointer.
Shell cleanup subsequently lost that allocation and attempted to free argv[0]
twice. Port tracking could correctly say zero while the shell-owned string leaked.
The old/new environment implementations both exposed the same defect.

CLX now passes a temporary, NULL-terminated pointer-vector copy. The shell retains
its original ownership; writable strings and normal C argv mutation remain valid.
The vector is released on return, including error/cancellation. Allocation failure
prevents execution and cleans up the image. No executable-format/ABI change,
checksum-policy change, resident reservation or modified memory accounting.
Cost: 4 × (argc+1) bytes plus native block overhead for the invocation only (12 B
for `awk 'program'`), one NewPtr/DisposePtr pair and a pointer copy.

## Native evidence

- Builtin `:`/echo, cat and ls controls remained flat apart from tiny allocation
  rounding/layout differences. Empty awk, string, numeric and regex variants
  showed workload-dependent decline; see raw control/isolation files here.
- Read-only 24-bit heap walks: four empty awk invocations increased live pointer
  blocks **151 → 152 → 153 → 154**, while handle count stayed **36** and handle
  bytes stayed **255074**. Free-block sums matched FreeMem exactly; CompactMem
  did not recover the bytes. This was not a resource handle, reclaimable pool or
  FreeMem measurement artefact. Zone counter fields returned zero on this fixture;
  they are not used as evidence. The independent diagnostic walker is restricted
  to this known fixture and is never part of production runtime.
- Previous matched old/new awk: each lost **384 B across four intervals**.
  Fixed new awk: **362454, 362452, 362452, 362452, 362452 B** free; largest
  **362220, 362218, 362218, 362218, 362218 B**. Old awk also plateaus after warmup.
  Ten fixed empty-program runs: **362352 B** free throughout. Owned remaining=0.
- Editor adapter guest: five runs importing **40 × 2048-byte exported values**,
  `TERM` override, private-variable exclusion and set/unset pass. Free heap
  alternates **309742/309744 B**; largest **306934 B** throughout. Peak tracked
  allocation **97220 B**, remaining=0 each time.

## Editor environment

Replaced the 18-import/24-slot/1024-byte restrictions with invocation-owned linked
entries and a growing import buffer. Imported values are never silently omitted.
`TERM=clarice` overrides the exported terminal setting; unexported variables remain
invisible. Import allocation failure reports an error and exits 2; ordinary
setenv failure preserves the previous value. Existing runtime cleanup owns partial
imports, errors and cancellation. Canonical input's separate 255-byte bound is
unchanged. No permanent bitmap, environment cache or shared allocation added.

## Focused host checks

- Actual loader/installer: 65 scenarios plus argv mutation on status 0/2/130,
  argument-vector allocation failure and subsequent recovery; ASan/UBSan clean.
- Editor production adapter: 80 × 4096-byte exports; all 88 import-allocation
  failure positions; cancellation; failed replacement preservation; 20 repeats.
  Peak host allocation 337062 B; cleanup zero. Terminal mode/signal checks pass.
- 54 POSIX + 39 focused + 41 elastic comparisons, 12 complete scripts,
  31 basic/3 cancellation cases; 9874 parser fault trials and 3090 scope-failure
  positions retained and rerun. No shell parser or expansion change.
- Six unchanged upstream script variants rerun in Snow: exact dash output/status,
  including space paths and full upstream Caps. Seven malformed scripts retain
  syntax diagnostics/status2 and recovery. Provenance remains in
  `tests/shell/practical/SOURCES.md` and `outputs/shell-practical`.

Reference study: existing newlib FILE-pool investigation remains applicable
(`outputs/utility-correctness/LIBC-POOL.md`); its reusable 2180 B high-water pool
was not changed. Heap-layout cross-check used metamage_1's ams-core/Zones.cc;
no reference implementation code was copied or installed.

## Final guest acceptance

- Three injected editor admission failures (10000-byte cumulative fixture budget)
  report insufficient environment memory/status2; every following unrestricted
  40 x 2048 import returns0. Free heap **289756/289758 B**, largest **286050 B**;
  owned remaining=0 on failure and success. This is targeted fault injection,
  not a changed production ceiling.
- Repeated malformed awk programs return2. 1000-entry and 100000-entry arrays
  genuinely exhaust this fixture's available application heap at roughly 914–917
  entries (owned peaks 203598–203688 B), report errors,
  return2, and free all owned memory; the following ordinary awk succeeds. This
  is not treated as a successful 1000-entry workload or a new artificial limit.
  Full upstream Caps processing succeeds in the practical-script fixtures.
- Four post-fix heap walks retain **769 pointer blocks / 36 handle blocks**,
  with no per-invocation growth. Absolute counts differ from the initial fixture
  because the active shell AST/workload differs. Largest block **234470 B**.
- Nano: six interactive launches total. First run saves a disposable file; later
  runs reopen it. A matched three-iteration loop with 30 exports plus a 1500-byte
  value returns0 each time: free heap **399650,399658,399658 B**, largest
  **236688 B**, owned peak **87684 B**, remaining0. Earlier measurements made
  inside different interactive command lines are kept as unmatched evidence,
  not used to claim a trend. No nano source or ncurses rebuild.
- Startup-profile cancellation visibly stops the loop and skips its following
  command, but startup resets the later shell status to0. It is not used as the
  status130 acceptance result; an ordinary-prompt check is recorded separately.

Known limits: this is focused repeated-run stability, not proof of leak freedom
for every utility. Native stdio retains its documented reusable high-water pool.
The separate terminal canonical-input limit and physical hardware acceptance
remain unchanged. No architecture or allocation-policy decision needed.

Ordinary prompt cancellation returns **130**, skips `BAD_AFTER_CANCEL`, restores
stdout/stderr and successfully runs a **500-entry array** afterwards (prints499,
status0, owned peak124769 B, remaining0). The diagnostic unmount helper reports
-47 when called inside redirected output still open on Audit; this is an expected
fixture sequencing limitation, not a transfer/command failure. Clarice was then
quit normally before stopping Snow and exporting. No live disk image was modified.

Reproduce with the existing pinned Retro68 image, `tests/heap-audit/build.sh`,
then pack its documented objects. Reuse the parser-safety Plus/system/audit image;
stage the diagnostics and acceptance.sh on Audit while Snow is stopped. The
normal build follows the existing CMake/finish-application path. Run
`tests/heap-audit/verify.py` against exported evidence; host environment checks are
`tests/terminal/environment.py` and `tests/ports/environment-audit.py`.

Local, untracked artifacts: `build/Clarice.bin` (957952 B) and `build/nano.clx`
(268212 B). The latter is an experimental independently installed editor, not a
new bundled utility or a release. Checksums are in `build-inputs.json`.

Heap-walk files retain the measurement and counted-block summary; full diagnostic
address listings remain local in work/heap-audit. Error and saved-text JSON retain
exact whitespace without adding trailing-whitespace noise to the Git diff.
