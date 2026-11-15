# Utility capability and elastic-memory audit — 16 November 2026

Source inventory against the current working tree; this is not blanket acceptance
of every utility. No executable cache, new builtin, native-process architecture,
or permanent allocation added. Physical SE is off; new results are host/Snow only.
Existing upstream sources remain pristine; adaptations are in preparation scripts.

## Builtins and reuse alternatives

Current shell builtins: cd, pwd, echo, export, unset, set, shift, read, printf,
test/[ , true, false, :, break, continue, return, exit, sh, ., history, clear,
prefs, status, help and appinstall. Diagnostic dispatch also includes scrollbench,
selftest and checks. ls/df/cat and clarice/update remain external commands.

Native unlocked purgeable Handles could retain one raw executable file, locked
while copying, and released before large allocations or allocation retries. They
would still require identity/replacement checks and fresh mutable invocation
memory. Current read timings do not justify this. Retaining relocated CLX code
is more complex: absolute relocations and writable state make movement/reuse
unsafe without reset/separation work. Native CODE segments use A5/jump-table
contracts, not interchangeable CLX storage; builtins sacrifice independent updates.
Keep on-demand loading. Existing matched tiny physical probe was 33–34 ticks,
not nano's 1.42 s; no new physical ls timing is claimed.

## Capability inventory

| Commands | Current storage/limits | Assessment and action |
|---|---|---|
| awk | Dynamic records, fields, arrays and regex state; Clarice injected 16 KiB adjbuf ceiling; 1 KiB initial records; expression/call depth 64; parser 64 initially, 128 maximum. | Remove record ceiling using upstream growth/error handling plus overflow guard. Keep small initial allocation. Stack/parser depth needs separate measured work; do not just raise it. system()/popen explicitly unsupported. |
| grep | Dynamic getline and pattern list; musl regex allocations; compiled repetition bound 255. | Current source elastic; installed guest binary still rejects >16 KiB. Rebuild and verify separately; remove false 96 KiB/16 KiB help claims. Pattern-file input avoids shell argument limits. Regex allocation/execution errors report nonzero; streaming matching is per record, not whole file. |
| sed | Dynamic pattern/hold/script buffers and labels; regex match storage; potentially growing hold space. | Current source elastic; installed guest binary still rejects >16 KiB. Rebuild separately; constrained by available heap/stdio slots. Remove false help claims. No GNU -i. Script-driven whole-file accumulation still legitimately consumes memory. |
| sort | 128-line/16 KiB batches, binary spill levels 16; Clarice 16 KiB record rejection; two merge input records; optional unique previous record. | Remove record rejection. Oversized records transfer getline ownership and form singleton runs, avoiding an extra full copy. Keep packed small records/batch policy. 16 merge levels and 20 runtime temp slots remain explicit capacity errors; total file is not held in memory. Same-file -o works on success, but output failure is not an atomic safe-save guarantee. |
| tail | Retained trailing bytes/lines grow; linear buffer compaction/scanning; -f yields and tracks truncation. | No current 96 KiB cap. Very large requested tails/one enormous line may exhaust heap; explicit allocation error. Consider backward seeking for regular files later; retain streaming input semantics. -m intentionally unsupported. |
| head, cut, uniq | Dynamic line buffers; cut range list; uniq current/previous records. | No fixed 16 KiB cap. Allocation exhaustion exits nonzero. Sequential file processing, no whole-file retention. |
| wc, tr | Streaming counts/runes; tr allocates parsed sets/ranges proportional to argument lengths. | Fixed I/O buffers are batching, not total-file limits. Numeric counter/integer width and UTF-8 semantics remain; terminal display is MacRoman. No speculative expansion table/cache needed. |
| tee | Streaming data, dynamically allocated output descriptor list. | Shared descriptors now grow on demand; native FCB/heap limits remain. Further opens report failure; already opened outputs can still be written. Host 40-stream/40-descriptor and guest native-capacity tests are recorded in utility-correctness. |
| ls, df, cat | Catalog-by-index streaming; no directory result array; ls has MAX_ARGS=32 operands; cat uses 2 KiB copy buffer. | Loader timing measured separately from enumeration/drawing. HFS index/type widths and full-path adapter limits remain. No arbitrary directory result count found in listing loop. Not alphabetical sorting. |
| find, du | Demand-allocated iterative DFS; explicit -maxdepth; no symlink following. | Former depth-8 restriction removed and host/Snow depth-24 tested. Native path/catalog limits remain. du may print partial totals accompanied by nonzero status; consumers must check it. |
| cp, mv, rm, mkdir, rmdir, touch, ln, readlink, hfsattr | cp/rm explicit demand-allocated DFS; fork copy streams through 4 KiB stack buffer; safe staged per-file cp; fixed path buffers. | Artificial depth cap removed; allocation/cancellation/fork recovery tests pass. Directory operations are not tree-wide transactions: earlier files may be affected before a later depth/error failure. HFS forks/type/creator preserved by appropriate operations. No Unix ownership semantics. |
| file, od, more | Native metadata descriptions; od/more stream 256-byte chunks (od rows 16 bytes). | Buffers do not cap file size. file is not libmagic. more's terminal history is bounded separately; redirected output bypasses paging. |
| basename, dirname | In-place path operations. | No extra whole-file or persistent allocations; shell/native path limits still apply. |
| serial, kermit | Bounded serial/protocol buffers, streaming files; Kermit filename/path validation and temporary receive file. | Protocol buffers are intentional. Data-fork transfer only; full Mac files need MacBinary. No new hardware tests in this audit. |
| wget | 2 KiB streaming body; host buffer 32, request 768, response line 1024; signed/native file lengths. | Host/request/header bounds reject unsupported sizes; not a response-body RAM cap. Numeric private-LAN HTTP client scope, no TLS. Optional updater download maximum is intentional. |
| clarice update, appinstall | Streaming package/checksum/staging; bounded paths/manifest; recovery copies and compatibility checks. | Integrity/transaction policy is not an artificial utility memory cap; preserve. appinstall is builtin, clarice is external. |
| nano / ncurses | Dynamic editor buffers and screens; application heap/stack/terminal dimensions constrain sizes. | Safe HFS replacement retained; no editor change in this task. Current 4 MB machine does not mean a command owns 4 MB: Clarice's MultiFinder partition and contiguous free blocks matter. |
| benchls, benchscan; ncurses/probe fixtures | Diagnostic commands, no userland capacity promises. | Keep benchmark instrumentation distinct from production behaviour. |

## Shared limits and risks

- Portable allocator already grows by demand: 2 KiB small-object pages, native
  larger blocks, up to 8 KiB invocation-only spare reuse. No permanent arena.
  32 KiB free-heap headroom is an error/UI reserve, not a 96 KiB command cap.
  Realloc currently copies and briefly needs old+new blocks. This is a practical
  large-record limit even when final allocation alone would fit.
- Portable streams/descriptors now grow on demand with stable cookies and reuse;
  host/Snow failure/cleanup evidence is in ../utility-correctness/RESULTS.md.
  The 20 temporary-name slots remain a separate limit. Native open-file capacity
  and shared newlib FILE-record high-water storage still apply.
- Format adaptation has a 510-byte format-string guard; overflow exits explicitly.
  This limits formats, not formatted output length. Do not confuse it with a line cap.
- Shell: 32 argv entries, 8 KiB source/expansion storage, 256-token guard,
  16 variables (31-byte names/127-byte values), 16 functions, read's 127-byte
  input, command substitution 2047 bytes, nested-source/AST bounds. These can
  constrain otherwise elastic tools. read previously returned a truncated prefix as success; now fixed to consume
  the record, report overflow, preserve the variable and yield while draining.
- Paths: HFS components are 31 bytes; adapters use 255-byte native Pascal paths
  and 512-byte encoded shell paths, and follow at most 16 links. HFS itself can
  address deep directories by ID: full-path adapter ceilings are not a universal
  filesystem depth limit. Overflow is generally rejected, not silently shortened.
- CLX image <=256 KiB, relocation count <=8192, imports <=128 are format/loader
  admission restrictions; not each command's dynamic working-memory ceiling.
- Runtime closes streams/descriptors, removes owned temps and frees owned/spare
  allocations on normal return, exit, error and cooperative cancellation.
  Fresh CLX images reset globals. Cleanup is not guaranteed after a machine crash,
  forced emulator termination, or noncooperative memory corruption.

## Ranking

1. Removed awk/sort text ceilings and corrected stale capability help: small,
   demonstrable improvement using existing upstream allocation/merge algorithms.
2. Fixed shell read overflow semantics without additional allocation; host
   boundary/EOF/next-record/cancellation checks pass. Guest acceptance below.
3. Stack-safe find/du traversal, then reuse suitable traversal for cp/rm with
   explicit partial-operation semantics. Avoid a broad filesystem rewrite.
4. Stable demand-allocated stdio/descriptor entries for awk redirects/tee; requires
   shared ownership tests. Do not just increase array sizes.
5. Tail backward seeking and realloc peak reduction only after matched workload
   evidence. Executable caching remains lower priority unless reads dominate.

Host ASan/UBSan tests compare actual output against pristine upstream programs;
large, low-memory, cancellation and spill-failure results are in elastic-host.json.
Further guest measurements and acceptance are recorded in CURRENT-STATE.

## Snow loader evidence

Disposable clone of the existing Plus/4 MB fixture, build 1536 diagnostics; guest
reports System **6.0.7**. Do not relabel this as independently verified 6.0.8.
Actual installed ls image is 7,912 bytes, packaged command 12,376 bytes. Six
invocations each of --help, empty-directory ls and ten-entry ls; stdout /dev/null,
status captured after each. Existing receipt matches, installed-fast policy;
no loader change. First --help after launch: 53 load ticks, 55 invocation ticks;
preparation 28 includes 16 stdio ticks and image/meta allocations 4/3 ticks.
Warm loads generally 39–42 ticks (~0.65–0.70 s). One 72-tick outlier includes a
33-tick event-service interval; retain it in raw evidence, don't hide it.
Executable+metadata read 12–14 ticks; receipt work ~14–16; preparation 7–9;
imports 1–2 and relocations 0–1. These phases are tick-quantised and nested event
counters must not be added again. Per-allocation image/meta work is usually one
tick each, included in preparation. ls has no portable-runtime initialisation;
first shared stdio preparation is separately visible. --help work/return adds
~1 tick warm; empty directory ~6–7 and ten entries ~12–14. Module invocation
counters exclude shell parsing, redirection setup and final prompt repaint.

Reading is roughly one third of warm load time, not dominant. This includes
synchronous native read/guest-driver CPU time, not a physical disk-latency probe.
An ideal cache could only eliminate part of that interval and would retain about
12 KiB, require copying/identity checks and not remove receipts/relocation/setup.
No cache experiment is justified here; prioritise utility capability instead.
No physical performance claim, receipt-policy change or repeat native-APPL test.

## Completed focused validation

Host: 37 elastic-text cases under ASan/UBSan; 30 upstream awk comparisons;
45 tail/sort comparisons plus spill, write-failure and cancellation checks;
31 shell comparisons and focused read boundary/EOF/cancellation tests. Logs and
JSON in this directory. Shell read does not pretend to support larger variables:
it now explicitly rejects overflow after consuming the record, preserves the old
value, and permits the next read to receive the next record.

Normal build 2026111538, fresh disposable Plus/4 MB image: 10 elastic cases
exported after quitting/flushing, exact bytes and complete evidence log verified
by tests/utility-audit/validate.py. 64 KiB sort and awk records succeed; 600,001-byte
records fail explicitly and subsequent small commands succeed. The 2,001-field
awk fixture exhausts the available heap, rather than hitting the removed text
ceiling. All cases report remaining=0. Peak owned bytes: small sort 4,492;
20 KiB multi-record sort 134,887; 64 KiB sort 208,408; awk length 152,101;
field exhaustion 215,793. Free heap across that script changed 386,422 to
386,202 bytes; zero owned survivors is not a claim of identical whole-shell heap.
Interactive cancellation screenshots show status 130 for both ports, remaining=0,
and successful subsequent small sort. Read boundary screenshot confirms exact
127-byte success, 128/8,192-byte failures preserving old value, and next-record
correctness. No extra permanent allocator/cache was introduced.

The original scratch disk developed a log extent that failed offline export
at 6,144 bytes and native cp with -127 after restart. Its history includes an
unflushed forced emulator termination. Root cause is NOT established. Preserved
image: work/utility-audit/audit-preserved.hda; partial original export retained.
Fresh driver-bearing image, explicit FlushVol, native copy and complete offline
comparison all pass. Do not call this a filesystem repair or discard the failed
fixture. Physical SE was not accessed. Guest reports 6.0.7, not verified 6.0.8.

Normal application package 943,360 bytes versus previous 943,104 (+256 bytes);
existing bundled command resources were not regenerated. Independently tested
ports are supplied separately. Earlier nano/editor/loader work remains intact.

Rebuilt grep/sed against current runtime after the installed guest binaries both
reported `text tool: line exceeds 16384-byte limit` (status 2, no owned survivors).
This distinction matters: updating Clarice itself does not refresh independent
commands. Source review of the 64 KiB sed all-character substitution found expensive upstream
String appends: strnacat repeatedly scans both source and growing destination,
then strlcat scans again. This is an algorithmic review candidate,
not grounds for a cache or a larger fixed buffer. No sed algorithm changed here.

### Unresolved additional grep/sed guest check

Current-runtime grep/sed cross-builds completed and host 64 KiB tests passed.
The combined ports.sh Snow run did not return to the prompt. Option-C, Control-C
and Command-period did not yield a confirmed exit. Subsequent single-substitution
and status commands were not visibly executed, so do NOT count them as tests.
A copy of the running disk had no persisted ports.txt; it cannot locate execution
within the script. The moving pinwheel alone is not proof of useful progress.
No attribution to sed's algorithm is established. Both rebuilt candidates are in
unvalidated/, excluded from the ready build directory. No production fix inferred.

Stopped ONLY this disposable Snow instance after preserving ports-interrupted.hda;
this snapshot is unflushed and is diagnostic, not filesystem-consistency evidence.
Earlier fresh-image exact outputs/logs had already been exported and validated
while offline before this extra experiment. Physical SE and other Snow instances
were untouched. Next action: isolate grep and sed as separate small/large guest
commands with observable stage results before accepting either rebuilt package.

Current acceptance inventory and portable shell matrix: [COMPATIBILITY](../utility-correctness/COMPATIBILITY.md). Shared native FILE retention is measured reusable stdio capacity, not automatically reclaimable general heap: [libc probe](../utility-correctness/LIBC-POOL.md). No libc change or executable cache implemented.
