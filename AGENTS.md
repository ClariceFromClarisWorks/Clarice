# Clarice engineering requirements

Authoritative direction: [revised brief](references/engineering-direction-2026-11-10.md).
Read [current state](outputs/CURRENT-STATE.md), [architecture](outputs/ARCHITECTURE.md)
and [roadmap](outputs/ROADMAP.md) before implementation. Earlier instructions and
historical phase ordering are superseded by this brief.

- Baseline: Macintosh Plus, 8 MHz 68000, 4 MB, System 6.0.8. Preserve SE/later 68k and single-Finder compatibility.
- Clarice is a native Macintosh CLI, not a replacement OS. Target separate external application processes under MultiFinder/System 7 and in-application execution under single Finder, sharing implementations where practical.
- Keep idle memory small; allocate foreground working memory according to need and safely available resources, retaining error/cancellation/file-operation reserves. Release invocation-owned resources.
- Measure significant changes. Prefer efficient algorithms and measured 68000 assembly in identified bottlenecks; retain practical C interfaces/upstream code.
- Nano acceptance target: submission to fully drawn, keyboard-responsive editor within three seconds for empty and ordinary 8 KiB text files. Separate first/subsequent launches and emulator/physical evidence. This is not yet achieved.
- Use genuine upstream applications and required libraries; preserve authorship, licences and separable adaptations. Build reusable file, stream, invocation, terminal and networking services. Never fake success or invent user/group/kernel semantics to claim compliance.
- External applications must eventually be independently buildable, packaged and replaceable through a versioned runtime contract. Startup must not overwrite newer independent installations.
- Preserve newer local work. Inspect code/tests before assuming a feature is absent. Maintain concise current-state documentation and explicit compatibility gaps.
- Keep tasks focused. Do not rebuild for documentation-only changes. Use focused regression tests for implementation changes and distinguish compilation, host tests, emulator execution and hardware validation.
- Do not push commits, publish releases or begin subsequent roadmap tasks without explicit authorisation.
- Current authorisation is **Task 2 / roadmap 1.2 only**: safe independent command installation/replacement, minimal runtime compatibility checks and focused loader instrumentation. Preserve unpublished editor work. No worker architecture, live pipes, ncurses changes, general SDK, pushes or releases.

At implementation-task completion report changes, tests, relevant measured costs,
limitations, decisions for review and the recommended next step.
