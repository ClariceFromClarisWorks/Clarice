# 1710 — physical renderer acceptance in progress

Same renderer as1709, with manual diagnostics instead of an automatic startup
batch. The 0.5 release candidate is unchanged. Physical stock SE/68000/4MB/
System6.0.8 was updated through the existing updater and relaunched. Prior apps
and recovery copies are retained; no old build was renamed or deleted.

MacBinary287872B, SHA256605047524b3f601976625dd66c242f227b33853477cb3379f451174318c11c2e.

## Matched physical timings

Each sample totals24 completed large history jumps, Monaco9/80columns, paired
within the same executable. Guest ticks exclude fixture/report creation. SEControl
can add noise at yield boundaries. These are not whole-command or visual timings.

| Theme | Per-row samples | Direct composition samples | Median reduction |
| --- | --- | --- | --- |
| Light |543,549,562|440,477,447|549→447 ticks,18.6%|
| Dark |612,630,613|494,493,525|613→494 ticks,19.4%|

Average per completed jump in these batches: about381→310ms light and426→343ms
dark using60ticks/second. Do not treat earlier416ms from a different workload as
this experiment's matched baseline. Sparse listing medians299→285/347→333;
three-sample variation prevents a precise small-gain claim.

For23 staged frames: composition262–278ticks light/262–281dark; presentation
30–40/36–37; dark inversion43–48; allocation0–1. Glyph composition remains the
largest measured phase. Counters overlap with whole-operation totals.
[Raw reports and parsed trials](physical/timings.json) preserve every sample.

## Correctness, memory and interaction

- Snow manual35-case pixel probe passes full/narrow/hole/occluded clipping,
  themes, selection, dirty/purged/OOM staging; subsequent commands recover.
- Physical35-case pixel probe also passes. Long uninterrupted diagnostics
  outlasted SEControl's response timeout; the report completed and the user
  reopened the helper. This is not evidence of a Macintosh crash.
- Physical22 scrolling bursts+20 views,147 scrollbar comparisons: zero
  mismatches. [Raw scrolling report](physical/native-scroll-report.txt).
- Physical native-pressure probe passes purge, locked refusal, unlock recovery,
  exhaustion and recovery. Owned0, freeheap378974→378974; largest375910→375752
  (−158B). No claim of unchanged fragmentation. Payload46,208B plus Handle
  bookkeeping, allocated on demand and purgeable; no extra buffer in1709/1710.
- Actual ls listing, mouse selection and dragging the scrollbar work. Typing
  returns from historical contents to live input and preserves the triggering
  text; echo LIVE-RETURN executes. Both Ctrl+C and Command-period interrupt
  output loops with status130 and subsequent successful commands. Runtime owned
  remaining0; nativefree354708/354706, largest351522 in these two checks.
- Snow Telnet connects to the restricted host PTY, runs full-screen less, exits,
  reconnects, and locally cancels back to Clarice; host children reaped, tracked
  remaining0. Automation dropped some follow-up letters, so exact Telnet cancel
  status was not captured cleanly. No networking source changes.
- Existing nano edit/discard/reopen and terminal restoration evidence is in
  ../direct-1709/interactive. No new editor/save/performance claim.

A boxed-minus mouse pointer was reported by the user. Movement-only captures
show it followed the mouse; Finder switching restored the arrow, including on
return to Clarice and another scrollbar drag. The trigger remains unresolved.
[Evidence](physical/pointer/README.md). No cursor-reset workaround was added.

Human observation of brief flicker/top-down wiping remains pending. Whole-view
QuickDraw is not VBL-atomic. PowerBook acceleration/resize and wider application
pressure remain unaccepted. This is an isolated experiment, not an RC promotion.
