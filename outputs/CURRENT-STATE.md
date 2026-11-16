# Clarice current state

16 November 2026, Adelaide. Development **2026111550** separates Core from the
standard utility collection. Host and fixed-maximum-speed Snow Plus/4 MB/
**reported System 6.0.7** validation only; physical SE remains off. No release,
tag or main change. Development checkpoint branch:
`codex/core-distribution-2026111550`, based on accepted `e67c569`.

## Core and utility distribution — implemented

- Core MacBinary **259.12 KiB** (was 1021.12 KiB), zero embedded CLX resources.
  All 43 standard command files are on separate offline native HFS media. Nano
  remains optional. Existing namespace, loader, resolver and updater retained.
- Ordinary startup no longer checks/installs utilities. Matched diagnostic
  repeated ready time **302 ->193–194 guest ticks**; removed catalog-check phase
  measured 107 ticks. Adding payloads back gives 193 repeated ticks: package-size
  reduction is not the source of startup speed. No new permanent allocation;
  payload-only resource-map saving ~330 B. Partition request remains 768 KiB.
- `appinstall setup DIRECTORY` and `install FILE NAME` install absent files only.
  Existing damaged/newer/custom commands remain untouched. `repair NAME DIRECTORY`
  uses external media; check/replace/recover keep existing validation/backup rules.
  Missing commands give an installation hint. [Offline instructions](../INSTALL.md).
- Final normal-build fresh installation from the native Clarice HDA installs all
  43 commands, byte-for-byte verified; **first-run awk OOM resolved**. Awk and
  ordinary pipelines execute. Explicit full setup takes 8565 guest ticks;
  startup never repeats it. `ls /bin` 1.91 guest seconds, loader 59 ticks.
- 77 host loader/installer scenarios, 31 shell comparisons, three cancellation
  cases, 32 builtin-help topics. Snow damaged preservation/repair/recovery,
  incomplete-stage refusal/retry, Ctrl+C130, real 290000-byte heap pressure and
  successful recovery. Native recovery preserves both forks, Finder metadata
  and dates byte-for-byte; profile unchanged. Repeated setup heap stable within
  2 B, largest block fixed, tracked command remainder zero.
- [Evidence, timings and limitations](distribution/REPORT.md). Collection setup
  is resumable, not atomic; leftovers need inspection. No physical/6.0.8 or new
  HTTP-updater acceptance. Native HDA reuses the existing Apple driver/header;
  no System/ROM/user data included. Generated packages stay untracked.

## Second utility batch — build 2026111549

- Genuine pinned sbase `dd`, `sleep`, `printenv`, `mktemp` are independent CLX
  commands. `env` and `time` are small shell wrappers: existing scoped execution
  avoids a nested-CLX process API. No new process architecture or resident cache.
- Runtime 1.5 adds native data-fork resize for dd only. Checked sizes, allocation
  before destructive opens, output-block seek units, explicit zero-filled HFS
  extensions and correct 32-bit record-count formatting. Ordinary writes remain
  partial on error; this does not replace the fork-preserving editor save path.
- Export snapshots are elastic. Explicit exported HOME/PWD now override their
  synthetic defaults; env -i/-u can suppress those defaults within a saved scope.
  Parent environment/functions/arguments/directory restore on return or failure.
- Native exclusive-create mktemp refuses unsafe name-only mode; -d is not added.
  Sleep cooperatively waits in Macintosh ticks. Time reports elapsed wall time
  on stderr; user/sys accounting and -p are explicitly unsupported.
- Snow: 37 utility cases, eight exported files compared byte-for-byte, native
  allocation/locked-file failure preservation, 2,049-byte exported value,
  pipelines, repeated runs and Ctrl+C status130. Repeat free heap settles at
  290,482 bytes; largest block alternates 289,034/289,164; tracked remaining=0.
  Workload includes the executing script, so these are not idle-heap figures.
- Host: eight dd reference cases, output/size/allocation failure, temp collisions
  and cleanup; 64 large environment entries; nine dash scope comparisons plus
  149 recovery trials (41 injected failures); 39 focused and 54 POSIX cases,
  1,268 parser recovery trials, 65 installation/loader scenarios and editor
  environment failure/cancellation/repeat coverage all pass.
- Help: all 43 bundled command names +32 builtin topics +nano exercised in the
  guest. Status/stdout/stderr and 72-column checks pass (upstream nano:80).
  Source/content review and regression contracts describe actual options and
  Mac limitations. Exhaustive option combinations are not claimed.
- Evidence, exact scripts, content inventory, binary checksums and build manifest:
  [utility-expansion/RESULTS.md](utility-expansion/RESULTS.md),
  [help-inventory-final.json](utility-expansion/help-inventory-final.json).
  Consolidated System 6.0.8 physical acceptance remains pending.

## Virtual streams and memory query — build 2026111548

- `/dev/null` and `/dev/zero` use stateless tokens through existing standard and
  portable file I/O; no HFS backing files or permanent allocations. Normalised
  paths and relative `/dev` paths resolve through the existing namespace.
  Read/write modes are enforced; seeks fail. Native catalog tools do not list
  these as physical files; old direct-HFS tools use them through redirection.
- `command -v [--] name` queries existing CLX/script PATH resolution, builtins,
  functions and implemented reserved words. One name; execution/-p/-V not added.
- Independent `free` (2.6 KiB class) reports installed RAM, application heap
  extent, free/largest blocks and purgeable capacity, not fictional Linux or
  machine-wide free-memory figures. Runtime 1.4 adds only `cli_memory`; older
  command compatibility remains. No purge or permanent allocation.
- Fixed pinned Multiversal PurgeSpace A062 glue: A162 must return A0. The original
  guest result incorrectly showed an address as contiguous capacity. Corrected
  Snow sample: free 398310 B, largest 398248 B, after-purge largest 398256 B.
- Host stream boundaries/access modes, shell restoration, six dash command-v
  comparisons, free formatting, 39 shell cases, 11 stream and 65 installer cases
  pass. Guest 23 cases, corrected memory query, six repeat pairs and actual
  infinite-zero cancellation/recovery pass (130). Repeat free heap settles at
  399858/399860 B; largest block stays 399630 B; tracked remaining always zero.
- Physical SE remains off. This is fixed accelerated Snow, reported 6.0.7.
  The second batch is completed in 1549 above; exhaustive option combinations
  and consolidated physical acceptance are not claimed.

## Utility expansion — first checkpoint, build 2026111547

- All 38 bundled command names, 29 builtin help topics and experimental nano
  actually executed their help in Snow: success, stdout, no stderr. Clarice help
  fits 72 columns; upstream nano retains its 80-column help. This verifies basic
  coverage/format, not exhaustive option conformance for every command.
- Corrected stale read/export/unset/printf/sh/test/preferences descriptions;
  expanded appinstall and modified upstream help. Native usage errors now use
  stderr; awk rejects unknown options instead of warning and continuing.
- Genuine pinned sbase cmp/cksum, independently bundled. Binary data forks only;
  POSIX checksum is corruption detection, not authentication. Host 1 MiB reference
  comparisons, ASan/UBSan and 16 runtime failure/cancel/cleanup trials pass.
- Snow: 16 cases include binary differences, stdin, pipeline CRC, empty/missing
  files and invalid options. Twelve repeated cmp/cksum pairs finish with zero
  tracked allocations. Free heap 376836 -> 376804 B; largest block 295896 ->
  295914 B. The 32-byte difference is recorded, not asserted to be a leak or
  proof of perfectly stable native heap. No permanent allocation introduced.
- The following 1548 batch adds virtual null/zero, command -v and free.
  Comprehensive help review and the second utility batch remain.
  New-command cancellation/failure tests are host-based; hardware acceptance is
  deferred. Editor, loader, safe-save and existing utility work preserved.

[Evidence, inventory and reproduction](utility-expansion/RESULTS.md).

Validated checkpoint **33f0075fb14385cd82d792f611261313b60e1583** on
`codex/se-alignment-2026111521` is preserved. Renderer and UX work through 1533 is checkpointed on
`codex/se-ux-2026111533`, inheriting Research Before Reinvention from 12ccb4a.
No tag or release. Physical loader profiling has resumed; build 1534 changes only
the approved installed-fast metadata validation path, not nano.

## Focused utility close-out — build 2026111546

- Startup profiles preserve their result, including Ctrl+C=130. Source execution
  also propagates exit/return/cancellation from loop conditions before restoring
  outer state. Host success/error/return/OOM, 80 cancellation points and recovery;
  Snow startup and ordinary foreground cancellation both return 130.
- `find` repeated predicates now combine with AND; added small `-path` filter and
  multiple roots using borrowed arguments. No expression tree, persistent cache
  or new working allocation. Host depth/failure/cancel tests and real Snow
  quoted-path/pipeline/cleanup workflow pass against dash/macOS reference output.
- `ls`/`df` unchecked 32-pointer arrays were a real stack-overwrite risk after
  shell argument growth. Replaced with allocation-free argv rescanning. ASan
  reproduced the old overflow; 130 host and 100 Snow operands each pass.
- Three final find runs: free heap **396578 / 396576 / 396578 B**, largest block
  **393318 B**, tracked remaining **0**. No persistent allocation added. Catalog
  order and final-component-only shell globbing remain documented limitations.
- Host 31 basic/3 cancel, 54/39/41 comparisons, 12 scripts, 3090 parser/adapter
  recovery trials and six upstream-script variants pass. Editor environment
  failure/cleanup regression remains passing. No new physical acceptance.
- Reviewed active bounds: sort's 16 merge levels/20 temporary slots, formatter
  510-byte format strings, and canonical terminal input 255 bytes remain explicit.
  Nano uses raw input; these are not new line-record or exported-value limits.
  No demonstrated ordinary-workflow failure justified changing them this batch.

[Evidence and reproduction](utility-closeout/RESULTS.md),
[compatibility](utility-correctness/COMPATIBILITY.md).

## Native heap and editor environment — build 2026111545

- Root cause: upstream awk overwrites argv entries. Passing the shell-owned vector
  lost its program-string allocation and caused a duplicate free attempt. CLX now
  lends a temporary pointer-vector copy, released on normal/error/cancel return.
  No checksum/ABI or allocator redesign; no permanent memory added. Two-argument
  commands require 12 transient bytes plus native block overhead.
- Before: native heap walk gained one pointer block per run, handles stayed flat;
  compaction did not recover it. Fixed matched new awk settles at 362452 B free;
  ten empty runs remain 362352 B. Four final walks retain 769 pointer / 36 handle
  blocks. Native stdio's previously measured reusable pool remains unchanged.
- Editor exports now grow on demand, including values >1024 B; TERM=clarice and
  private-variable exclusion preserved. Host 80 x 4096 B, 88 import-failure sites,
  cancellation and repeats pass. Snow 40 x 2048 B: five successes, free heap
  309742/309744 B, largest 306934 B, owned peak 97220 B, remaining=0. Three injected
  import failures return 2 and recover with stable heap and explicit diagnostics.
- Existing nano/ncurses objects relinked with only the terminal adapter recompiled.
  Six launches, disposable save/reopen. Matched three-run loop: free heap
  399650/399658/399658 B; largest 236688 B; owned peak 87684 B, remaining=0.
  30 exports and a 1500-byte value present. Canonical 255-byte input bound unchanged.
- Host 54/39/41 shell comparisons, 12 scripts, 31 basic/3 cancel, 9874 parser and
  3090 scope fault trials pass. Loader mutation/error/cancel/admission tests pass.
  Six upstream-script guest variants still match dash exactly. Native awk heap
  exhaustion fails explicitly and recovers; 1000 entries exceed this fixture's
  available heap, so that workload is not labelled successful. Ordinary Ctrl+C
  returns 130, skips the next command and restores streams; a following 500-entry
  awk array returns 0 with remaining=0.
- Normal MacBinary 957952 B (+256); experimental nano 268212 B (+212), separately
  installed. Local build artifacts excluded from Git; no physical-SE acceptance.

[Evidence, limits and reproduction](heap-audit/RESULTS.md),
[compatibility](utility-correctness/COMPATIBILITY.md).

## Practical scripts and environment audit — build 2026111544

- Syntax cleanup no longer overwrites existing errors with OOM. Seven malformed
  quote/parameter/compound cases recover with status 2; 128 injected failures on
  those same inputs still report genuine OOM, free allocations and parse again.
- Three complete unchanged upstream scripts (musl specs, ncurses 6.5 definition
  and parameter generators), six default/space-path variants, match dash exactly
  on host and native guest. Full upstream Caps produces 414 entries. The default
  ncurses path exposed `set - Caps`; ordinary positional set and set-minus forms
  now work. No Unix build-system, trap/PID or echo-extension compatibility implied.
- awk previously dropped exports beyond 18 entries or its 164-byte buffer. Rebuilt
  standalone awk now imports on demand through existing invocation allocation.
  Host: 30 x 4096-byte exports, low-memory failures, cancellation before execution,
  remaining=0. Snow: 30 x 512-byte exports, three successful repeats, owned peak
  67723 B, remaining=0. No idle reservation or runtime ABI change.
- The subsequently isolated argv-ownership leak and editor environment limits
  are corrected in 1545 (above); original 1544 evidence remains historical.
- Retained host 54 POSIX, 39 focused, 41 elastic and basic/cancellation cases pass;
  9874 parser and 3090 scope failure positions pass. Native Ctrl+C returns 130,
  skips the following command and restores stdout/stderr. Guest runs fixed max
  FFWD (~15–16×); synthetic entry briefly normal speed. Normal MacBinary 957696 B
  (+128 B), separate awk.clx 188460 B. Existing utility/editor work is preserved.

[Practical results/provenance](shell-practical/RESULTS.md),
[guest evidence](shell-practical/guest.json),
[memory comparison](shell-practical/memory-comparison.json),
[compatibility matrix](utility-correctness/COMPATIBILITY.md).

## Parser safety and elastic capacity — 16 November

- Fixed the reproduced parser-constructor NULL dereference and unchecked AST,
  token-buffer, heredoc, arithmetic and partial-copy failures. Explicit ownership
  cleanup preserves existing functions and restores shell streams/state. Uses
  mrsh's existing AST/destructors; no parser rewrite or region allocator.
- Removed source-size, total-token, function-count/name and source-chunk ceilings.
  Storage follows available heap. Explicit source/execution (8/16), parser (32)
  and deep expression-chain (64) stack guards remain. Interactive input is still
  1023 bytes; completion safely declines names outside its existing fixed buffer.
- Host: 9,874 parser, 8,268 AST-copy, 660 arithmetic and 1,268 whole-shell injected
  allocation positions; single failure and continued exhaustion, zero retained
  tracked bytes, subsequent command recovery. Existing 39 focused, 12 whole and
  41 elastic cases pass, as do 31 comparisons / 3 cancellation cases and 329
  adapter allocation failures. Long-name completion passes sanitizer checks.
- Snow diagnostic run: all 39/12/41 cases, eight capacity/stack-guard cases and
  282 actual injected failures pass; zero retained parser bytes after each trial,
  pre-existing function remains callable. Diagnostics are excluded normally.
- Normal Snow build: 32 KiB source, 600 simple commands, 48 long-named functions,
  200 arguments; three actual heap-exhaustion attempts return 2 with an error,
  execute no partial program and recover. Free heap 411350 → 411360 → 411360 B.
  Actual Ctrl+C returns 130, skips the next command and restores both output
  streams; portable remaining=0. Fixed uncapped FFWD about 15–16×.
- Ordinary host adapter `echo hello` remains six allocations / 259-byte peak /
  zero retained bytes. No permanent cache or reservation. Parser checking adds
  host CPU cost (recorded, not a physical-performance claim). Very large source
  parsing still shifts its read buffer repeatedly and does not poll inside lexical
  scanning; large-script throughput remains a limitation.
- Earlier elastic variables/arguments/read/substitution work and utility/editor
  changes are preserved. The following batch extends quoted `$@`, export visibility,
  printf and test. No claim of full POSIX conformance.

Evidence and reproduction: [parser results](parser-safety/RESULTS.md),
[diagnostic guest](parser-safety/guest-diagnostic.json),
[normal guest](parser-safety/guest-normal.json),
[compatibility matrix](utility-correctness/COMPATIBILITY.md).

## POSIX expansion, scope and formatting — build 2026111543

- Quoted `$@` preserves vector boundaries, empty arguments and prefixes/suffixes;
  quote-group handling resolves the empty `"$@$@"` dash discrepancy. Braced special
  parameters now parse. Default/alternate expansion retains nested quoted fields.
- Export flags distinguish local, exported and exported-but-unset variables;
  preserve flags on reassignment, clone and restoration. Child sh drops locals and
  functions; functions/sourced scripts share variable changes, subshells isolate
  them. No-argument dot correctly shares changes made by `set --`. Native external
  commands receive only exported entries, plus existing HOME/PWD defaults.
- printf streams integer/string/character/escape conversions with literal width,
  precision, format reuse and flags. No width-sized allocation; 64-byte temporary
  output blocks and Ctrl+C polling. `test`/`[` gain grouping/negation and checked
  integer operands, retaining arity-sensitive operand interpretation.
- Host: all 54 new dash comparisons, 39 focused, 12 complete, 41 elastic cases,
  31 existing comparisons/3 cancellation cases pass. 3,090 new whole-shell fault
  trials restore streams, preserve the seeded function/variable and leave zero
  tracked bytes; parser/AST sweeps and 329 adapter failure trials still pass.
- Normal Snow build at fixed uncapped FFWD (~15–16×): all 54 new and 39/12/41
  retained cases pass, plus native awk export visibility. Actual Ctrl+C interrupts
  billion-column printf with 130, skips BAD and restores stdout/stderr. Three
  repeats: heap 314612 / 314612 / 314608 B; largest block 225530 B throughout;
  portable remaining=0. These are script-context measurements after running awk,
  not directly comparable to the previous idle parser-only fixture.
- Small host `echo hello`: six allocations, 267-byte peak (previous 259), zero
  retained; quote position adds transient word state and export flags add per-variable
  state. No permanent cache or reservation. Normal MacBinary 957568 B (+4224).
- Remaining: full POSIX certification is not claimed. Floating printf and dynamic
  width, legacy test -a/-o, Unix permission predicates, complex vector parameter
  operations, shell options and the documented native path/terminal limits remain.
  Large-source lexical cancellation/throughput limitations are unchanged.

[Results and reproduction](shell-posix2/RESULTS.md),
[guest evidence](shell-posix2/guest.json),
[compatibility matrix](utility-correctness/COMPATIBILITY.md).

## Portable scripts and compound redirection — 16 November

- Fixed the imported parser's missing compound redirect-list support: redirection
  now covers the entire group/loop/subshell/if/case, through the existing shell I/O
  owner. Restores previous streams after completion/error/control flow/cancellation.
  Redirect lists are owned/copied/freed with their AST. Failed copies reject and
  free partial trees and retain existing function definitions.
- Host: 12 complete scripts match dash, plus existing 31 comparisons/3 cancellation
  checks and 39 focused cases. Eleven scope/failure/cancellation checks pass with
  no open native references; 18 allocation-position trials exercise the added
  parser/copy sites (10 reached injections, 8 beyond-allocation controls).
- Snow Plus/4 MB, reported System 6.0.7: all 12 script outputs match, using actual
  CLX utilities; od values compared after portable whitespace normalisation.
  Fresh native metadata/locked-file/listing/df checks and all 34 external --help
  exits pass. Repeated scope/here-document/read batches: heap **399,996 B** and
  largest block **375,926 B**, identical across three runs; portable remaining=0.
- Real guest Ctrl+C interrupts a redirected infinite loop with **130**, skips its
  following command, restores stdout/stderr and permits subsequent file writing.
  Functional runs at fixed uncapped FFWD (~15.6–16×); briefly normal speed for
  synthetic keyboard setup only. No physical results inferred.
- Memory cost: one 4-byte optional pointer per parsed command, redirect storage
  only when used, one 4-byte copy-failure generation counter; no new permanent heap
  allocation. Scoped execution reuses the existing simple-command frame, including
  its temporary ~4.14 KiB argument structure; not a new stream implementation.
- Shared newlib 4.6.0 retains **2,180 B** after the 32-stream probe and reuses all
  32 FILE addresses on two repeats. This is useful stdio capacity, not automatically
  reclaimable general heap. No libc change justified or made.
- Complete declared-utility acceptance inventory and supported/partial/unsupported/
  platform-limited shell matrix: [compatibility](utility-correctness/COMPATIBILITY.md).
  Capacity limits, function-definition redirects, full export/subshell semantics,
  complete printf/read remain gaps; parser OOM hardening is completed above. No POSIX claim.

[Evidence](utility-correctness/RESULTS.md), [libc finding](utility-correctness/LIBC-POOL.md).
Local normal app: `outputs/utility-correctness/build-1540/Clarice.bin` (diagnostics
and nano profiling OFF). Bundled resources remain separate from independently
rebuilt/tested utility candidates; startup does not replace existing installations.
The pushed checkpoint excludes generated packages, raw control sessions, private
preferences, legacy build-log noise and disposable large-output fixtures. All
existing branches/tags/releases preserved; subsequent task changes remain local.

## Utility elasticity and small-command audit — 16 November

Experimental normal build **2026111538** and independently built awk/sort are
host/Snow validated; no physical deployment or new cache. Full inventory and
reproduction evidence: [utility audit](utility-audit/AUDIT.md).

- Actual installed ls: 18 Snow invocations (help, empty and ten-entry directory).
  Warm loader median **40 ticks / 0.67 s**, of which reading is 13–14 ticks,
  receipt work 16, preparation 8, imports/relocations about 1–2 each. First load
  53 ticks. Reading does not dominate; retain on-demand CLX, no caching/builtin
  conversion. These module timings exclude shell parsing and final prompt paint.
- Removed Clarice's artificial 16 KiB awk/sort record ceilings using upstream
  growth and existing spill merging. Small sort batches remain bounded; large
  records transfer buffer ownership instead of allocating a duplicate.
- Shell read now consumes an oversized record, reports failure and preserves its
  variable instead of silently succeeding with a truncated value. Exact 127-byte
  records and subsequent reads work; no new permanent allocation.
- Host: 37 elastic-text cases, 30 awk comparisons, 45 tail/sort comparisons,
  spill/error/cancellation tests, 31 shell comparisons and focused read boundaries.
  Snow: exact 64 KiB records, multi-run sort/unique, genuine heap exhaustion,
  recovery, Ctrl+C status 130 and zero command-owned survivors. Small sort peak
  4,492 bytes; 64 KiB sort 208,408; awk length 152,101. Large fields still exhaust
  the available application heap explicitly. No claim that every command is elastic.
- Rebuilt grep/sed: accelerated Snow now verifies exact 64 KiB grep and single
  substitution output, 32 KiB global substitution, and subsequent small-command
  recovery. Sed no longer scans beyond the requested positive occurrence or
  repeatedly scans the growing substitution destination. Host: 35 comparisons
  against pristine upstream, allocation failure and cancellation cleanup pass.
  Global 64 KiB substitution still exhausts this guest's available command heap:
  explicit status 1, no output and zero portable-owned survivors; not silent success.
- find/du now use demand-allocated iterative traversal instead of depth 8.
  Host depth-24, allocation-failure-at-each-frame and cancellation tests pass;
  Snow verifies 26 entries, explicit maxdepth 12, and expected allocated-space
  total. Native path/catalog limits remain. cp/rm now also use iterative traversal;
  depth-24 guest copy/removal, allocation failure, cancellation and recovery pass.
- Test at fixed uncapped FFWD, dynamic acceleration disabled. Startup sometimes
  stays around 1.5× until FFWD is toggled off/on; verified runs show 15.1–15.4×.
  Use prepared guest startup scripts: current automated key/clipboard delivery
  is unreliable at this speed. This is not a demonstrated Clarice paste defect.
- Disposable-volume exports are reliable after explicit native flush/unmount.
  Earlier host-emulator termination without guest unmount lost catalog changes
  despite a name-based FlushVol success. Cause is not established; preserved
  evidence must not be treated as successful byte verification. No production
  filesystem changes made to compensate for the test shutdown problem.
- Guest reports **System 6.0.7**, Plus/4 MB; physical SE/6.0.8 acceptance pending.
  Focused results: [utility correctness evidence](utility-correctness/RESULTS.md).
  Native-error descriptions and inverse pinwheel are now guest-validated below.
  Stream/descriptor elasticity and cp/rm traversal are validated. The broader
  utility/POSIX audit remains incremental, not a full compliance claim.

## cp/rm and portable streams — 16 November

- Removed depth-8 cp/rm restriction with explicit demand-allocated DFS frames.
  Both-fork replacement remains unchanged. Partial directory trees after failure
  are reported, not rolled back as a whole; source files remain intact.
- Removed 12-entry portable stream/descriptor ceilings. Stable stream cookies grow
  individually; descriptors grow on demand and reuse closed numbers. Standard
  streams remain small/static. Command-owned records/buffers are all reclaimed.
- Host ASan/UBSan: depth 24, 25 copy allocation boundaries, removal allocation
  failure, cancellation and cleanup; 40 simultaneous streams plus 40 descriptors,
  fdopen ownership, repeated invocations, low-memory and cancellation checks with
  native handle counts restored. Existing 30 awk / 19 sed comparisons pass.
- Snow fixed uncapped FFWD (~14–15.5×): deep cp/rm and fault injection pass;
  failed/cancelled overwrite preserves both forks and Finder metadata; no leftover
  copy temporary siblings. 16 streams and 16 descriptors separately pass three
  times, as do injected runtime OOM/cancellation and subsequent recovery.
  Real upstream awk writes 16 simultaneously open files, repeated; all bytes exact.
- Native combined capacity test reaches EMFILE after 16 streams plus 11 descriptors
  in this guest: genuine system-wide open-file capacity remains. All portable
  survivors zero. Stream test peak 13,816 B; repeated free heap 397,948 B unchanged.
  Shared newlib FILE records retain roughly 2.10 KiB after first larger use; largest
  free block can shrink from allocation placement (first stress: 388,476→364,500 B).
  This is libc high-water storage, not a new Clarice resident cache; no repeated loss.
- Normal external packages prepared locally: cp/rm 12,840 B each (+708), awk
  188,312 B (+532). Other installed/bundled commands are not silently updated;
  they acquire the new runtime when rebuilt. No application/SE update, push or release.
  [Evidence and commands](utility-correctness/RESULTS.md); guest remains System
  6.0.7/Plus/4 MB. Physical SE and 6.0.8 acceptance remain pending.

## Native errors, activity display and POSIX checks — 16 November

Normal local application **2026111539** passes Snow Plus/4 MB/System 6.0.7
at fixed uncapped FFWD (~15.6×). SE remains untouched; no push or publication.

- Native error reporting retains numeric codes and adds shared descriptions;
  missing-file, disk-full, memory and unknown-code fixtures pass, as does actual
  cat failure. Unknown errors are not guessed; upstream Unix errno remains separate.
- Pinwheel CopyBits now uses neutral port colours for exact pixel transfer.
  Native QuickDraw tests pass all four frames, both themes and both destination
  polarities, with exact restoration after cancellation. Free heap stays 422,922 B;
  no additional bitmap or permanent allocation. Clock/catch-up tests pass.
- Shell fixes: IFS non-whitespace empty fields, quoted `$*` joining, substitution
  status for assignment-only commands, sh -c positional arguments, read -r/escape
  handling and IFS trimming. Temporary assignments preserve unrelated builtin
  effects. Two small mrsh parser fixes preserve empty assignment boundaries and
  prevent `$#` following another expansion being swallowed as a comment.
- Host ASan/UBSan: existing 31 shell comparisons + 3 cancellations, read overflow
  boundaries, and 39 new dash comparisons pass. The same 39 cases pass byte-for-byte
  in eight Snow batches. Existing pipeline, spilling-sort, cancellation and
  output-failure checks pass against the current runtime.
- Shell remains bounded (127-byte variables, 32 arguments, 8 KiB source, 256-token
  guard); read supports one variable. Embedded quoted `$@`, broader parameter
  grammar and other documented shell gaps remain. No general POSIX compliance claim.
  These changes add two scalar substitution counters (8 B in the 68k build),
  temporary read quote markers and no resident cache. Full shell memory/latency
  profiling and physical/System 6.0.8 acceptance are not claimed.

[Tests, references and evidence](utility-correctness/RESULTS.md).
Normal MacBinary: `outputs/utility-correctness/build/Clarice.bin`, with SHA256SUMS;
activity diagnostics and nano profiling are OFF. Independent utility candidates
remain separately packaged; the normal application does not silently replace them.
Next: continue capability/failure coverage and remaining artificial shell limits,
then consolidated physical acceptance when authorised.

## Physical SE screensaver — 16 November

The user completed After Dark/Flying Toasters installation and shut down the SE.
User reports Flying Toasters activates after **10 minutes of inactivity**. Before
future SEControl screenshots or keyboard input, send a mouse movement to wake the
display, allow it to restore, then capture/check the foreground application before
typing. Screensaver activation was not independently verified remotely.
Clarice wget successfully transferred the control panel and module beforehand;
no Clarice production changes were needed. Leave hardware off until available.

## Nano post-loader experiment — 16 November

Keep the existing strip renderer; omit trailing-space drawing after full-row
clearing and render directly at the destination bit phase where the same buffer
fits. QuickDraw clipping, inverse spans and cursor behaviour remain. No additional
bitmap or permanent allocation. The shared newlib format adapter now covers nano's
vsnprintf/sprintf paths: `Read 128 lines`, not `Read zu lines`.

Matched physical warm startup, measured internally to painted input readiness
(excludes screenshot overhead): empty **5.68 → 5.10 s**, 8 KiB/128 lines
**9.19 → 8.57 s**, 8 KiB/two long lines **7.34 → 6.88 s**. Two runs per case;
portable remaining=0 throughout, peaks unchanged. Drawing totals respectively
62–65 → 34–35, 103–105 → 69–71, 69–71 → 39–42 ticks. Physical editor-body,
shortcut-bar and scrollbar pixels match exactly (corrected status line excluded).
Normal nano image grows 196 bytes; no new idle heap reservation. Instrumented app
free heap is stable at 418,906 bytes versus prior 419,010, not accumulating per run.

Host dirty-buffer/clip/theme/cursor checks and safe-save fault injection pass.
Snow normal nano repeated saves preserve exact data, resource fork, type/creator,
flags and creation date; cancellation returns 130 without saving, remaining=0.
Normal 1537 opens/exits nano successfully. Physical normal nano installed with prior command retained; new-file editing,
save, readback and exit passed with remaining=0. Clarice was then closed for the
night; no additional hardware work is running.
No high-speed physical flicker capture or fresh matched cold run this evening.

Remaining priorities: short-file open/construction ~2.8 s (native reads only
6–10 ticks), final ncurses refresh ~1.65 s including output/rendering, warm CLX
~1.42 s. Counters overlap; do not sum. Cold preparation/receipt policy unchanged.
Three-second target remains unmet. Hardware work is stopped for tonight; no further renderer campaign. [Evidence and methods](nano-profile2/README.md).

## Executable loading — accepted focused experiment, 16 November

Accepted checkpoint `2e84229` was pushed to `codex/clx-fast-2026111534` on
16 November, preserving AGENTS `43eb445` and checkpoint `7eb7064`. Main and
releases were not changed. Build **2026111534**
retains receipt-gated installed-fast loading but omits its repeated metadata hash.
One cancellation poll and all structural/import/relocation bounds remain. Full
installation/repair/check/strict and unknown/changed-file verification remain.
No executable-format change, receipt removal, permanent allocation or nano change.

Physical stock SE loader: cold **356 → 215 ticks (5.93 → 3.58 s)**; warm **190 →
85/86 (3.17 → 1.42/1.43 s)**. Fresh follow-up cold 222, warm 87 ticks. The removed
metadata/event interval accounts for the improvement; preparation remains 132–139
cold versus 3–4 warm. Receipt work remains about 15–16 ticks (~0.25 s).
Host 64 scenarios pass, including structural corruption, strict/check rejection,
revocation, replacement/recovery and cancellation. Snow disposable unchanged-identity
metadata corruption passes only in fast mode, then fails strict/check and succeeds
after explicit replacement. This intentional residual corruption risk is documented.
No retained invocation allocations in repeated physical nano and native-probe runs.
[Acceptance and evidence](loader-fast2/ACCEPTANCE.md).

Whole-nano physical observations include first rendering, not just loading:
complete-screen capture upper bounds **12.26 s cold empty, 9.35 s warm empty,
19.88 s warm 8 KiB/126 lines**. Input was confirmed after complete rendering
and test edits discarded. Wi-Fi/capture overhead is included; these are not exact
internal startup marks. Cold 8 KiB remains unmeasured here. `wc` subsequently confirmed the historical
fixture has 126 lines; its previous two-long-lines label was erroneous. The three-second editor target remains unmet. Earlier partial
frames must not be counted as complete startup. Both SE and Snow greetings report
System 6.0.7; do not claim independent 6.0.8 acceptance from these captures.

Minimal returning native APPL comparison completed with the same bounded argument,
stdout/stderr/status body. Snow CLX **25/27/27 ticks**, native through driver
**126/127/126**. Physical CLX **34/34/33**, native **157/155/155**. Native includes
file exchange, separate 160 KiB partition, actual launch, execution and return;
module timing excludes final prompt painting. Both routes return status 7 and
recover memory. This small-command result does not establish a large-app or nano
advantage. Keep CLX; no production process replacement is justified by this probe.
[Methods and limits](native-bench/RESULTS.md).

Receipts are useful only for the current checked-versus-unknown launch policy;
staging and retained backups provide recovery. Recommend policy review before
removing receipts, not another trust database. Cold NewPtr/image/metadata/stdio
preparation is the next loader measurement; do not hide it with permanent caches.
[Native-first and receipt assessment](loader-review/SIMPLIFICATION.md).

Runtime allocation already adapts to available application heap minus 32 KiB
headroom; there is no current 96 KiB command ceiling. Small 2 KiB pages and direct
allocations are invocation-owned. Genuine limits include the 768 KiB partition,
contiguous space and 64 KiB stack. Separate loader limits (256 KiB image, 8192
relocations, 128 imports) need compatibility/overflow review before changing.
Initial utility limits: FILE/FD tables 12, temporaries 20, recursive traversal depth
8; inventory is preliminary, with no widespread utility rewrites authorised.

Deployment issue: Finder was scrolled past the current app and contained many prior
`.cu-*` copies. Remote inspection located build 1534 as **Clarice Updatastute** on
Macintosh HD and confirmed its greeting before measurements. Older copies were
opened during identification; those are not benchmark evidence. No application
contents or backups were deleted. Owner requests one clearly named current app,
identifiable recovery and safe staging cleanup. Updating an existing copy is already
the intended workflow; broader cleanup remains future updater work. No push/release.

## Terminal UX and capabilities — 15 November 2026

- Configurable physical Control/Option/Command, CLI and Preferences popup;
  physical Control always works. Macintosh Command shortcuts retain precedence;
  Command+Option sends conflicting controls in Command mode. Native KCHR mapping
  is centralised; editor control bytes no longer collide with arrow encoding,
  including input queued during loading. Queue overflow rejects without partial
  delivery; actual native queued Ctrl-underscore opens nano's Go to Line.
- Greeting reports native hardware/build details. Allocation-free runtime 1.3
  `cli_hardware` provides a reusable versioned snapshot; older commands retain
  compatibility. Plus/SE API reports System 6.0.7 despite installed 6.0.8, labelled
  as reported. Later CPU/FPU/colour-machine qualification remains outstanding.
- Full historical viewport with no pinned prompt; typing/paste returns live and
  retains the trigger. Themes repaint immediately, preserving input/history.
  Preferences and Command mode/dark theme survive Snow application restart.
- Visible update `[Y/n]`, Enter accepts, n cancels, `-y` skips only confirmation;
  warning explains only Clarice closes/relaunches. Existing verification/recovery
  unchanged. Shared width-aware progress has current/total IEC sizes, percentage,
  speed, unknown totals, narrow-window fallbacks and one-second throttling.
  Explicit 32-bit arithmetic tests cover counters through 4 GiB; no huge-file
  transfer claim. Native Plus and SE counter probes pass.
- Host sanitised input/preferences/progress/HTTP checks and 61 loader/installer
  scenarios pass. Snow UX probe zero failures; 322 clipping/pixel comparisons pass;
  both themes and normal post-nano scrolling: zero text/bar mismatches. Option-C
  returns 130; Command-X exits genuine nano. Kermit binary round trips exact;
  cancellation/recovery work, owned remainder zero. Normal 1529 → 1531 `-y`
  update automatically relaunched; prior app and Finder metadata verified.
  Diagnostic → normal update reproduced historical error 28; cause still unknown.
  Physical 1525 → 1531 with Enter and 1531 → 1533 with `-y` relaunched normally.
- Plus capability query ~22 ms, no retained heap; normal launch-to-ready counter
  318 ticks (5.3 s; excludes time before main/first completed paint). Resource fork
  +2,291 bytes over 1525 in final 1533; no new bitmap, partition or stack increase.
  Snow dense history 336.8 ms versus historical 327.1, now drawing an extra historical row;
  not a matched same-workload renderer comparison. Physical wget 64 KiB old 347,
  new 352/322 ticks: no demonstrated throughput regression from progress display.
- Physical controls, Command-C/V, both immediate themes, historical viewport and
  retained triggering keystroke pass. Output interruption returns 130; both themes
  pass 22 bursts + 20 views with zero text mismatches and 147/149 clean scrollbar
  checks. Capability query ~26 ms, retained allocation zero; model/CPU/FPU/RAM/
  monochrome display match this SE. Later CPU/FPU/colour hardware remains pending.
- Physical new command images installed/validated with backups retained. Update
  warning/Enter tested; staging 72.4 s, verification 33.8 s. Owner reported ~4 min
  for Clarice to return during the first update; SEControl lost contact and was
  restarted manually. Final `-y` update capture confirms 1533 without another
  helper restart; coarse polling does not establish exact relaunch duration.
  Update handoff remains slow. [Evidence and limits](ux-six/REPORT.md).
- Final 1533 physical nano: queued Ctrl-underscore opens Go to Line; Ctrl-C
  cancels it; Option-X exits. In Command mode, 8 KiB file/search/Command-X pass.
  Three runs return to **420,960 bytes free heap**, owned remainder zero; peak
  portable allocation 74,572 empty / 101,736 populated. No editor save changes.
- Resumed physical loader profiling: cold empty **356 ticks / 5.93 s**, first
  and repeated 8 KiB **190 ticks / 3.17 s** each. Cold preparation 139 ticks falls
  to 4/5; warm metadata validation 108/107 dominates, followed by reading 38/35,
  relocations 22, record lookup 14/16 and imports 4 ticks. Full payload verification
  is already skipped for this recognised installed image. These are loader times,
  **not submission-to-editable-screen timings**; the three-second total target is
  still unmet. Next focus: physical allocation/preparation and metadata-pass cost,
  preserving current verification policy; no speculative renderer or nano rewrite.

## Historical terminal graphics investigation — 15 November 2026

- Inspected original NCSA Telnet 2.7b5 and BetterTelnet 2.0fc1 source archives,
  plus pinned Flynn source; attribution, licences and source hashes recorded.
  No third-party code copied into Clarice. No runtime changes, build or deployment.
- Native text runs, pixel scrolling, dirty tracking, glyph reuse and output
  batching largely confirm techniques already used. Flynn's retained full
  bitmap/shadow costs more RAM; its glyph loop offers no demonstrated advantage
  for large dense history jumps. Direct visible erase/draw would compromise
  completed-row presentation. No suitable low-risk experiment justified.
- **1525 remains unchanged**, with previous physical dense redraw 416 ms and
  glyph work approximately 253 ms. No new application performance or regression
  results claimed. Prioritise physical loading/nano profiling; optional dark
  inversion or sparse-glyph ideas first need targeted cost evidence.
  [Investigation and ranked opportunities](terminal-history/REPORT.md).

## Direct aligned output — 15 November 2026

- Accepted baseline measurements attributed about **98 ms/frame** to a separate
  row-alignment pass. Eligible Monaco 9 ASCII history spans now compose directly
  at the required bit phase in the same 4096-byte buffer. No additional bitmap,
  persistent allocation, renderer preference, cursor or wider renderer change.
  QuickDraw clipping and existing short-span/font/selection fallbacks remain.
- Three alternating pairs, 24 completed large-jump redraws each, 80×24/light/128
  history lines; quiet SEControl. Physical dense **678 → 599 ticks**, mean frame
  **470.8 → 416.0 ms (11.7%)**. All pairs improve: 678/599, 679/601, 678/598.
  Directory-style **264 → 265 ticks**, effectively unchanged (183.3 → 184.0 ms).
  Snow dense **579 → 471 ticks (18.7%)**; directory **228 → 228**. Paired heap
  delta zero. These are complete redraw timings, not new `ls` throughput results.
- Across all matched draws Snow is about **20% faster** than the SE; the retained
  path ranges from 16% to 27% faster by workload. SE delivers 79–86% of Snow's
  speed here. The former provisional 60% assumption is not supported for these
  workloads; do not generalise this ratio to disk I/O, loading or nano.
- Physical dense presentation **158 → 61 ms/frame**; alignment **97 → 0.7 ms**,
  the residual including checks/timing overhead. Glyph work increases **204 →
  253 ms**, consuming about half the avoided pass. Text/glyph work now accounts
  for about 61% of the 416 ms total. Counters are coarse and nested; medians do
  not sum exactly. Dense redraws remain expensive.
- Host: 1,339 actual M68000 direct-composition cases with bounded/even accesses
  and read-only glyph storage; 8,034 glyph/phase/theme cases; 356 existing
  compositor/ABI cases. Existing tail-clearing, selection and scroll guards pass.
  An exactly fitting tail's unnecessary zero-byte write was corrected before
  physical installation; a strict write guard now catches it.
- Snow and physical diagnostic: **322 dirty-buffer/clipping comparisons** pass,
  including actual occlusion, heap delta zero. Each theme passes 22 bursts + 20
  views, zero text mismatch and 147 clean scrollbar checks. Real selection,
  dragging and output/pager Ctrl+C (130) pass on both machines. User watched SE
  rapid dragging: **no blanking or missing scrollbar**. Snow's 80-sample drag
  captured no transient blank bands; sub-sample flashes are not excluded.
- Genuine nano opened the 8 KB fixture and exited in Snow; subsequent native
  pixel/bar checks pass, owned remainder zero. No new nano total-startup,
  safe-save or physical nano timing claim. Normal 1525 native pixel/bar checks
  pass in Snow and on SE; normal output/pager Ctrl+C returns 130 on both.
- Normal resource fork **940,593 bytes, +556 over 1521**; no additional persistent
  buffers, partition (768 KiB) and stack reserve (64 KiB) unchanged. Code growth
  is a small memory cost, not a new cache. Final physical free heap **414.06 KB**
  (423,998 bytes), 558 bytes below 1521; owned remainder zero. Retained recovery
  applications include the 1521 baseline; no rollback rerun claimed.
- **Recommended next priority:** current physical command-loading and nano-startup
  profiling before more renderer work. Latest controlled Snow startup is 7.23 s
  empty / 10.08 s for 8 KB. This task's cold nano loader alone is **4.85 s**:
  read 61, validation 85, imports 4, relocations 19, preparation 110 ticks.
  Investigate cold allocation/preparation and metadata work first; full payload
  checksum is already skipped for verified images. No loader redesign begun.
- Earlier accepted glyph reuse, redundant-clear removal and row alignment remain.
  [1517](jump-perf/REPORT.md), [1519](clear-perf/REPORT.md), [1521](align-perf/REPORT.md).
  Their separate diagnostic binaries are not interchangeable timing baselines.
  Dark-to-light invalidation and updater `Y/n`/`-y` are addressed in the UX work above.
  [Current report](direct-perf/REPORT.md), [measurements](direct-perf/measurements.json).

## Listing throughput — 15 November 2026

- Build **2026111514** removes temporary numeric formatting/copy work and uses
  bounded 68000 printable-run copying and aligned row clearing. No HFS changes,
  new permanent cache, partition increase or drawing/cursor redesign.
- Controlled alternating old/new Snow runs, real `ls -la` through final prompt:
  **1,000 files 10.18 → 9.62 s (5.6%)**; 100 files 2.60 → 2.55 s. Three-run
  medians; equal-depth command paths, same session/dataset/geometry. Module median
  562 → 529 ticks; paint 83 → 82; seven large-listing frames throughout.
- Chunked 96-tick bulk presentation, complete rows, immediate first/pager/final
  display and two-tick input polling remain. Earlier cadence/output work reduced
  1,000-file time 13.20 → 10.03 s; [previous evidence](chunk-perf/REPORT.md).
- Native HFS sync/async walks both measured 300 ticks, with 338 distinct catalog
  block reads; no safe speedup was demonstrated. [HFS evidence](hfs-cost/REPORT.md).
- Actual 68000 tests: 75,540 numeric fields, 8,911 copy cases, eight row clears;
  host formatter/writer/HFS/volume regressions pass. Normal guest output matches
  accepted data byte-for-byte; Ctrl+C status 130 in output and pager. Native scroll
  pixel mismatches zero (24 bursts + 20 views); 153 scrollbar checks also match.
- Normal idle free heap **416.16 KB**, 128 bytes below prior build; invocation
  owned remainder zero. Core +280 bytes; application resource +412 bytes.
  Renderer remains automatic native QuickDraw, with no renderer preferences.
- Separate-run timing shifts of about 44 ticks remain unexplained; use the
  interleaved paired result, not the fastest isolated run. Nine-plus seconds is
  still slow; timestamp reuse varies with content. No new nano measurement.
- Physical SE (CPU code 1, user-reported stock mode/4 MB/6.0.8): unchanged
  452-file directory, old build 1506 → normal 1514 listing median **8.43 → 5.47 s
  (35% faster)**. Ordinary loader/execution counter **10.03 → 6.10 s**; this excludes
  PATH/final prompt. Cumulative deployed improvements, not today's isolated gain.
  After-listing heap 415.99 KB (425,970 bytes), owned remainder zero; Ctrl+C 130 in
  output and pager. Setup was cancelled at 452 files; do not call it a 1,000-file test.
- Physical missing scrollbar section was captured in old build 1506. It has not
  reappeared in 1514 after listings, pager, thumb dragging, overlap/focus switching
  or resizing. Native check: 22 bursts + 20 views, zero pixel mismatches; 147
  scrollbar checks, zero mismatch. This is focused evidence, not a universal fix claim.
- Full power cycle restored Wi-Fi after MacTCP -23015; cause unproved. New update
  downloaded in 63 s, staged in 72.40 s, verified in 34.38 s, automatically relaunched.
  Previous app: `Macintosh HD:.cu-000001c6-78c4021a.old` (resource 939,251 bytes).
  No updater/firmware changes made. Other installed commands preserved.
  [Current report/build](output-perf/REPORT.md), [physical measurements](output-perf/physical-measurements.json).

## Implemented and tested scope

- Native System 6 shell, external `/bin` utilities, HFS `/mnt`, symlinks, PATH,
  preferences, bounded history/scrollback and finite spool pipelines. These are
  Unix-like subsets, not full POSIX compliance or live Unix processes.
  [Userland](userland-next/TEST-REPORT.md), [preferences](preferences-text/TEST-REPORT.md).
- QOL-1 cross-volume resolution, QOL-2 input batching/fail-closed overflow and
  QOL-3A changed-region selection have focused host/Plus guest evidence:
  [filesystem](qol1/TEST-REPORT.md), [input](qol2/TEST-REPORT.md),
  [selection](qol3a/TEST-REPORT.md). Physical cursor rendering is working; Snow
  artifacts alone do not justify changing it.
- Independent CLX command installation preserves existing applications at startup;
  explicit replacement/repair retains recovery and checks runtime compatibility.
  [Task 2](task2/TEST-REPORT.md), including 43/43 guest acceptance checks.
  Installation-verified fast-launch code and explicit strict verification are
  present; do not infer a new nano benchmark from host tests in clx-fast/.
- Genuine GNU nano 9.2 tiny and ncurses 6.5 edit and save in Snow. Same-volume
  staged saves preserve the resource fork and applicable Finder metadata, with
  fault/recovery tests. [Safe-save evidence](safe-save/TEST-REPORT.md).
  This is recoverable replacement, not atomic or guaranteed power-loss safety.
- Runtime allocation is demand-driven within available application heap and
  recovery headroom; the former universal 96 KiB ceiling is obsolete. Focused
  demand/exhaustion tests reached 346,080 bytes with zero retained allocations.
  Stream update/seek/error services pass focused guest tests.
  [Runtime evidence](runtime-completion/PROGRESS.md).
- Serial 57600 8N1, local PTY shell and bounded E-Kermit binary transfers have
  host/Snow coverage. [Serial-3](serial-3/TEST-REPORT.md). Physical short echo,
  reconnect and binary transfer evidence is in [hardware progress](hardware-update/PROGRESS.md);
  tested USB serial setup requires host pacing of 2 ms per byte for Kermit.
- SEControl is separate development infrastructure. Physical Wi-Fi screenshots,
  keyboard/mouse control, Finder/Clarice launch and disconnect recovery work.
  [Graphical-control report](hardware-control/graphical/REPORT.md).
- External HTTP wget uses MacTCP, bounded streaming, incomplete-file cleanup,
  errors/cancellation and progress displays. Physical CLX download, verification
  and appinstall have passed. No HTTPS/authenticated internet update claim.

## Performance and limits

- Normal application partition request is 768 KiB, with 64 KiB stack reserve.
  History payload is 16,896 bytes; normal rendering uses no bitmap cache.
  Diagnostic cached-renderer comparisons allocate 29,696 bytes temporarily.
  Command/editor working allocations are released; there is no resident nano.
- Nano's three-second target remains unmet. Latest recorded controlled repeated
  dispatch-to-ready times after small-allocation-page optimisation: empty 11.15 s,
  8 KiB 14.07 s (previous 12.17/17.68 s). Temporary 8 KiB workload peak increases
  about 21 KiB; no idle arena. [Timings](nano-startup/REPORT.md).
- Native APPL/CODE alternatives were reviewed, not installed as a general worker
  backend. [Loader review](loader-review/REVIEW.md). Separate command partitions,
  general live pipes, complete awk system()/command pipes, SDK, Telnet and SSH
  remain unimplemented or incomplete; no new work is authorised by this list.
- Physical SE: user reports 4 MB, System 6.0.8, MultiFinder and BlueSCSI networking.
  MiniPerformer+ remains installed but was bypassed with programmer switches;
  subsequent CPU code is 1, consistent with 68000. Earlier CPU-code-3 timings
  are accelerator-state-unverified and must remain separate.

## Current updater acceptance

Private HTTP manifest/package uses CRC-32, lengths and native compatibility;
CRC detects accidental corruption and does not authenticate the source. Both
forks are staged beside the application. An independent native helper retains the
previous application, commits only after the running app exits, and relaunches.
The System 6 unavailable HOpenResFile trap and documented resource-fork scratch
bytes have been addressed; native launch uses a working-directory reference.

Snow full normal-build configured-source workflow passed: 2026111402 to
2026111502, automatic relaunch, --check and pwd. Extracted previous/new forks
and applicable Finder metadata match (hardware-update/snow-full-integrity.json).
Snow HTTP transport is an HFS fixture, not network acceptance.

Physical SE **passed** the same normal `clarice update` workflow over MacTCP/Wi-Fi.
Downloaded 919.50 KB in 58 s (15.85 KB/s); staged readback 2113 ticks (35.22 s).
After confirmation the independent helper installed and automatically relaunched
build 2026111502; --check and pwd succeeded. Active app is
`Macintosh HD:Clarice Update`. Retained previous target:
`Macintosh HD:.cu-00000a12-78c4020e.old` (data 0/resource 941423 bytes).
Original Clarice and Clarice Dev also remain unchanged. Configured source:
`http://<MACBOOK_LAN_IP>:8080/update.txt` in `/etc/clarice/update.conf`.

SEControl's heartbeat timed out during handoff; user reopened SEControl only,
then remote commands verified Clarice's automatic relaunch. Exact relaunch latency
was not measured; physical forks were not exported for host byte comparison.
The one-time binUnpk bootstrap needed its omitted bundle flag restored using a
disposable probe; no new production utility was added. Subsequent updates need
neither binUnpk nor manual Finder/restart operations.

[Tested package and command](hardware-update/accepted-build/),
[physical result](hardware-update/physical-update-acceptance.json),
[full evidence and limitations](hardware-update/PROGRESS.md).
Existing native rollback and 482 host fault scenarios remain relevant evidence.
No power-loss safety, authentication, public update service or peak-memory claim.
[Pending hardware checks](PENDING-HARDWARE-TESTS.md). No further updater expansion.

## Physical performance programme — in progress

- Full SE shutdown/restart restored Wi-Fi after MacTCP -23015. Exact network
  cause unproved. CPU code 1 rechecked after power cycle. First launch
  main-entry-to-ready 331 ticks (5.52 s), excluding OS loading.
- Static binary-search CLX import table preserves 180 production/181 diagnostic
  bindings. Physical `cat` import resolution **16 → 2 ticks**; observed loader
  **45–56 → 35 ticks**. After-run heap 424918 bytes, owned remainder zero.
  No persistent cache or allocation; isolated lookup code/constants shrink
  6970 → 3438 bytes. ASan/UBSan lookup checks, 61 loader/installer scenarios
  and validation of 24 real command images pass.
- Snow repeated nano: empty **449 → 434 ticks** (7.48 → 7.23 s), 8 KB
  **623 → 605 ticks** (10.38 → 10.08 s); loader **203 → 187 ticks**.
  Import phase 17–18 → 3–4 ticks; metadata validation remains 85 ticks.
  Before/after diagnostic configurations have different heap layouts; total
  change is not an isolated measure of the lookup improvement. Three seconds
  remains unmet. Physical loader-only measurements are now recorded in the UX
  section above; a controlled physical submission-to-editable-screen run remains
  outstanding.
- Snow safe-save regression preserves resource fork, type/creator, flags and
  creation date. Repeated nano exits return tracked allocations to zero.
  Four selection probe cases pass with at most one tick/highlight, no pixel
  mismatch or heap delta. Synthetic one-tick input lost 24/240 characters;
  that stress case is **not passed**; physical confirmation remains pending.
- Physical scroll benchmark: 30 frames native 259 ticks, cached C 500, cached
  assembly 460; worst frames 8/16/15 ticks. Four cache release cycles stable,
  zero test failures. No rendering preferences or cursor changes made.
- Host-only quiet mode and bounded 120 s listener timeout added; 14 focused
  host tests pass. Screenshots work again after the power cycle.
- Pre-exit update warning is implemented and visible on physical hardware.
  Physical profile package download 56 s; staging 4023 ticks (67.05 s), readback
  2088 ticks (34.80 s). Automatic relaunch then unexpectedly quit with error 28.
  Opening the installed application from Finder succeeded; --check confirmed
  build 2026111506 and `cat` worked. Cause not isolated. The later normal 1514 update/relaunch passed; see above.
- The same profile package passed Snow automatic update/relaunch; extracted
  old/new forks and metadata match expected contents. Incremental Retro68 build
  dependency on the version stamp fixed after a mismatched package was safely
  rejected. Normal experimental build 2026111507 was not deployed; it is superseded by
  the accepted normal 1514 deployment above. No diagnostic nano installed on the SE.

Evidence: [measurements](physical-perf/measurements.json), screenshots and focused
logs in [physical-perf](physical-perf/). User priority is now Snow interaction and
rendering performance first, physical retest after meaningful gains, then updater
error 28. Keep the working application and previous versions.

### Snow interaction/rendering — 15 November

- These earlier runs were Snow-only while the SE was shut down. The provisional
  60%-speed estimate was not measured; current matched scrolling gives 90.4%
  throughput. Neither number is a general conversion for other workloads.
- Source audit found earlier scrolling/selection implementations still present
  relative to v0.4.0; installed SE binary/preferences equivalence remains unproved.
  Typing after selection now clears only the affected region: eight matched
  guest cases fall from 32–45 to 3–4 ticks with identical pixels. No allocation.
- ncurses uses the existing strip to batch rows and inverse spans; parser avoids
  dirtying rows solely for attribute bytes. 8 KB first paint 118 → 96 ticks;
  cold total ~12.27 s, warm empty 6.80 s, warm 8 KB 9.65 s. Three seconds unmet.
  Nano edit/search/save passed, both forks/metadata preserved, exit allocations
  zero. These preceding results are retained, not rerun in the drawing pass.
- Matched same-build drawing probe identifies redundant full cached-screen
  presentation and duplicate Control Manager drawing. Reuse native pixel scroll
  for cached presentation, copy exposed rows, retain the staged scrollbar bitmap.
  Full redraw and covered/clipped fallback remain. No new persistent allocation.
- Complete 100-file ls median: native 173 → 172 ticks (effectively unchanged),
  cached C 191 → 177, cached assembly 186 → 174 (6.5% shorter). Assembly output
  sequence 353 → 197 ticks (44.2% shorter); history 281 → 218 (22.4%). Native
  history 231 → 199 (13.9%). These are guest timings, not physical acceptance.
  Assembly worst history frame is still 18 ticks; smooth drag latency unproven.
- Light and dark native/cached pixel checks pass: each 24 bursts + 20 views, zero pixel
  mismatches and 153 scrollbar staging/reference checks with zero mismatches.
  Focused host selection and 1560 scroll-guard checks pass. Ctrl+C stops native
  and cached assembly 1000-file listings, paged/unpaged, with status 130 and prompt
  restoration. Stock Snow modifier delivery failed; existing patched Snow fixed
  the automation without Clarice input changes. Normal quick scroll/ring/copy
  checks pass; four cache release cycles stable (29704 bytes including overhead).
  Restored native heap 425716 bytes, tracked remaining zero. Guest API 0607 is
  the previously documented 6.0.8 media/resource discrepancy, not a new OS claim.
- Native strip batching and scrollbar changed-scanline-copy experiments were
  measured and discarded. Only the invalid-row cursor-bounds correction remains
  from the strip experiment. No cursor design change.
- Normal experimental build 2026111509 compiled; resource 938732 bytes (+238
  versus 1508), no new heap cache. Matched probe heap stable within renderer;
  whole probe delta -2 bytes. This stage was not deployed; later 1514 hardware
  acceptance is recorded above. No push or publication.

[Drawing evidence and limitations](draw-performance/REPORT.md),
[matched counters](draw-performance/matched.txt),
[normal experimental build](draw-performance/experimental-build/Clarice.bin).
Earlier interaction/editor evidence: [Snow measurements](snow-performance/measurements.json).
### Listing, enumeration and scrolling — 15 November

- Normal experimental build **2026111511**; Snow Plus/4 MB/System 6.0.8 only.
  The SE was unchanged during that earlier stage; current deployment is above.
  No push/publication.
- Earlier duplicate executable-selection removal remains: complete warm native
  ls medians 10 files 118→108 ticks, 100 files 173→163. Loader checks unchanged.
- Deeper 1,000-entry scan counters put 284–289 ticks (~4.75 s) in native HFS
  calls, versus 10–13 setup and 24–33 name handling. Cost per 100 entries stays
  roughly constant. Later [HFS investigation](hfs-cost/REPORT.md) shows 338 unique
  catalog reads per scan and substantial cache-hot CPU cost; exact CPU/I/O split
  remains unmeasured.
- Reuse the synchronous catalog parameter block and avoid an intermediate name
  copy; restore explicit volume/parent/index every call. Normal warm scans now
  **323 ticks (5.38 s)** versus prior 336–338; cold first pass369. Same-image
  controlled reuse experiment supports a ~4% gain. Still too slow.
- Native small-scroll heuristic now accounts for per-row Toolbox costs. Matched
  four-row rendering: six frames 82→57 ticks using pixel reuse; large moves can
  be slower, so retain patching there and leave cached weighting unchanged.
  No new cache/idle allocation; normal resource +13 bytes, core CLX -20 bytes.
- Listing already streams bounded rows; captured output advances before the
  scan completes. More frequent painting alone was rejected: 100-file internals
  110→213/336 ticks at six/three-tick intervals. Smooth single-line bulk display
  remains unmet. Prior actual drag captures remain ~250–298 ms native and
  ~195–254 ms cached; no new end-to-end drag-latency claim this pass.
- New ASan/UBSan listing-loop tests and existing fail-closed volume checks pass.
  Actual two-volume guest fixtures pass for spaces, hidden/encoded names, cd/pwd,
  relative/absolute listings, disposable operations and missing-path errors.
  Native pixel checks: 24 bursts/20 views, zero mismatches; 153 clean scrollbar
  checks. Four Ctrl+C cases pass (native/cached, paging off/screen), status130.
  Final tracked allocations zero. No cursor design or loader validation changes.
- Follow-up catalog evidence is now above; async gave no gain. Retain chunked
  presentation and investigate remaining non-native work before painting more often.

[Current evidence](catalog-perf/REPORT.md), [experimental build](catalog-perf/experimental-build/Clarice.bin).
Earlier [dispatch/drag evidence](list-drag/REPORT.md) retains its original measurements.

## Development checkpoint — 15 November 2026

Application CODE/resources through 1533 have physical-SE acceptance above.
Bundled utilities do not automatically gain fresh hardware acceptance: wget and
updater were exercised physically; Kermit regression was Snow/host; other utilities
retain earlier evidence. [Exact CLX build inputs](ux-six/bundled-inputs.json) record
hashes, not new acceptance. Existing pinned build scripts remain authoritative;
generated resources/packages and local configuration stay outside Git. Generated
timestamps are not guaranteed bit-identical on rebuild. Focused input, progress
and confirmation checks are rerun for this checkpoint; no fresh hardware run.
