# Terminal UX and hardware capabilities — 15 November 2026

Development work against accepted 2026111525; checkpoint 33f0075 remains intact.
Final normal build **2026111533** includes all six changes and the bounded
queued-control correction. Host and Snow validation pass. Physical 1531 validated
the main UX changes; final 1533 updated/relaunched automatically and passes the
physical editor/queued-control checks. The SE is left at the live shell prompt.
No release or push.

## Changes

- `prefs control control|option|command`, also in Preferences and saved in
  `/etc/clarice/clarice.conf`. Option remains the default; physical Control always
  works. Command menu shortcuts retain priority; Command+Option supplies a
  conflicting control in Command mode. Command-period is local cancellation.
  Translation uses the active native KCHR keyboard layout. A local volatile
  resource-ID variable avoids a demonstrated Retro68 Pascal-call stack-operand
  miscompilation (the initial generated code passed the low half of `KCHR` as
  the resource ID). Raw editor control bytes are distinct from Macintosh arrows, including when
  queued during loading. The queue preserves the modifier and counts one raw
  control byte versus three arrow-sequence bytes. Unsupported shell controls
  28–31 are ignored rather than treated as history arrows. The actual encoder/
  queue test fails before the fix and passes after; 128 control bytes fit, while
  129 controls/43 arrows reject without partial delivery. Final Snow native probe
  and actual queued Ctrl-underscore → nano Go to Line pass.
- Greeting reports build, model, CPU/FPU, installed RAM, main display and the
  reported System API version. `cli_hardware` is a bounded, allocation-free
  runtime 1.3 snapshot; existing commands keep their earlier runtime requirements.
  SysEnvirons fallback and guarded Gestalt use native queries. Unknown CPU/FPU
  values remain unknown; accelerator detection is only as good as native APIs.
  This 6.0.8 Plus image reports 6.0.7 through the API; the greeting labels that
  version as reported. Later CPUs, FPUs and colour hardware are not qualified.
- History uses the full viewport without a pinned prompt. Typing/paste returns
  to live output without losing the trigger; foreground typeahead does likewise.
  Selected historical text includes the former prompt row. Existing bitmap,
  pixel-scroll, clipping and glyph paths are retained.
- CLI/theme changes invalidate the complete window, avoiding a partial pending
  update swallowing the full repaint. Input and history survive both themes.
- Update confirmation occupies a visible line: `[Y/n]`, Enter/y/yes accepts,
  n/no cancels; `-y` accepts noninteractively. EOF/truncated input fails closed.
  Messages say Clarice closes and relaunches. Staging, verification, helper,
  native replacement and recovery operations are unchanged.
- Shared invocation-local progress fits terminal width (up to 130 columns),
  displays current/total IEC sizes, percentage, average speed where available,
  unknown totals and completion/failure. Optional fields disappear in narrow
  windows. Updates replace one row, at most once per second plus initial/final;
  redirected stderr suppresses them. No allocation, floating point or 64-bit
  division in the formatter. wget, Kermit and updater use it. HTTP support is
  unchanged: Content-Length is required; unknown totals currently serve Kermit.

## Actual evidence

Host ASan/UBSan: progress widths 1–200, sizes/counters, throttle, redirection,
confirmation/default/EOF/truncation; preferences round-trip; control mapping;
input layout/wrap, paste/overflow, selection delta, scroll guards; editor raw
bytes versus arrow sequences; runtime imports. See `host-tests.txt`.
Kermit host relay: seven binary, corruption/retry, cancellation and recovery
cases pass with the updated adapter; see `kermit-host.txt`.

Snow Plus/4 MB/6.0.8: extended native UX probe passes (initial two keyboard-layout
failures were reproduced, diagnosed in generated code and fixed). Real native
resource translation, physical Control, Option and Command modes, Command-C,
editor Ctrl-underscore, serial cancellation, full scrollback, triggered input,
foreground typeahead and immediate theme redraws are exercised.
`scrollbench native`: 22 bursts + 20 views, zero pixel mismatches, 147 scrollbar
checks pass; repeated in dark theme. Clipping test capture is retained separately.
The extracted clipping result is **322 comparisons, zero failures, heap delta 0**
(`snow-clipping.txt`). Normal 1531 also passes native pixel/bar checks after nano.
Genuine nano opens the 8 KiB document in Command mode and exits with Command-X;
Option-C interrupts an output loop with status 130. The actual Preferences popup
selects Command, CLI and dialog changes apply, and unfinished input survives theme
changes. Command-A moves to the beginning without losing the following character. A full
application/emulator restart restores the dialog-selected Command mode and dark
theme (`snow-restart-preferences.png`). Normal launch-to-ready is 318 ticks (5.3 s);
that existing counter excludes pre-main loading and completion of the first paint.
It is a new Snow sample, not a matched baseline comparison.

Snow Kermit: two 4096-byte all-values round trips match byte-for-byte; cancellation
returns guest status 130, then the same destination can receive successfully.
All other statuses are zero; invocation-owned remainder is zero on all five runs.
Heap during redirected status capture ranges 420,938–420,948 bytes; after returning
to the prompt it is 421,184. Timings in `snow-kermit.json` include automation typing,
so are not transfer-rate measurements. Protocol data remains separate from progress. Ordinary serial echo/reconnection
also passes: the host receives exactly `UX-echo\x03UX-reconnect`; Option-C delivers
byte 03 remotely, Ctrl-] q exits with zero and Command-period returns 130 locally
(`snow-serial.jsonl`, screenshots).

Updater: declining with n leaves the installed application unchanged. A diagnostic
1526 → normal 1529 run replaced correctly but hit the existing System error 28 on
relaunch; no cause/fix claimed. A subsequent **normal 1529 → normal 1531 with -y
completed and automatically relaunched**. Exported data and protected resource
bytes exactly match the package (native resource-manager scratch bytes 16–127 are
normalised by the existing policy). The prior normal application is recoverable;
type/creator, Finder flags and creation date match (`snow-normal-install.json`).
Disposable-disk recovery apps were moved into a recovery folder after tests to
separate overlapping Finder icons; no recovery copy was deleted.

Snow large jumps now draw the additional historical bottom row: retained-path
median dense **485 ticks/24 = 336.8 ms**, directory **233 ticks/24 = 161.8 ms**,
versus historical 1525 measurements 327.1/158.3 ms. These are successive runs,
not a matched same-binary comparison; changed content/row count prevents claiming
an optimisation/regression of the unchanged glyph kernel. Paired heap delta zero.
20 native capability queries used 26 ticks (about 22 ms/query) in the Plus probe,
with no retained allocation. No renderer optimisation undertaken.

Physical SE, stock CPU code 1, 4 MB, System 6.0.8 (API 0607):

- New wget, Kermit and updater CLX images validate/install independently under
  accepted 1525. Existing command backups retained, with older backups archived
  under `/etc/clarice/ux1525`; no overwrite of the only recovery copy.
- Old wget 64 KiB: **347 ticks**; new display **352 and 322 ticks** (load 39 versus
  44/42). These small LAN samples do not establish a speed change; about 5.4–5.9 s
  total. Completed in-place full-width bars and amounts are visible. Free heap
  returns to 423,998 bytes, owned remainder zero on the old shell.
- `--check` identifies 1531. Staged 920.88 KiB package with visible progress;
  staging **4345 ticks (72.4 s)**, staged verification **2025 ticks (33.8 s)**.
  Those unchanged costly stages are not optimised in this task.
- The full warning and `[Y/n]` are visible above the viewport bottom. Enter was
  sent to accept. SEControl then exceeded its 120-second response deadline during
  the handoff and disconnected. The owner reported that Clarice returned after approximately four minutes and
  SEControl appeared to have crashed. After restarting SEControl, foreground
  capture **confirmed build 2026111531 running on the SE**. No new System error 28
  was reported; cause of the helper loss remains unestablished. The four-minute
  observation is user-reported, separate from instrumented staging/verification.
- Physical greeting and external runtime 1.3 consumer identify SE/68000/no FPU/
  4 MB/512×342×1 correctly; 20 queries take **31 ticks (~26 ms/query)**, retained
  heap zero. Normal startup counter **358 ticks (5.97 s)** versus earlier 320;
  successive unmatched launches, not a controlled attribution of 38 ticks.
  Post-launch heap 422,306; after loading the capability command 421,006 bytes,
  owned remainder zero. Baseline-after-command 423,998: ~2.9 KiB difference,
  including loaded resources/heap state, not a pure allocation measurement.

Normal 1531 resource fork **942,730 bytes**, +2,137 bytes over accepted 1525;
final 1533 is **942,884 bytes** (+2,291). Existing
768 KiB partition, 64 KiB stack reserve and all drawing buffers unchanged. Hardware
query and progress formatter allocate no new buffers; preference menu/state adds
small bookkeeping. Physical Preferences popup selects Command; the checkbox changes the whole theme
immediately while retaining the unfinished command. Command-A then inserts at the
beginning, and physical Ctrl-C cancels. Command-C/V still copy/paste selected
text in Command mode. Shift-Up shows history without an editable prompt; typing
X returns to `echo liveX`, preserving the trigger. Option-C interrupts the physical
output loop with status 130. Both light and dark native pixel/bar checks pass (22 bursts + 20 views, zero
pixel mismatches; 147/149 scrollbar checks, zero mismatches). Physical nano was
initially absent: those earlier command-not-found captures are failures, not
editor acceptance. Installed the unchanged genuine nano 9.2 CLX from the
validated Snow fixture through wget/appinstall. Final editor results follow.

## Final counter correction and build

The original formatter could overflow a 32-bit multiplication near 2 GiB even
though 64-bit-host arithmetic passed. Split the remainder into 64 parts before
multiplication; largest intermediate stays below 2^32. Explicit uint32_t locals
make host tests exercise the target width. 41,011 sampled values match a 64-bit
reference, plus boundary cases; native Plus and physical SE probes pass all eight
cases through 4 GiB counters (`se-size32.png`, `snow-final-build.png`). This is a
counter test, not a claim to transfer a 4 GiB HFS file. No floating point/64-bit
runtime division or additional allocation. Code growth: 40 bytes per command.

The three commands were rebuilt and explicitly replaced on the SE. Previous
backups were moved to `/etc/clarice/ux-progress`, then appinstall retained each
just-replaced command as its new recovery copy. Complete normal 1533 MacBinary:
`work/ux-six/build/Clarice.bin`; package/index `work/ux-six/source33/`.
The immutable 1531 private update package is retained as tested history; it
predates this formatting correction. Source/build hashes are in `measurements.json`.

## Final physical acceptance and resumed loader profiling

The normal **1531 → 1533** update used `-y`, the private HTTP source and the
unchanged staging/replacement/recovery path. Staging reported **4190 ticks**.
At 166.10 seconds from the host script start the staged application was being
checked; at 357.50 seconds the captured screen showed the **1533 greeting and
prompt**. Command entry completed at 19.25 seconds. These sparse captures bound
completion but do not measure the exact handoff/relaunch duration. Quiet host
intervals avoided a premature response deadline; this run needed no helper
restart. See `se-final-update.json`, `se-final-update-mid.png` and
`se-final-update-result.png`. Previous working application/command copies retained.

An initial post-update keyboard attempt was sent to Finder after switching focus;
`se-final-nano-queued.png` is **not** editor evidence. After confirming Clarice's
foreground menu/window, native Ctrl-underscore queued during nano loading opens
Go to Line (`se-final-nano-queue-pass.png`). Ctrl-C cancels that prompt and Option-X
exits. Command mode opens the 8 KiB fixture, Command-W accepts `abc`, Return moves
the cursor to the match, and Command-X exits. Explicit search entry/result captures
are `se-final-nano-search-entry.png` and `se-final-nano-search-result.png`.
Files were not modified. Light theme/Monaco/Option mode restored; `se-final-ready.png`.

Existing production counters, ticks at 60 Hz, transcribed from the three
`se-final-nano-*-runtime.png` captures:

| Loader phase | First empty after relaunch | First 8 KiB | Repeat 8 KiB |
|---|---:|---:|---:|
| Total load | 356 (5.93 s) | 190 (3.17 s) | 190 (3.17 s) |
| Reading | 35 | 38 | 35 |
| Validation, metadata only | 138 | 108 | 107 |
| Imports | 4 | 4 | 4 |
| Relocations (includes structural validation) | 23 | 22 | 22 |
| Preparation / close | 139 | 4 | 5 |
| Verification record | 16 | 14 | 16 |
| Event servicing (included above) | 40 | 23 | 19 |

All use installed fast verification: header/payload counters zero, structural
validation 14 ticks within relocations. Coarse counters need not sum exactly.
These are **loader-only** measurements; module lifetime includes time spent
interacting and waiting for automation. No new total-to-editable-screen claim.
Snow final cold sample was 307 ticks (5.12 s), but heap/device differences prevent
deriving a universal emulator speed ratio from these successive runs.

All three exits return to **420,960 free bytes**, portable owned remainder zero.
Portable peaks: 74,572 bytes empty, 101,736 for both populated runs. No accumulating
loss; free heap is 3,038 bytes below the older 1525 post-command snapshot, including
code/resources and differing heap history, not a pure permanent-allocation delta.

The next focused performance investigation is **cold native allocation/preparation
and the warm relocation/import metadata pass**, not more glyph assembly. Warm
metadata work is over half the loader time; event servicing is included, so split
that cost before choosing a change. Prior Snow substage evidence attributes the
cold preparation penalty mainly to native image/workspace allocation. Physical
attribution needs confirmation. Preserve integrity/structural checks and avoid
preloading or permanent caches. No loader/nano optimisation was added here.

## References and limits

Native keyboard-resource lookup follows Apple's develop Q&A (June 1993):
[Apple develop 14](https://vintageapple.org/develop/pdf/develop-14_9306_June_1993.pdf)
Hardware detection follows native SysEnvirons/Gestalt semantics, not CPU/FPU
inference. System/ROM patches, new bitmap caches and new dependencies are absent.
Snow update transport fixtures exercise the production HTTP parser and native
HFS/relaunch but substitute a file-backed transport; physical MacTCP results must
be reported separately. Remote captures cannot prove absence of brief flicker.
