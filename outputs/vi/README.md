# Original vi comparison — isolated investigation

Investigation on `codex/upstream-vi-comparison-20261118`, based on xvi
investigation commit `79a1000`. No Core changes, merge, push or physical SE use.

## Source and configuration

Genuine Berkeley ex/vi 3.7 lineage, maintained Heirloom port:
<https://github.com/n-t-roff/heirloom-ex-vi>, pinned
`3f1e9786d249d18012600ae21175eb3521579402` (4 July 2026).
Original authors Bill Joy and Mark Horton; portability work Gunnar Ritter.
Bundled BSD termcap is compiled. The supported traditional regex option uses
Heirloom `libcommon/regexp.h`, pinned at `2be457cd6b889105d5a550c473238e6922e4bd51`,
SHA256 `f5a68615b7997c87205f68763af673d8632462c189dccaca35c27220e049e73f`.
Copyright and licence text retained in `licenses/vi` and upstream source.
This product includes software developed by the University of California,
Berkeley and its contributors. This product includes software developed or
owned by Caldera International, Inc.

68000 `-Os` compile and `-O2` whole-program LTO at link, VMUNIX line-address representation, byte-oriented BIT8/ISO8859_1,
no multibyte locale, no optional Lisp mode, no fixed 64 KiB INCORB cache.
Ordinary upstream temp-file buffers and editor/screen algorithms remain.
Changes are separable in `src/ports/vi/upstream.patch` and the platform adapter.

## Boundary choices

- Reuse CLX runtime allocation/stdio, native safe replacement, terminal modes
  and existing runtime 1.9 Telnet raw ownership. No custom loader or Core edits.
- `sbrk` use is confined to the line table. A native 4 KiB NewPtr grows in place
  with SetPtrSize, preserving upstream interior pointers. Failure is ENOMEM;
  no fake Unix break or large reserved arena. Release after every invocation,
  including runtime longjmp. Native line-table storage is additional to tracked
  runtime allocation metrics. Growth depends on contiguous adjacent space.
- Explicit normal saves use existing fork/metadata-preserving safe replacement.
  Commit only after all writes and close succeed; unwind aborts incomplete
  stages and permits retry. Append writes refused.
- HFS temp-file cleanup closes before unlinking. Default directory is `/tmp`.
  O_EXCL remains; O_NOFOLLOW is omitted on HFS, which has no symlinks. Upstream unique temp names use
  guest ticks and O_EXCL retries, not invented process IDs.
- No child processes, filters, shells, cd, automatic ownership-based exrc,
  job control, or recovery helper. Unsupported operations fail. The temp-file
  header UID is only a zeroed format field, not an identity/security claim.
- Telnet 80x24 ANSI/BINARY contract and experimental disconnect/cancellation
  limitations inherited from the xvi investigation. No production-readiness claim.

The initial package (1860) was rejected by `clarice update`: 8,989 relocations
exceeded the existing 8,192 CLX limit. The guest reports this as “damaged or
truncated CLX image”, which is misleading for a well-formed but unsupported image.
No checks were weakened. Disabled Unix paths and optional Lisp mode were removed
from the small build. Whole-program O2 LTO then reduced the final image to
167184 B, 7151 relocations and 256564 B packaged (1861), within unchanged Core limits.
The native Toolbox adapter is compiled without LTO because Retro68 GCC 16.1
hit an internal compiler error when streaming native trap declarations. GCC
produces the actual relocatable ELF as `.o.gdb`; the build packs that ELF, not
its accompanying Macintosh wrapper.

## Tests

`tests/vi/save.c`: ASan/UBSan host failure injection passes begin/open/short-write/
close/commit failures, error unwind, retry, successful commit and unrelated closes.
This is adapter logic evidence, not HFS or emulator validation.

Guest regression: **44 observations pass** including insert/search/change,
substitute, undo, arrows, save/readback, read-only and HFS-locked refusal, Finder
creator retention, help/error streams and exit status, redirected-input rejection,
unsupported shell/recovery refusal, and **byte-for-byte equality of the full
8192 B file after vi writes it** (`cmp` status 0). Updater incomplete marker is
absent and catalog records the installed vi. Runtime cleanup is zero throughout.
Disconnect after a visible unsaved insertion exits and releases tracked memory;
first reconnect authenticates, subsequent edit succeeds, and
the original file remains unchanged. Unsaved recovery is not provided.
A fresh preparation/build from pinned sources reproduces the installed package
**byte-for-byte** (`rebuilt.json` matches `build.json`).

Guest build **2026111864** installed through `clarice update`; Core 2026111823
and telnetd 2026111814 unchanged. Final CLX: **256780 B packaged, 167504 B
loaded image, 7137 relocations, 56 imports**. xvi 2026111843: 156044 B package,
99436 B image, 4404 relocations. Vi has 1.68x the image and 1.62x the relocations.
No 64 KiB in-core cache is used.

## Matched performance results

Same isolated Snow Plus/4 MiB/System 6.0.8 guest, same disks/Core/Telnet, 80x24,
maximum acceleration enabled (observed speed varies around 1.3–1.5x), host also
running other agents. **These are emulator observations, not stock SE timings.**
Empty and exactly 8192 B (128 numbered 63-character lines + LF) inputs.
The harness validates every input line, observes the final visible row and home
cursor using pyte 0.8.2, inserts X and observes it at the cursor, then discards
and waits for the shell prompt. Three runs each, interleaved with reversed editor
order on run 2. First below means first in this measured batch, not a cold-machine
benchmark. No results from the earlier incorrect-file-display build are accepted.

| Input/editor | Full screen: first; repeats (s) | Submission through key response (s) | Whole open/insert/discard/prompt (s) |
|---|---|---|---|
| Empty xvi | 2.276; 2.416, 2.228 | 2.414; 2.511, 2.351 | 2.665; 2.847, 2.597 |
| Empty vi | 2.792; 2.894, 2.340 | 2.877; 2.960, 2.434 | 3.258; 3.285, 2.776 |
| 8 KiB xvi | 9.138; 10.271, 9.780 | 9.269; 10.404, 9.931 | 9.717; 10.897, 10.367 |
| 8 KiB vi | 5.510; 5.467, 5.324 | 5.567; 5.529, 5.429 | 6.018; 5.987, 5.900 |

Vi's median 8 KiB full-screen time is **44% shorter**; complete operation is
**42% shorter**. Empty startup generally favors xvi, with overlapping ranges and
host scheduling variability. Guest complete-operation ticks reinforce the file
result: xvi 793/810/811, vi 460/453/453. Empty ticks: xvi 186/214/184,
vi 202/193/197. Loader alone: xvi 87–92 ticks, vi 115–126 ticks.
Keyboard response once drawn: xvi 95–151 ms, vi 57–105 ms in this harness.

Startup bytes: empty xvi 1138 / vi 139; file xvi 13380 / vi 1671. This is
consistent with original vi's much more economical terminal output, whereas
xvi's prototype emits repeated cursor/color sequences. It is not a controlled
attribution of every saved tick, nor a claim about all vi/xvi implementations.
Compiler configurations differ: xvi Os, vi requires O2 LTO to fit existing CLX
limits. Both are genuine upstream cores behind experimental Clarice boundaries.

Tracked allocation peaks: xvi 23728 B empty / 45996 B file; vi 36292 B for both,
plus a separate 4096 B native line table. All report `remaining=0`; post-exit
heap varies only 333536–333596 B in the interleaved batch, with no progressive
loss. These are not whole-process RAM peaks: add the loaded image, loader
metadata, libc/host/terminal/network allocations and native block overhead.
Native line-table growth beyond its initial 4 KiB is not acceptance-tested;
in-place growth can fail when adjacent space is unavailable.

Raw evidence: `measurements.json`, `summary.json`, `fixture.json`, `build.json`.

## Decision and next step

Original vi is substantially heavier in executable footprint and porting
requirements, but is faster for the displayed 8 KiB workload in the current ports.
Keep both experiments separate from production. A focused improvement to xvi's
terminal adapter, followed by the same comparison, is the most useful next
performance step. Neither editor is production-ready without bounded disconnect/
cancel recovery and broader save/compatibility acceptance. No such further task
has been started. Physical SE and MultiFinder/System 7 tests remain outstanding.

Unsupported gaps also include manual `:source` (descriptor duplication), wildcard
filename expansion, optional Lisp mode and Unix process controls. Some disabled
paths still use upstream generic errors. Resource-fork preservation uses the
same host safe-replacement service tested in the xvi investigation; this vi task
has not independently re-exported and compared resource forks.

## Portability failures found and corrected before file timings

Build 1862 opened/saved a new short file and drew an empty screen, but the strict
8 KiB screen oracle rejected it: the first visible line was 64 instead of 0.
No timing from that file run is accepted. Original vi ignores a failed lseek
in private temp-file I/O; native HFS SetFPos rejects an offset past EOF, so
subsequent writes landed at the wrong position. The port now checks all temp
seeks and extends only necessary write gaps with existing zero-filling
`port_ftruncate` / `cli_file_resize`. It does not change general runtime seek
semantics. `tests/vi/temp.c` passes ASan/UBSan failure injection and distinct
block readback against a simulated HFS EOF bound.

Clarice's minimal `port_fstat` supplies type and size but no write-mode bits.
Upstream interpreted this as read-only. The port uses its existing native HFS
lock check through `access(W_OK)` for this decision, without inventing permissions.

After the failed screen oracle disconnected, vi repeatedly retried a fatal raw
read error and blocked reconnects. Local Command-period did not recover the
loop. Only this disposable Snow guest was reset. The corrected adapter exits
on fatal transport read/write failure, as the xvi experiment does. Unsaved
recovery on disconnect/cancel is still unsupported, prominently documented.


## Reproduction

```
scripts/build-vi.sh
python3 tests/vi/check-build.py work/vi/vi.clx PATH_TO_GUEST_CORE_clx-imports.inc
clang -fsanitize=address,undefined tests/vi/save.c -o work/vi/save-test
clang -fsanitize=address,undefined tests/vi/temp.c -o work/vi/temp-test
work/vi/save-test
work/vi/temp-test
python3 tests/vi/publish.py PRIVATE_REPOSITORY work/vi/vi.clx NEW_VERSION
```

Install from that private repository using local `clarice update -y --source URL`;
the existing Core deliberately rejects remote updater execution. All editor tests
then use loopback Telnet with the existing saved authentication, not the SE.
Use Python 3.9 (`telnetlib`) and pyte 0.8.2, as in `tests/xvi/requirements.txt`:
`tests/vi/measure.py PASSWORD_FILE`, `regress.py PASSWORD_FILE`, and
`lifecycle.py PASSWORD_FILE`. Passwords are read from a file and are not logged.
Ports 23240/8094 and all emulator/disks are private to `work/vi-snow`.
The current branch's base Core lacks raw runtime 1.9; tests intentionally use the
newer local Core fixture, as documented for xvi. No unpublished Core work was
copied into this branch. `scripts/relink-vi.sh` is only an incremental development
helper; the normal build always re-prepares upstream and clears old objects.

Final cleanup: Telnet disabled, Clarice quit to Finder, own Snow workspace closed
to Untitled (emulation stopped), own HTTP server stopped. Other agents and SE untouched.
