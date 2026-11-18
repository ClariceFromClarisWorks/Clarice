# BusyBox vi evaluation — 18 November 2026

Isolated branch `codex/busybox-evaluation-20261118`, based on the preserved
Heirloom/xvi comparison. No existing command replaced, Core changed, physical SE
used, branch merged or release published. Experimental command name: `bbvi`.

## Source and implementation

Genuine BusyBox **1.37.0** (2024 release), downloaded from
https://busybox.net/downloads/busybox-1.37.0.tar.bz2 . SHA-256:
`3311dff32e746499f4df0d5df04d7eb396382d7e108bb9250e7b519b837043a4`.
`licenses/busybox/` retains LICENSE and AUTHORS; source-file notices remain.
`prepare-busybox-vi.py` verifies/extracts pristine source and applies a separate
patch. Retained upstream vi editing/buffer/undo/screen algorithms and libbb
read_key, full_write, linked-list and whitespace helpers. Full-read and memrchr
helpers are attributed upstream excerpts in the boundary translation unit.

This is a selected-source vi port, not a general BusyBox build or libbb SDK.
`libbb.h` exposes only needed declarations/configuration. Allocation, diagnostics,
option parsing, terminal and HFS boundaries bind to existing Clarice services.
Raw mode uses existing runtime 1.9, no ncurses/termcap. No new resident cache,
loader, runtime API or Core modifications. The 1 KiB output buffer belongs to
this loaded invocation and is flushed before input; it avoids per-character
transport writes. Current configuration preserves upstream 10 KiB initial text
allocation and 4 KiB maximum screen-width scratch buffer; no arbitrary file cap
was added. Whole files and undo live in RAM, unlike Heirloom's temporary file.
This can make large-file insertion/movement and undo growth costly on a 68000.

Safe saves use existing `cli_replace_begin/commit/abort`, never upstream's
in-place overwrite. All writes and close must succeed before commit. Host exits
abort abandoned transactions. Existing HFS metadata/forks service remains owner
of recovery semantics; no power-loss atomicity claim. Automatic Unix ownership-
checked `.exrc` is excluded; explicit EXINIT and `-c` remain. HFS lock checks use
native catalog attributes rather than fictional Unix ownership/permission bits.

Enabled: insert/change/delete, literal search/substitution, undo and undo queue,
repeat, marks, named buffers, read-only, set options, byte-oriented display.
Disabled: optional GNU regex, shell/process/filter execution, job-control
signals, automatic .exrc, dynamic Telnet resizing/NAWS in this adapter. BusyBox
has limited colon commands rather than full ex. GNU regex search uses
`re_compile_pattern`/`re_search`; the existing sbase POSIX regex library is not a
drop-in replacement. Regex would require a separate size/compatibility decision.
Unsaved edits are lost on disconnect/cancellation; this remains experimental.

## Broader BusyBox value

Portability assessment from the pinned sources, not benchmarks or added commands:

- **Keep sbase and One True Awk.** No measured replacement benefit here; their
  adapted behaviour/tests are already investments. BusyBox is an optional source
  of commands, not a reason to switch the base userland.
- **Small stream utilities:** `comm` is an example of a narrow dependency path
  (stdio, allocation, line reading, option handling). Compare sbase's own comm
  before selecting BusyBox: this experiment grants no automatic preference.
- **diff:** useful candidate if wanted, but per-line arrays and file traversal
  deserve measured working-memory costs; vi's successful build doesn't prove it.
- **Archives/compression:** evaluate actual needs and standalone upstreams. BusyBox
  tar's compressor/decompressor integration uses pipes/fork/vfork/exec paths;
  gzip/tar is not unlocked simply by adapting vi. HFS metadata and safe extraction
  also require explicit behaviour decisions.
- **Shell, networking daemons, process/admin applets:** not low-cost extensions of
  this port. Unix process/socket/kernel assumptions remain absent or different.
  Preserve existing Clarice shell, native network and Telnet work.

Separate CLX packages fit independent updates and avoid loading a large multicall
image for tiny commands. Multicall can share disk code but its startup/RAM costs
on this loader would need measurement. Do not add a general BusyBox layer merely
to obtain that theoretical saving. Retain GPL notices/source and examine the
actual linking/distribution arrangement before integrating into a release.

## Validation and reproducibility

Host `tests/busybox/save.c`: ASan/UBSan injected begin/open/write/short-write/
close/commit failures, retry and empty save pass. No physical HFS failure test.
Build: stock 68000 `-Os`, no LTO, same pinned Retro68 image as prior ports.
CLX bounds/import validation uses the existing test, runtime requirement 1.9.

Private Snow Plus/4 MiB/reported System 6.0.8, own system/bench images/PRAM and
`org.clarice.BusyboxSnow`, loopback 23241 -> guest 10.0.0.2:2323, HTTP 8095.
Core 2026111823 and telnetd 2026111814 unchanged from the vi/xvi comparison.
Deployment uses local `clarice update -y --source URL`; authenticated Telnet is
used for actual tests. Authentication secrets are read from the existing local
fixture file and never logged. No other Snow instance or physical SE touched.

Harness corrections: initial Snow forwarding omitted the guest address and
panicked its NAT thread; corrected environment is
`SNOW_TELNET_FORWARD=23241,10.0.0.2:2323`. Initial tool-only repository omitted
its required Core record and was rejected with error -50. Added the unchanged
Core record; subsequent update installed bbvi. Neither was a Telnet editor bug.
Initial adapter rejected invalid options after entering raw mode, causing an
intentional raw-abort disconnect. Validate options first in corrected candidate.

Benchmarks separate Escape from following colon keys by 150 ms (all editors).
Functional tests also drain preceding input/output before sending Escape.
BusyBox's upstream decoder waits 50 ms to distinguish Escape/terminal sequences;
unknown multi-byte sequences can be discarded. This client spacing is not editor
startup time and is included in complete-operation timings. Fixed 80x24 ANSI
oracle validates row 22 and home cursor, then a visible insertion, before exit.

Guest functional regression: **43 observations passed** on corrected build
2026111871, including search/change, literal substitution, undo, arrows, save/
readback, read-only/HFS-locked refusal, Finder creator retention, help/error
streams/status, redirected-input refusal, unsupported shell refusal, updater
catalog/completion and byte-for-byte full 8192 B save (`cmp` status 0). Every
recorded invocation cleanup reports zero tracked survivors. Resource forks use
the same host safe-save service as the earlier ports; this evaluation has not
independently re-exported a resource fork. No physical/MultiFinder/System 7 or
large-file exhaustion acceptance is claimed.

## Matched performance results

Three interleaved runs per editor/input, order reversed in run 2. First means
first in the measured batch, not cold-machine launch. All 23 visible fixture
rows and cursor validated, then visible X insertion and discard/return to prompt.
Maximum-speed Snow, variable host load; **emulator evidence, not SE timings**.
No compiler build was running during this batch. Same Core/disks/terminal for
all candidates. BusyBox/xvi use Os; Heirloom requires O2 LTO to fit CLX limits.

| Editor/input | Fully drawn: first; repeats (s) | Whole open/insert/discard (s) | Tracked working peak (B) |
|---|---|---|---|
| bbvi empty | 1.161; 1.366, 1.203 | 1.816, 1.929, 1.761 | 21266 |
| bbvi 8KiB | 1.884; 2.156, 1.858 | 3.026, 3.201, 2.992 | 23314 |
| vi empty | 3.497; 2.854, 2.314 | 4.388, 3.603, 2.840 | 36292 |
| vi 8KiB | 6.974; 6.248, 5.486 | 7.695, 6.849, 6.087 | 36292 |
| xvi empty | 2.698; 2.814, 2.429 | 3.330, 3.533, 2.993 | 23728 |
| xvi 8KiB | 12.703; 10.898, 11.910 | 13.603, 12.409, 12.810 | 45996 |

Heirloom additionally allocates a native 4096 B line table outside tracked peak.
These figures exclude images, loader metadata, libc/native overhead and shared
terminal/network storage. All runs report remaining=0. Post-exit free heap spans
333518–333598 B, with no progressive loss across the interleaved batch.

Final BusyBox package **56704 B**, loaded image **36912 B**, **1352 relocations**,
55 imports. Heirloom: 256780/167504 B and 7137 relocations; xvi:
156044/99436 B and 4404 relocations. BusyBox image is **78% smaller than
Heirloom** and **63% smaller than xvi**. Loaded 68000 sizes, not upstream's
architecture-unspecified “26 KB” estimate. Final SHA-256:
`f94c494b1e252503ebbe7bf56403f1110d9511f4e600a4c3ff96c5113494156b`.

Median 8 KiB startup: bbvi1.884s, vi6.248s, xvi11.910s: **70% shorter than
Heirloom**, **84% shorter than xvi**. Whole operation median3.026/6.849/12.810s:
**56% shorter than Heirloom**. Complete-operation guest ticks bbvi192/199/201,
vi508/450/446, xvi839/829/836; bbvi loader49/49/50 vs vi136/112/113 ticks.

**Important trade-off:** first visible insertion after the screen is ready takes
bbvi267–332ms on 8 KiB, versus vi61–74ms and xvi145–199ms. This is the combined
`iX` interaction, not a sustained typing benchmark. BusyBox is not faster at
all operations. Its contiguous text buffer shifts following bytes on insertion;
that is a candidate cost to measure, not a proven attribution of every tick.
Startup output is similar to Heirloom (1719 vs1671 B), far below xvi13380 B.
Smaller loading costs and absence of Heirloom temp-buffer file work are plausible
contributors; no isolated causal breakdown of all startup time is claimed.

## Recommendation

Strong candidate for a compact **optional/default basic vi**, pending recovery,
compatibility and physical acceptance. Keep the existing editors available while
reviewing literal-only search, limited ex and insertion-latency trade-offs. Do not
replace sbase, awk, shell, network services or updater based on this experiment.

For additional applets: base64 encoding/decoding is a narrow stream-oriented
candidate; diff is a useful developer-workflow candidate with explicit memory
measurement; patch follows only with safe HFS replacement/metadata treatment
(upstream patch uses rename/unlink/temp files). Standalone compression deserves
its own RAM/performance comparison. These are recommendations, **not added ports**.

Reproduce:
```
scripts/build-busybox-vi.sh
python3 tests/vi/check-build.py work/busybox/bbvi.clx GUEST_CORE_clx-imports.inc
clang -fsanitize=address,undefined tests/busybox/save.c -o work/busybox/save-test
work/busybox/save-test
python3 tests/busybox/publish.py PRIVATE_REPOSITORY work/busybox/bbvi.clx NEW_VERSION
# Repository must retain exactly one matching Core record.
# Install locally: clarice update -y --source PRIVATE_URL/index.txt
PYTHON39 tests/busybox/regress.py PASSWORD_FILE
PYTHON39 tests/busybox/measure.py PASSWORD_FILE
PYTHON39 tests/busybox/lifecycle.py PASSWORD_FILE
python3 tests/busybox/summarize.py
```
Python3.9 plus pyte0.8.2 as in tests/xvi/requirements.txt. Local password fixture
is not tracked. Raw regression/timings/build/lifecycle records live beside this
report. Broad production compatibility and physical SE speed remain unmeasured.

Lifecycle validation: disconnect after visible `UNSAVED` insertion; first
reconnect one second later authenticates. Runtime remaining0, help and another
full-screen edit/discard pass, original8192 B file unchanged. Unsaved text is
lost, not recovered. Corrected final clean build reproduces the installed package
byte-for-byte (`build.json` equals `rebuilt.json`).

Cleanup: disabled remote access, quit Clarice to Finder and closed only the
private BusyboxSnow workspace. Stopped its private HTTP repository server.
Private disks remain available for reproduction; the physical SE was untouched.
