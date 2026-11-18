# Validated development checkpoint — 18 November 2026

User requested local commits of replacements and work that has passed. This
checkpoint preserves the integrated serial/Kermit remote service, approved
Telnet peer compatibility fix, nano metadata/buffered-input improvements and
physical/Snow evidence. Existing filesystem safety commits remain its base.

Fresh focused ASan/UBSan host checks pass: remote session ownership/authentication,
Telnet wildcard listening and rejection before bytes/auth/session, native serial,
resolver metadata reuse, and nano buffered I/O. Logs are adjacent. Existing
68000/Snow and physical results retain their original scope in the linked reports;
no new emulator or physical execution was performed for this commit. Nano's
three-second target remains unmet. Latest Telnet peer fix and newest filesystem
fixes still need physical acceptance. No merge, deployment, push or release.

BusyBox utilities and editor candidates already have clean local commits:
`a2a9595` (15-tool comparisons), `d9bf254` (BusyBox vi), `d20c41a` (Heirloom vi),
`79a1000` (xvi). wc/tr/sort replacement approvals stand; standard package switching
is not part of this checkpoint. Preserve encoding and spill-sort limitations.

Excluded: separate CLX-memory work in progress; unrelated generated root-worktree
artifacts; whole-collection recovery experiment. The recovery host runner currently
fails in tests/release/startup.py because shell_deployment_ready is absent from
this tree. Its modifications remain untouched and uncommitted, and are not
claimed as validated integrated behavior. Failed nano optimisation trials are
retained as evidence only; their rejected implementations are not included.
