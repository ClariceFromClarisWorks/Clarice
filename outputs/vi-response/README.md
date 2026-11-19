# BusyBox vi cursor-refresh experiment

Accepted source base:3a34f50; upstream BusyBox1.37.0/GPLv2 retained through the
existing pinned archive and minimal patch. No downgrade or changed feature flags.

`refresh__clean` is invocation-owned. Erasures and every command except proven
command-mode h/j/k/l, arrows, space and numeric prefixes invalidate it. Invalidation
occurs before and after execution, so nested refreshes and queued edits cannot
incorrectly make a later motion clean. Viewport/offset changes and forced/resize
refreshes keep upstream formatting. Cursor/status generation remains upstream.

Normal executable:51392B vs51244B; loaded image36336B vs36176B. Working peak
23396B vs23394B on the matched8KiB local fixture; no new allocation. Profiling is
opt-in with `CLARICE_VI_PROFILE=1 scripts/build-busybox-vi.sh` and writes only to
stderr before runtime cleanup. An earlier diagnostic printed after cleanup and
crashed on exit; that failed build is not retained as acceptance evidence.

`refresh-guest.json`:52 actual Snow Telnet document/cursor comparisons against
forced Ctrl-L redraw, including undo, paste, repeat, settings, horizontal/vertical
scrolling, long lines, substitutions and queued edits followed by motions. Final
harness waits for stable output; an earlier long burst was sampled prematurely.
The genuine modified editor runs through the existing native runtime; pyte is
only the host display oracle. No network transport change.

Host actual growth/OOM/overflow, safe-save and recovery tests pass. The companion
Core branch `codex/vi-response-audit-20261119`, `outputs/editor-response/REPORT.md`,
contains local-window response/processing/redraw measurements and normal-build
exact-file/cancel/recovery evidence. Combined response:232–233→83–85ms vertical,
180→52ms horizontal in Snow. Measured vi-formatting752→61ticks. The requested
SE×1.2 estimates are not hardware results; one-tick response remains unmet.

Existing primary vi and all physical/shared packages remain untouched. No push,
release or normal repository publication. The experiment is installed only as
`vi-fast` (profile) and `vi-final` (normal) in the isolated Snow fixture.
