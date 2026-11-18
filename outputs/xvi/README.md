# xvi port investigation

18 November 2026. Branch `codex/xvi-investigation-20261118`, separate worktree
`Clarice-Xvi`, based on committed `7e3e036`. No merge, push, release or physical SE
access. Other agents' worktrees, Snow processes and disks remain untouched.

## Candidate and engineering decision

This is Chris and John Downey's portable **xvi**, descended from Tim Thompson and
Tony Andrews' **STEVIE (ST Editor for VI Enthusiasts)**, with Martin Guy's later
maintenance. It includes the Atari ST/TOS port (`src/tos.c`, `tos.h`,
`makefile.tos`); it is not an X Windows vi application. Upstream also contains
optional Unix/X frontends, which this build does not compile.

Pinned source: https://github.com/martinwguy/xvi at
`109d2f39849ec81cc36001dbc341aaa45ff7e8ce` (31 March 2021). The source's version
string is **2.50**, while its unreleased notes describe 2.51; we identify the
commit rather than claim a released 2.51. Copying terms are preserved in
[licenses/xvi/COPYING](../../licenses/xvi/COPYING). Original source is fetched
unchanged into `work/xvi/upstream`; the preparation script copies it and marks
the sole header integration change. All editing, regex, buffers, undo, commands,
preservation and screen-difference algorithms are genuine upstream code.

The existing default VirtScr frontend and Atari port establish a small documented
platform boundary. Clarice binds that boundary to its existing screen/streams,
invocation-owned allocator, HFS and safe-save services. No new loader, ncurses,
resident editor, worker, signals/process emulation or terminal subsystem is needed.
The only new dependency is the already implemented runtime 1.9 remote-wire API.
Current committed base predates that API; it exists in the newer local Core tested
here and must accompany a future merge. Core and telnetd source are unchanged.

Cost/benefit: 99,300 B loaded image, 155,908 B CLX package, 4,404 relocations and
58 existing imports. Invocation-owned memory is measured below, not capped.
No new idle allocation. Remote mode uses the service's existing demand-allocated
4 KiB raw queue. Smallest useful experiments are full submission/edit/save/exit,
ordinary 8 KiB startup, subsequent launches and post-exit memory, not isolated
render or loader loops. Physical performance remains unmeasured.

## Reproduction and isolation

```
scripts/build-xvi.sh
clang -std=c99 -D_DARWIN_C_SOURCE -fsanitize=address,undefined -g \
  tests/xvi/save.c -o work/xvi/save-test
work/xvi/save-test
python3 tests/xvi/publish.py PRIVATE_REPOSITORY work/xvi/xvi.clx VERSION
```

Pinned Retro68 container, `-m68000 -Os -std=gnu99`, no network during compilation.
`--relink` is a developer-only shortcut after platform edits; use a full build
when upstream headers or compiler flags change. The normal script invalidates the
previous package before compiling. Preparation never patches another worktree.

The test guest owns `work/xvi-snow/{system,bench}.hda`, PRAM, workspace and separate
`XviSnow.app` bundle (`org.clarice.XviSnow`). Source media were copied from a stopped
fixture, not another agent's running disk. Loopback forward **127.0.0.1:23239** to
**10.0.0.2:2323**; private HTTP **8093**. Existing authenticated test settings were
retained; no password changes or public service exposure. Source and network
fixture paths are local prerequisites, not redistributable ROM/System assets.

Installation used **`clarice update -y --source .../index.txt`**, adding xvi
revision **2026111841**. No offline injection of xvi or replacement installer.
The first HTTP attempt at Snow's gateway address failed; using the host's actual
address succeeded. This is NAT fixture behaviour, not a Telnet editor failure.
Core 2026111823 / telnetd 2026111814 were already current on the copied fixture.
Full `appinstall check /bin/xvi` passed after repository installation.

## Current boundaries

- Core currently misclassifies `/dev/null` and `/dev/zero` as ttys; redirected
  editor startup on those devices is not safely rejected. See Telnet findings.
- Local Clarice screen or ANSI Telnet with BINARY negotiated both ways. Remote
  size is explicitly 80x24; no automatic resize/NAWS. No ncurses requirement.
- Existing in-process CLX execution only. No new MultiFinder/System 7 worker
  claim; neither System 7 nor physical hardware was exercised.
- Byte-oriented upstream text, Unix LF default; upstream options retain their
  meaning. Not a Unicode editor; binary-file fidelity is not claimed.
- HFS saves use `cli_replace_begin/commit/abort`, including upstream preservation
  files. Append (`:w >>`), shell/filter commands, directory changes and wildcard
  completion are explicitly unsupported. No fake Unix users, permissions or jobs.
- **Production blocker:** remote disconnect/cancellation currently bypasses upstream
  hangup buffer preservation; shared cancellation can also abort attempted recovery
  I/O. Unsaved edits are not guaranteed recoverable. See the Telnet/runtime review
  recommendation; this is not a completed faithful recovery port.
- Preserved recovery files are named `/tmp/xvi-...`; upstream controls their
  deletion. No new crash-recovery index/UI or power-loss atomicity claim.
- `--help` and `--version` are available over ordinary streams. Upstream `:help`
  still requires its separately installed `xvi.help` text; this prototype package
  contains the executable only. This is an investigation, not production completion.
- First display adapter emits redundant cursor/colour escapes. The captured
  transcript demonstrates the cost; upstream termcap's cursor/attribute tracking
  is a proven starting point for a focused next improvement.

## Evidence

[Build/import inventory](build.json), [safe-save host tests](save-host.txt),
[first guest edit](guest-smoke.json), [guest regression](regression.json), and
[Telnet integration findings](TELNET-FINDINGS.md).

Host sanitizer checks exercise the actual save adapter's begin/open/write/close/
commit failure paths and append refusal; they are not native HFS durability tests.
Guest evidence and timing limitations follow.

### Observed guest results

- Authenticated Telnet: new file insert/save, search/change, substitution, undo
  followed by save/readback, arrows, EOF movement and an 8 KiB edit all pass.
  Locked-file and `-R` refusal preserve the original contents. Type/creator and
  flags before/after an APPL-resource fixture save match. Exact resource-byte
  comparison is recorded separately below.
- `--help`, `--version`, unknown long-option status 2, stdout/stderr separation,
  and ordinary-file stdin rejection/status 2 pass. `/dev/null` rejection **fails
  because of Core's isatty defect**; the initial suite stopped there, and the
  remaining 25 observations were run separately in `more.json`. Do not describe
  the original suite as an uninterrupted full pass.
- Disconnect during unsaved editing: subsequent authentication, zero tracked
  survivors and another successful saved edit pass. Ctrl-C closes the connection
  (not vi cancel semantics), and first immediate reconnect EOFs; later reconnect
  and all six measurement cases pass without restarting Core. See Telnet report.
- Local native screen: visually checked complete editor/status display, inserted
  `Local xvi works`, saved with Ctrl-[/`:wq` and restored the shell. The initial host Escape
  automation was not delivered reliably in this Plus fixture; its accidental
  command text was discarded before the successful repeat. Telnet readback
  and runtime snapshot are in `local-check.json`.

### Measured costs — emulator, not physical acceptance

The independent host ANSI emulator **pyte 0.8.2** checks the last visible row and
home cursor, then checks a rendered inserted character and updated cursor. Thus
startup is measured to a full screen with subsequent demonstrated input response,
not the earlier unreliable quiet-output heuristic. The measured file has 128 lines of 63 decimal digits plus LF, with ` END`
appended to its final line by the preceding edit test: **8,196 bytes (8 KiB + 4)**.
The earlier functional/open-repeat cases used exactly 8,192 bytes before that edit.

| Case | First run in measured batch | Subsequent runs | Keyboard response | Full open / insert / discard / prompt |
| --- | ---: | ---: | ---: | ---: |
| Empty buffer | 1.892 s | 1.866 / 2.084 s | 78–129 ms | 2.177–2.458 s |
| 8,196 B file | 9.080 s | 9.056 / 8.952 s | 128–164 ms | 9.631–9.685 s |

These are **host elapsed seconds in accelerated Snow**, with other agents' emulators
running. Fixed-maximum acceleration was selected but achieved speed varies with
host scheduling. They are not 8 MHz SE wall time, nor cold-boot/first-ever launches.
No three-second physical-editor acceptance is claimed. Complete-operation guest
counts were 160–171 ticks empty, 780–787 for the file, including key/quit handling;
loader 77–79 ticks. Full timestamped data: [screen measurements](screen-measurements.json).

Tracked invocation peaks: **23,728 B empty**, **45,996 B file**, zero retained
on every measured exit. Stable post-exit native free/largest heap **333600/331260 B**
with Telnet enabled. Add the **99,300 B image** and existing host/terminal/service
allocations; tracked peaks are not total peak native heap (e.g. Newlib buffers).
No new idle cache. Earlier heuristic timings in `measurements.json` are retained
as diagnostic evidence, explicitly superseded for startup by the screen oracle.

The first adapter emitted 1,247 B for the empty open/quit transcript versus
13,490 B for the file. Much is redundant absolute cursor addressing and colour
selection, demonstrable in raw transcripts. Do not attribute all file latency to
that without a measured comparison: reading/line allocation also need separation.
Reuse upstream termcap cursor/attribute tracking before any assembly or new cache.

### Review decision and recommended next step

**Feasible genuine port; not ready as production `vi` or for automatic merging.**
Keep the separately packaged `xvi` command and branch. Resolve shared runtime
isatty, orderly hangup preservation and general Telnet foreground-input/size/cleanup semantics in their existing
workstream. Then make a bounded comparison using upstream-style output suppression
and measure file construction versus transport, preserving safe HFS saves.
Package upstream help and close unsupported-option/recovery coverage before a
production command claim. Physical SE and System 7 testing remain deferred.

Final optional package **2026111843** changes only help text from the measured
1841 build: documents accepted-but-ignored `-w`, distinguishes upstream startup
errors from adapter usage errors, and explicitly warns of unsaved-buffer loss on
remote disconnect/cancel. Final package **156,044 B**, image **99,436 B** (+136 B).
All timings above are the measured 1841 package, not a fresh timing claim for the
text-only follow-up. The 1841 and 1842 binaries remain as guest recovery copies.

Final installed 1843 verification passed: full appinstall check, help/version,
completed repository catalog/no incomplete marker, another Telnet edit/save/readback
and zero tracked survivors. [Final-package observations](final-package.json).

### Offline resource-fork readback

After quitting Clarice and closing only this task's Snow workspace, hfsutils
exported the original APPL and edited copy from the stopped isolated HFS disks.
The **280,364-byte resource fork's first 16-byte structural header and every byte
from offset 128 onward match**, including application-reserved area, resource data
and complete map. The edited data fork is exactly `Fork preservation\n` (18 B).
APPL/CLRC type/creator and creation date match. [Raw hashes/differences](forks.json).

The initial whole-fork equality assertion **failed**: 39 bytes at offsets 48–117
changed, all within the documented **system-reserved 112-byte area at 16–127**.
They include staging filenames and Finder information. These are consistent with
native file-manager metadata updates; no byte-identity claim is made for this area.
The source/copy Finder flag difference already existed immediately after `cp` and
was unchanged by xvi save, as the two `lifecycle.json` snapshots show.

Format references: [Inside Macintosh I, Resource Manager file format](https://vintageapple.org/inside_o/pdf/Inside_Macintosh_Volume_I_1985.pdf),
[resource-fork layout and system metadata description](https://formats.kaitai.io/resource_fork/).
This is a content/metadata preservation check, not power-loss or physical-hardware
acceptance. The native safe-save implementation was reused unchanged.

The isolated emulator and HTTP server are stopped. Test disks, exact packages,
pristine upstream source and local build/readback artifacts remain under `work/`.
No other agent's process was stopped. No main-branch change, merge, push or release.
