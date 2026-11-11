# QOL-2 — keyboard latency and input safety

11 November 2026. Local implementation and focused validation; no push or release.

## Cause and changes

The old main loop processed one event and then painted. Native prompt rows were excluded from changed-span painting, so ordinary typing repainted the whole prompt row. The actual Plus guest reproduced lost input: only 219 of 240 successfully posted events reached Clarice at 144 WPM, and 75 at 720 WPM. This identifies a drawing/event-service bottleneck; it does not reproduce the reported 20 WPM threshold exactly.

The loop now services up to 32 pending events before painting; foreground yields service up to 16. Earlier queued input retains priority. Native prompt painting uses the changed text span, widened to cover old/new cursor cells, retaining the existing offscreen drawing path. No new rendering cache or persistent input buffer was added.

There were also independent silent-truncation defects: full command/typeahead/editor queues beeped and discarded input; paste inserted characters individually, flattened newlines and could leave an executable prefix. Command/paste/continuation and pending/editor queue failures now reject pending input, report the error and require Ctrl+C before submission can resume. Application queue overflow cancels the foreground invocation. Paste validates the complete TEXT scrap before insertion, normalizes CR/CRLF to LF, preserves multiple lines and never submits automatically. Return explicitly submits the complete buffer. The existing combined command limit remains 1023 bytes; larger pastes are rejected, not partially executed. Unsupported control bytes and allocation/read failures also reject the submission.

## Actual Plus guest measurements

Snow 1.5, Macintosh Plus, 4 MB, System 6.0.8/MultiFinder, normal emulated speed, native drawing. Disposable existing QOL-1 image copied to `work/qol2/system.hda`; the user's other Snow instance and its image were not modified.

Diagnostic builds use the guest Event Manager PostEvent trap, with identical 240-character patterned input before/after. Tick unit is 1/60 second. WPM assumes five characters per word.

| Input schedule | Before retained | After retained | Before worst event age | After worst event age | Paint ticks before → after |
| --- | --- | --- | --- | --- | --- |
| 103 WPM (7 ticks/character) | 240/240 | 240/240 exact | 51 (850 ms) | 4 (67 ms) | 1432 → 1021 |
| 144 WPM (5 ticks/character) | 219/240 | 240/240 exact | 148 (2467 ms) | 1 (17 ms) | 1237 → 876 |
| 720 WPM (1 tick/character) | 75/240 | 240/240 exact | 152 (2533 ms) | 2 (33 ms) | 339 → 175 |
| 16-event bursts per outer iteration | 34/240 | 240/240 exact | 84 (1400 ms) | 4 (67 ms) | 133 → 71 |

Event age measures posting to the beginning of event handling, not final visible pixels. The producer runs in the guest main loop and catches up scheduled posts; it is not an independent interrupt producer. Paint totals at high rates are not like-for-like work because the old build lost input. Burst elapsed values are not independent producer throughput. Raw evidence: [before](before.txt), [final](final.txt). An intermediate two-tick batch cap still lost burst events ([intermediate](after.txt)); replacing that cap with a bounded event count resolved this measured case.

Each stage had the same +26-byte free-heap delta before/after. Eight repeated 900-byte pastes released their temporary allocations. Final diagnostic free heap after safety/demo checks: 404,912 bytes; this is not the total application partition size. Production adds one `int` rejection latch, no persistent heap allocation. Diagnostic counters, producer and demo injection are compile-time excluded from the normal build.

A separate host CGEvent physical-key test delivered 203 key pairs through Snow in 18.059 seconds (approximately 135 WPM); the resulting 180-byte payload matched exactly, including its newline. Evidence: [output](host-input-result.txt). This complements rather than replaces the guest event tests.

## Safety and regression evidence

[Final guest log](final.txt) records 20 passing checks: cursor insertion; 900-byte paste; repeated-paste allocation release; CRLF/multiline retention; explicit submission producing two output files; oversize paste, typed overflow, continuation overflow, control-byte rejection and pending queue overflow; blocked suffix submission; Ctrl+C reset; editing; history up/down; completion; screen ownership; ncurses arrow/text/Ctrl+C bytes; editor queue overflow cancellation; and execution of the actual upstream ncurses demo through its input adapter (four events, exit q). [Guest screenshot](guest-complete.png).

Focused host checks passed: [paste with ASan/UBSan](paste-host.txt), [wrapped/multiline input layout](layout-host.txt), [output writer](output-host.txt), [screen parser](screen-host.txt), [platform input modes/signals](platform-host.txt), [shell and cancellation](shell-host.txt), and [QOL-1 HFS regression](qol1-host.txt). Host mocks do not establish Macintosh hardware behaviour. `tests/terminal/input-results.py` validates saved guest evidence; it does not run the emulator.

Normal build succeeded with both diagnostic options OFF: [build log](build-normal.log), [MacBinary application](Clarice.bin). The normal build was then launched in the guest: 205 physical key pairs in 14.072 seconds (about 175 WPM) produced the exact 180-byte payload and newline, followed by a clean exit to Finder. See [keyboard timing](normal-keyboard.txt) and [verified output](normal-input-result.txt). No utility, ncurses or nano payload was rebuilt for this task.

## Files and boundaries

Production: `src/app.c`, `src/editor-screen.inc`, `src/CMakeLists.txt`. Tests: `tests/terminal/input-layout.py`, `paste-host.py`, `input-guest.inc`, `input-safety-guest.inc`, `input-results.py`. Documentation/evidence: this directory and `outputs/CURRENT-STATE.md`. The start-of-task hash comparison confirms only the four intended pre-existing source/test files changed ([scope check](scope-check.txt)); Task 2, QOL-1 filesystem and unpublished port sources were preserved.

No physical Macintosh testing. Measurements cover native rendering, not every font/cache configuration. Existing finite System 6 and emulator queues cannot guarantee delivery of arbitrarily fast input before Clarice receives it. Snow's host clipboard-as-keystrokes mechanism lost a command prefix in one intermediate-build attempt; direct Macintosh ScrapManager paste passed the safety checks, and targeted physical-key automation subsequently delivered an exact command. Use real guest clipboard paste for atomic multiline submission; a stream of simulated Return keystrokes is not an atomic paste. The serial-specific input queue and broader canonical runtime input limits were not redesigned. No separate cursor/typeahead milestone, text selection, worker architecture or Task 3 work was undertaken.

No product decision required for this bounded fix. Retaining the 1023-byte limit with visible rejection is intentional; larger scripts remain files. Stop after QOL-2.
