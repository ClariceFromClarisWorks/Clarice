# Nano post-loader profiling — 16 November 2026

Physical timing/pixel/cleanup comparison and normal nano installation/save check passed.

Baseline 1535 and candidate 1536 enable existing opt-in nano instrumentation.
The command is separately installed as `nano-profile`; normal nano is retained.
Matched fixtures: empty, 8192 bytes / 128 lines, 8192 bytes / two 4096-byte lines.
The historical `ux-nano.txt` is actually 8192 bytes / 126 lines, confirmed with wc.

Timings use guest TickCount from shell submission to the first fully painted input
wait, before automation sends another key. Screen capture is afterwards; its
latency is excluded. Raw screenshots and transcribed JSON accompany this report.
A newly installed executable's first full-verification run is excluded from warm
installed-fast comparisons. These runs are warm application sessions, not reboot
cold starts. SEControl quiet mode suppresses heartbeats during startup.

Candidate changes are restricted to the existing editor strip: skip trailing-space
DrawText after clearing the full row; draw directly at the destination's word phase
when the existing 512-pixel strip can hold it. Other layouts retain QuickDraw's
original copy. No new buffers, allocation, direct framebuffer writes or clipping
bypass. Inverse spans and existing cursor XOR remain unchanged.

The `%zu` defect was an unadapted vsnprintf path in nano's statusline(), not encoding.
Extend existing newlib z/t/j format translation to vsnprintf/sprintf/vsprintf;
reuse the existing vsnprintf import. No nano source substitution or new ABI import.

Research used the locally pinned genuine nano 9.2 files.c/winio.c and ncurses 6.5
lib_refresh.c/lib_tputs.c. Nano batches wnoutrefresh then doupdate; our measured
public doupdate wrapper records one final refresh, not every internal ncurses call.
Draw/output counters are nested inside stage times and must not be added again.
Native allocation/read statistics include the entire invocation through exit.
Nano's 120-byte long-line growth is potentially costly, but matched results do not
yet justify changing it. Short lines draw more rows than two horizontally clipped
long lines; these are not equal-screen drawing workloads.

Focused host checks: 300 production/reference dirty-buffer pixel comparisons,
20,000 inverse/cursor-span cases, 200,000 parser/dirty-state cases, format adapters
including bounded truncation and escaped percent. Safe-save actual implementation
passes faultable HFS model: 27 failures, 27 interruptions, paired rollback faults,
read-only/locked/cancellation, forks/metadata and 20 repeated replacements.
Host models do not establish physical power-loss guarantees.

Deployment evidence caveat: the first candidate-command download refused an
existing destination; an orphaned host automation child then interfered with the
retry. Both were stopped, the disposable unsaved buffer cancelled, and screenshots
prefixed `excluded-` are not acceptance or timing evidence. The replacement uses
fresh `np36a.clx`; old profiling-command recovery is retained separately. No normal
nano or user document was changed by the interrupted run.

Normal build 2026111537 (profiling OFF) and corrected nano both build successfully;
Snow opened the 8 KiB fixture and returned with owned remaining=0. Diagnostic
1536 is the physical comparison app. Normal nano's package grows 256 bytes and
its loaded image 196 bytes; no new idle allocation. Repeated Snow saves preserve
exact bytes, resource fork, type/creator, flags and creation date; cancellation
returns 130 and does not save the unsaved prefix. See save-validation.json.

## Measured result (physical stock SE, 60 ticks/second)

| Fixture | Before ticks | After ticks | Mean seconds before → after |
| --- | --- | --- | --- |
| Empty | 346, 336 | 306, 306 | 5.68 → 5.10 |
| 8 KiB / 128 lines | 558, 545 | 515, 513 | 9.19 → 8.57 |
| 8 KiB / two lines | 442, 439 | 413, 413 | 7.34 → 6.88 |

Total startup improves 10.3%, 6.8%, 6.2% respectively. Keep the focused change.
Drawing totals fall 62–65 → 34–35, 103–105 → 69–71, 69–71 → 39–42 ticks.
Short-line glyph work: 46–47 → 24–27 ticks; CopyBits 26–29 → 7–9 ticks.
Warm loader remains 84–87 ticks; receipts approximately 14–15 ticks. No policy change.
SE frame comparison: zero changed pixels in editor body, scrollbar and shortcut
bar, excluding the intentionally corrected status line. Static captures cannot
prove absence of every transient flash. Both themes are exercised in Snow and
host pixels; physical timing uses the unchanged light theme.

Peak portable owned bytes remain 75,112 empty / 101,736 short / 95,122 long;
remaining=0 on every accepted physical run. Candidate idle post-command free heap
is stable at 418,906 bytes across cases. The prior instrumented app was 419,010;
the 104-byte difference is app-build overhead, not accumulating command memory.
No bitmap cache, additional buffer or permanent heap reservation was introduced.
Normal configuration omits profiling state; normal Snow free heap is 421,078.
Those different builds' heap totals are not a leak comparison.

Snow warm totals: empty 318 → 277; short 489/473 → 440/427;
long baseline sample 394 → 353/352 ticks. Snow is not the hardware authority.
First full-verification cold samples are excluded from these warm comparisons.
Fresh matched cold physical launches were not repeated tonight. Previous cold
image/metadata preparation 132–139 ticks versus 3–5 warm remains outstanding;
normal Snow cold preparation remains 120 ticks. No permanent preload is proposed.

## Remaining costs and next focus

1. Physical short-file open/buffer construction: 166–173 ticks (~2.8 s), with
   77–99 inside the byte/line loop. Native reads only 6–10 ticks across invocation;
   allocations, scanning, metadata/setup and cooperative work need finer separation.
2. Measured final ncurses doupdate: 97–101 ticks (~1.65 s), including terminal
   output and some rendering. It is one refresh, not an excess refresh-call count.
3. Warm CLX: ~85 ticks (1.42 s); cold native allocation remains a separate cost.
4. initscr/screen-open phase: ~31 ticks (0.52 s). Terminal byte feeding ~17 ticks
   in total. Editor painting ~70 ticks is nested across these phases.

Do not sum nested counters. Further work should profile the file/setup path and
ncurses final refresh before another renderer change. Three-second startup remains
unmet. No new utility, alternate execution model, receipt removal or cold-loader
redesign was undertaken.

## Final physical acceptance

Normal corrected nano installed via appinstall; previous nano retained at
/bin/.clx-nano.old. It edited a new disposable /tmp/se-save36, showed `Wrote 1 line`,
exited with owned remaining=0, and cat returned `SE save test`. Physical forks/
metadata on an existing file were not newly tested; that regression was executed
and byte-checked in Snow as above. The SE stays on diagnostic app 1536; normal
app 1537 is built and Snow-tested locally, not newly deployed to hardware tonight.
Clarice was closed to Finder after acceptance. No further SE testing is running.

Prior Git checkpoint remains intact; this focused experiment is preserved in the
working tree, with no additional push/release. The old greeting reports System
6.0.7; the configured target remains 6.0.8. This does not newly certify the OS build.
