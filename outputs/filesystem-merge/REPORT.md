# Filesystem development integration — 18 November 2026

Merged `47eb033` by fast-forward into `codex/coherent-deployment` from `7e3e036`.
The existing 35 tracked work-in-progress files and 13 untracked source/test files
were preserved. All untracked source files were verified byte-for-byte unchanged;
the restored tracked overlay was verified against the tested integration copy.
Existing work and integration resolutions remain unstaged and uncommitted.
No push, release, installation or emulator disk mutation was performed.

## Integration decisions

- Retained nano buffered input and metadata resolution improvements, serial service,
  remote Kermit binary transfers, session wire API, and runtime **1.10**.
- Combined source version is **2026111841**. The historical fix-only version1840
  and runtime1.8 in the original report describe that separate baseline.
- Preserved cwd relocation across saved/session contexts together with newer wire
  callbacks. Preserved serial port ownership checks and remote Kermit support;
  remote nano retains its explicit local-input diagnostic and help exemption.
- Kept Telnet cursor editing together with binary-mode interrupt handling.
- Fixed the host service test helper to feed the NUL-valued Telnet BINARY option
  by explicit length; strlen had silently truncated that negotiation fixture.
- Retained both branches' current-state entries and added integration context.

## Validation

Passed on the combined source in an isolated integration worktree:

- Filesystem regression and old-code negative control: empty mkdir operands,
  locked-source mv, native failures, overwrite/cross-volume behavior, cwd rebasing,
  saved/session contexts and nonmutating path-length preflight.
- Session, authentication, Telnet service/cursor/BINARY and native serial host
  regressions, including delayed-abort ownership and cleanup.
- Native cross-volume rename/recovery/flush regression and wrong-volume negative control.
- Metadata resolver paths/links/volumes/errors and 3-to-2 catalog-call check.
- Nano buffered getc boundaries, binary bytes, pushback, EOF, interrupted refill
  and recovery under ASan/UBSan.
- Retro68 68000 Core application build and telnetd/seriald CLX builds.
- Changed-source whitespace checks and final Git/preimage verification.

Logs are alongside this report. Combined service images are telnetd **40784 B**
and seriald **30060 B**; these include existing development features and are not
an isolated measurement of the filesystem patch cost. Prior fix-only costs and
35 Snow cases plus two cursor cases are in [the fix report](../filesystem-fixes/REPORT.md).
No fresh combined Snow run, physical-hardware performance measurement, or
MultiFinder acceptance is claimed. Existing compiler warnings remain.

## Recovery and next step

Original tracked work is retained in recovery stash
`77ab28e656e99ee105c1fa64cdce3fe158e7c747` (`pre-filesystem-merge-20261118`).
It is already restored in the working tree; do not apply the stash on top of it.
Preimage copies, hashes and the original patch are also retained under
`Clarice/work/filesystem-merge-20261118`.

Recommended next step: run the combined development candidate through Snow
before packaging, then review/commit the existing unfinished work in its owning
tasks. Telnet tab completion, immediate single-listener reconnect limitations,
and remote nano input ownership remain as documented in the fix report.
