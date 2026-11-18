# Filesystem audit fixes — 18 November 2026

Branch `codex/filesystem-audit-fixes-20261118`, created in a separate worktree from
committed `codex/coherent-deployment` (`7e3e036`) as explicitly selected by the
user. Private Core build 2026111840; runtime remains 1.8. No uncommitted nano,
serial/session or other-worktree changes were imported. No push, merge or release.

## Finding disposition

| Audit finding | Result |
|---|---|
| FS-001 empty `mkdir -p ''` succeeds | Fixed: rejects with invalid-name error/status 1; ordinary parents/dot paths retained. |
| FS-002 rename breaks current directory | Fixed for Clarice native directory rename/move: active, suspended subshell, local and remote cwd paths follow the mutation. A rename that would exceed the native path limit is rejected before mutation. |
| FS-003 locked-source move replaces target before failing | Fixed: source lock checked before any copy/replacement. Genuine native I/O/lock/catalog errors do not trigger copy/delete fallback. Normal overwrite and cross-volume regular-file move remain supported. |
| TN-001 silent remote nano refusal | Fixed for standard `nano`/`nano-profile` names and absolute paths: explicit local-input diagnostic/status 2 before module loading. |
| TN-002 transient immediate-reconnect EOF | Retained single-listener lifecycle limitation, now stated in banner/help. No unsupported instant-reconnect guarantee or second resident stream introduced. |
| TN-003 remote `telnet --help` blocked | Fixed: exact `--help` invocation bypasses input-ownership restriction; real interactive invocations retain it. |
| TN-004 ignored Left Arrow / no Tab completion | Left/Right, CSI H/F and SS3 Home/End and middle insertion/backspace now work; Ctrl-U and last-command Up/Down retained. Tab completion remains unimplemented, explicitly disclosed; stored tabs remain whitespace and echo as one blank cell. |

The two remaining capability limits are **not claimed fixed**. A second listening
stream could reduce reconnect races but costs native driver/storage memory and
changes lifecycle behavior; recommend a separate measured design review. Remote
completion should reuse the shell completer with safe session ownership and
input deferral, not call the parser from a transport callback. Full-screen remote
nano support is outside these diagnostic fixes and remains separate work.

## Implementation and proportionality

Native HFS rename/move remains authoritative. A successful directory mutation
rebases affected stored native/Unix cwd prefixes. Existing saved-shell and
supplementary-session objects are reused; one linked pointer per suspended frame
makes those existing contexts discoverable. Preflight uses the same conversion
without modifying any state, checks the 255-byte native-path bound and escaped
Unix buffer, then commits only after successful HFS mutation. No directory cache,
new runtime imports, per-path catalog revalidation or loader changes.

This repairs in-application mutations, including a remote rename affecting the
local shell and a rename inside a subshell. It does not detect unrelated Finder
or other-application renames under MultiFinder. Native directory-ID tracking for
those cases needs separate acceptance; no general Unix cwd/permission emulation
is claimed. Direct renames performed entirely inside another application's own
Toolbox code cannot notify Clarice's saved contexts.

`mv` now limits copy fallback to unsupported native moves or known cross-volume
regular-file moves. If an unexpected source-delete failure occurs after a copy,
it reports that the destination was copied but the source could not be removed.
This does not promise rollback after every late I/O failure or power loss.
Fork-preserving replacement implementation and lock semantics are retained.

Remote input editing reuses the existing bounded line buffer and genuine
libtelnet engine. Tail output batches contiguous bytes, preserving tab bytes in
the command while displaying them at a predictable cursor width. No new parser,
protocol library, bitmap or input allocation. No authentication policy change.

## Validation and costs

- New production-function host regressions under ASan/UBSan: empty mkdir,
  normal parents, locked source with absent/existing target, native I/O/lock/
  catalog errors, copy failure, normal overwrite/cross-volume move; cwd ancestry,
  saved/local/remote contexts, case boundaries, encoded names and overflow.
  A negative control against original `7e3e036` detects the actual overwrite bug.
- Existing deep traversal regression passes: depth 24, 25 allocation boundaries,
  cp/rm cancellation, copy failure and cleanup.
- Existing native volume/rename regression passes, including wrong-volume
  negative control. Session/authentication tests pass under sanitizers, extended
  with edited-input bytes, cursor bounds, Ctrl-U/interrupt reset, tabs and CSI
  parameter consumption; delayed abort retains driver-owned storage.
- Shell reference comparison: 31/31 plus 3/3 cancellation cases pass. Suspended
  frame tracking does not change environment/function/argument ownership.
- Core, file multicall image and optional telnetd compile/link for 68000 using
  pinned Retro68. Shipped upstream sources untouched. `git diff --check` clean.
- Disposable Snow Plus / 68000 / 4 MiB / System 6.0.8: **35 final guest checks plus two edited-command cases pass**; results are in
  `guest.json`/`guest.log`, edited wire/output in `cursor.json`, and hashes in
  `installed.json`. First-run evidence is retained in `first-run/`; final run
  adds the native-length preflight case and uses the final batched echo binary.
  Other agents' emulators and physical SE were not used.
- After Macintosh Finder Shut Down, independent exported MacBinary comparisons
  check actual moved data/resource bytes, type/creator and preserved timestamps
  (`forks/comparison.json`). No mounted/live-image inspection.

Matched baseline rebuild of committed `7e3e036`, same compiler/options:

| Artifact | Baseline | Fixed | Change |
|---|---:|---:|---:|
| Core MacBinary | 276,992 B | 277,504 B | +512 B |
| File utility CLX | 12,996 B | 13,268 B | +272 B |
| Optional telnetd CLX | 37,248 B | 38,748 B | +1,500 B |

Package size is not resident RAM. Core adds one 4-byte frame-list pointer and
4 bytes per existing saved/session state; telnetd adds a 4-byte cursor. Existing
native ring and line/queue buffers are unchanged. Cwd preflight uses two temporary
512-byte path buffers on the existing invocation stack, only on directory
mutation; no allocation or extra work on ordinary relative reads. Move adds a
source-catalog preflight before replacement; directory move adds a native catalog
read to distinguish files from directories. No startup scan/cache introduced.

First guest run's settled repeated cp/cmp/rm snapshots reached heap 335176,
largest 334852 bytes, owned survivors 0, unchanged across the last three repeats.
Final run: all four repeats held heap **335132**, largest **334628**, owned
survivors **0**. Raw snapshots remain in `guest.json`. No physical performance, exact
all-in latency, system-heap peak, System 7 or MultiFinder acceptance is claimed.
Concurrent accelerated Snow wall seconds are recorded for diagnosis only.

## Reproduction / handoff

Run from this worktree:

```
python3 tests/filesystem-fixes/host.py
sh tests/remote-sessions/run-host.sh
python3 tests/utility-correctness/file-depth.py
python3 tests/namespace/rename-host.py
```

`tests/filesystem-fixes/remote/check.py` expects disposable volumes, installed
candidate images, a fresh `fix1840` directory, owned PID file and loopback forward
on 127.0.0.1:23282. It deliberately renames directories and replaces files; it is
not a host-shell test or suitable for production disks. The script uses the
existing audit UI helpers for the local/remote cwd test. Disk staging helpers and
private fixtures are under `work/filesystem-fixes/`, not shipped runtime code.

Recommended next step: review/integrate these bounded fixes into the development
line, reconciling independent Telnet work rather than overwriting it. Retain
physical/MultiFinder acceptance and completion/listener enhancements as explicit
follow-ups. No installation on the physical Macintosh or publication performed.
