# Clarice current state

Reviewed 11 November 2026. Start here; dated test reports remain historical evidence.

Checkout: `Documents/ChatGPT/Clarice` (renamed from `System6Unix`), branch
`codex/ncurses-port`, HEAD `ac3d15d`. That commit is the v0.3.0 baseline;
v0.4.0 publishes Task 2 and QOL-1/2/3A plus the experimental runtime source, preserving
the ncurses/nano work without distributing an editor binary. Task 1 changed documentation only. Task 2 adds safe updates; see
[implementation and validation boundaries](task2/TEST-REPORT.md). Old absolute paths in logs describe
the earlier checkout; emulator workspace paths/build caches need a separate audit
before reuse. No tests or builds were rerun during Task 1; Task 2 host/build evidence is separate.

QOL-3A selection now updates only changed highlight rectangles; [Plus guest measurements and limits](qol3a/TEST-REPORT.md).

QOL-2 input batching, changed-span prompt painting and fail-closed input limits are implemented; see [guest measurements and limits](qol2/TEST-REPORT.md).

| State | Verified scope and evidence |
| --- | --- |
| Implemented with selected host/Plus guest coverage | Loadable utility groups, HFS `/mnt`, symlinks, PATH, finite spool pipelines and statuses; see [utility report](userland-next/TEST-REPORT.md). These are bounded subsets, not full Unix tools. |
| Implemented with selected guest coverage | Wrapped interactive editing, solid cursor, bounded history/scrollback/typeahead, native Preferences; [interaction](interaction/TEST-REPORT.md), [preferences](preferences-text/TEST-REPORT.md). |
| Partial | mrsh parser with native execution adapter; selected shell fixtures pass, but no POSIX conformance, jobs or live pipes. One True Awk lacks `system()` and command pipes. |
| Experimental, guest probe executed | Actual ncurses 6.5 linked to a loadable demo, native screen ownership, partial terminal modes and per-command environment/signals. [Evidence and version boundaries](nano/NCURSES-TEST-REPORT.md). Not a completed general curses runtime. |
| Experimental, linked only | Nano 9.2 tiny/non-NLS/non-UTF-8 and GNU helpers link as a CLX command. `work/nano/link.log` and `nano.clx` exist. Last `nano --version` guest command was submitted but its result was not observed before UI access ended. No verified nano editing/saving or launch measurement. |
| Implemented, host and Plus guest tested | Safe startup installation, explicit replacement/repair/recovery and runtime 1.0 requirements; [Task 2 report](task2/TEST-REPORT.md). |
| Planned | Adaptive budgets; worker applications; reusable complete portable I/O; nested commands, live pipes, completed awk, SDK, vi, TCP/Telnet/OpenSSH/SFTP. Serial source exists, but no verified usable connection. |

## Costs actually known

- Current `SIZE` request: 768 KiB; app reserves 64 KiB for its stack. Source-port
  allocation ceiling: 96 KiB. This is a current implementation limit, not the new
  adaptive-memory policy. Neither number measures the whole machine's free RAM.
- Screen history: fixed 16,896 bytes; optional rendering cache: 29,696 payload
  bytes. Editor grid is allocated on ownership, not as a permanently loaded editor.
- First ncurses probe: 100,424-byte loaded image, 28,806-byte owned-allocation peak,
  zero tracked remainder on normal and cancelled runs. Loader times 338/277 ticks
  (about 5.63/4.62 seconds at 60 Hz). These are **ncurses demo load times**, not nano
  launch measurements; they already expose a risk to the three-second target.
- Later session observations: demo image 103,300 bytes; peak 28,931; remaining 0;
  same free heap 441,646 after normal/cancelled runs. No physical Plus evidence.
- A linked file's disk size includes relocation/import records and is not its RAM
  footprint. No end-to-end nano performance result is available.

## Current stopping point

QOL-1 (11 November): fixed default-volume catalog enumeration after resolving
`/mnt` paths. Focused host and Plus guest regressions pass; see
[QOL-1 evidence and limits](qol1/TEST-REPORT.md). No Task 3 work or publication.


Task 2 / roadmap 1.2 implementation is complete, with host fault tests, 68000 CPU
fixture execution and cross-build evidence. Automated Plus guest acceptance now passes 43/43 extracted-result checks; ncurses
loader totals are 313/279/278 ticks, dominated by validation/checksum. See
[guest evidence](task2/guest/ACCEPTANCE.md) and [the report](task2/TEST-REPORT.md).
Stop here: no Task 3 work, editor continuation, push or release is authorised.

## Task 1 validation

Documentation reconciliation completed 10 November 2026. SHA-256 comparison against
an inspection-start manifest confirmed all 294 files under `src/`, `scripts/` and
`tests/` unchanged, including unpublished work. Local Markdown targets in the
reconciled documents resolve and `git diff --check` passes. Historical architecture
notes are retained under `outputs/history/`. No runtime builds, emulator tests,
commits, pushes or releases were performed for this documentation-only task.
