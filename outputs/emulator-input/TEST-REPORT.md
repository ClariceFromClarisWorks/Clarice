# Plus emulator input verification

**Version correction (8 November 2026):** subsequent installation-media and Finder verification identifies System 6.0.8 despite the 0607 environment-query result. See [the 6.0.8 verification](../system608/TEST-REPORT.md). Raw historical results below are preserved.

8 November 2026. Project-local Snow 1.5.0, base a921b108ef8bce8ba43fce121d46df0ea9d219af, Macintosh Plus, 4 MB, MultiFinder, System 6.0.7. These are emulator functional results, not physical hardware tests or System 6.0.8 validation.

## Change

The previous Plus keyboard translator discarded arrow and Control keys. The macOS frontend also ignored modifier-change events. The local patch:

- Emits make/break keypad packets for Up/Down using the 0x79 prefix.
- Handles macOS modifier changes explicitly, without duplicate modifier key events.
- Maps host Control to guest Option on non-ADB models. Clarice already interprets Option-C as interruption. Command remains Command, preserving Command-C for Copy and Command-period for interruption.
- Clears queued undelivered key events on focus loss, releases held keys, and completes pending keypad packets in order.

Host Control therefore behaves as Option in other applications on these non-ADB guests too. This is an emulator convenience mapping, not an original Plus hardware Control key or full Unix signal support. ADB models retain Control semantics, but were not guest-tested here. Existing emulator saved states were not tested for compatibility with the changed keyboard state; use a clean boot.

The raw protocol includes horizontal arrow packets, but this System 6 guest translated Left/Right incorrectly during an extra check. Host Left/Right remain ignored on non-ADB models, as before this change. Only Up/Down are enabled by this patch. No claim of complete M0110A keypad emulation is made.

## Verification

The pinned release build succeeds. Five focused Rust tests pass: arrow packet ordering and releases; ordinary key/modifier preservation; Option-C press/release ordering; focus release with queued keys; and focus release during a partially transmitted keypad packet. See tests.log.

Observed in Clarice using actual emulator UI input:

| Check | Result |
|---|---|
| Run `echo first`, `echo second`; type an unfinished `draft`; press Up | Recalls `echo second` |
| Press Up again | Recalls `echo first` |
| Press Down twice | Restores unfinished `draft` |
| Ctrl-C with an unfinished command | Records `^C`, clears input, returns a fresh prompt |
| Ctrl-C again at an empty prompt | Another fresh prompt; no literal C inserted |
| Ctrl-C during `while true; do echo active; done; echo bad` | Stops loop, does not print `bad`, returns prompt |
| `echo $?` immediately after interruption | Prints `130` |
| Command-period during a quiet infinite loop | Stops loop, skips trailing command, status `130` |
| Command-C while `pending` is being edited | Leaves input intact; does not interrupt. Full clipboard round-trip not tested. |
| Shift combinations for `$?` | Arrive correctly; no stuck modifier after release |

The terminal's bottom hint bar remains absent. No shell runtime or memory-cache changes were needed for this emulator input fix. No physical key-to-pixel latency measurements were made.

Mounted HFS access was also reconfirmed before the emulator update: `ls /mnt` listed the test volume, `cd /mnt/sys6unix/documents` succeeded, and `pwd` printed that full virtual path.

## Delivery and reproduction

The patched binary is installed only in `work/emulator/Sys6Snow.app`. Its original executable is retained as `work/emulator/Snow-before-input-fix`. An unused copied `hdd0.img` in that app bundle was moved intact to `work/emulator/bundle-hdd0-preserved.img` to allow ad-hoc signing; the guest continues to use `work/emulator/system.hda`. Original ROMs, original system disks, SkiFree and other emulator installations were not modified.

Use `scripts/build-emulator-input.sh` to reproduce the pinned source build and focused tests. The script does not install or launch the emulator. Patch: `patches/snow-plus-input.patch`. Upstream author/licence: SNOW-LICENSE.txt. Hashes and base commit: provenance.json. This is a local patch, not an upstream Snow release.

## Final installed build recheck

After the horizontal-key guard was rebuilt, the signed project-local bundle was installed and clean-booted again. Up recalled `echo final`; Down restored `draft`; Left/Right left that draft unchanged. Repeated Ctrl+C produced fresh prompts. Ctrl+C stopped `while true; do true; done; echo bad`, skipped `echo bad`, and returned status 130. Command+C preserved `pending`; Command+period cancelled it and returned a fresh prompt. All five input unit tests passed for this final build. The provenance hash identifies the installed, signed binary.
