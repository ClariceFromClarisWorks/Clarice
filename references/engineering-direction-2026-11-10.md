# Clarice — Engineering Direction and Task 1

We are establishing a revised engineering roadmap for Clarice, informed by a review of the public GitHub repository at:

https://github.com/ClariceFromClarisWorks/Clarice

The last reviewed public baseline was v0.3.0 (`ac3d15d`), dated 10 November 2026. The local checkout may be further ahead.

**First inspect the current checkout, existing documentation, `AGENTS.md`, and relevant implementation. Preserve all newer work. Do not duplicate functionality already implemented.**

## Standing engineering principles

These requirements apply to future Clarice development.

### 1. Hardware and operating systems

- Baseline: Macintosh Plus, 8 MHz Motorola 68000, 4 MB RAM, System 6.0.8.
- Macintosh SE and later 68k systems must remain supported.
- Preserve compatibility with System 6 running without MultiFinder.
- Under System 6 MultiFinder and System 7, external tools should eventually execute as separate Macintosh application processes.
- Under System 6 single Finder, retain execution within Clarice itself.
- Share application implementations and runtime services between execution modes wherever practical.
- Additional 68020/68030/FPU optimisations may be introduced later without breaking the 68000 baseline.

### 2. Performance is a requirement, not an afterthought

Clarice must remain fast and memory-efficient on the baseline hardware.

- Keep the idle terminal's memory footprint small.
- Avoid unnecessary persistent caches, large dependencies and permanently loaded applications.
- Allow demanding foreground commands to use substantially more memory when justified.
- Allocate working memory according to the task and safely available resources, rather than imposing arbitrary universal limits.
- Release command-owned resources when execution finishes.
- Preserve enough memory for error handling, cancellation and safe file operations.
- Measure performance and memory consumption before accepting significant architectural changes.
- Prefer efficient algorithms and measured 68000 assembly optimisations in performance-critical areas.
- Avoid premature optimisation of code without an identified bottleneck.

**Performance target: GNU nano should launch into a fully drawn, keyboard-responsive editor within three seconds on the baseline machine**, initially measured with an empty document and an ordinary 8 KiB text file.

Measure from command submission to usable editor. Include command loading, process startup, library initialisation and first rendering.

Report first launches separately from subsequent launches. Emulator measurements must be distinguished from physical hardware results.

The three-second target is an acceptance goal, not an existing verified capability. If it is missed, identify the bottleneck and propose a focused optimisation rather than declaring the performance requirement satisfied.

### 3. Clarice's architectural identity

Clarice is a native Macintosh command-line environment, not a replacement operating system.

The Macintosh OS remains responsible for its native hardware and system services.

Clarice provides a Unix-like shell, base userland, application runtime and compatibility services.

Distinguish between:

**Built-in shell functionality**

Small, frequently used shell operations remain inside Clarice's normal application footprint.

**External applications**

Tools such as grep, sed, awk, nano, vi, Telnet, SSH and SFTP should use the external-program interface.

Under MultiFinder/System 7, the target is independent application processes. Under single Finder, they must retain a suitable in-application execution path.

A single implementation of each application should be reusable wherever possible.

Do not implement unnecessary Unix kernel facilities, simulated users/groups or artificial permissions merely to claim POSIX compliance.

### 4. Unix application portability

The long-term aim is to make Clarice a practical target for portable Unix command-line applications that are suitable for classic Macintosh hardware.

Build reusable services for:

- File I/O, descriptors, streams and seeking.
- Filesystem metadata and directory operations.
- Memory allocation and cleanup.
- Arguments, environment and working-directory handling.
- Child-command execution and completion.
- Pipes and streaming I/O.
- Terminal input, screen control and terminal modes.
- Cancellation, signals where meaningful, errors and exit statuses.
- Networking, DNS and TCP connections.
- Runtime compatibility and dependency management.

Use native Macintosh APIs where they can provide the required behaviour.

Where Unix behaviour can reasonably be emulated, implement reusable compatibility services.

Where functionality is genuinely unnecessary, optional features may be disabled or carefully stubbed.

**Never return false success for operations that were not performed.**

Unsupported functionality must return meaningful errors or be explicitly excluded.

Do not claim full POSIX, Linux ABI or Unix process compatibility without providing those semantics.

### 5. Source portability and upstream fidelity

Continue using genuine upstream source implementations wherever practical.

Preserve upstream behaviour, licensing, attribution and clearly separated adaptation patches.

Avoid rewriting entire applications simply to make them fit Clarice.

Where upstream functionality is expensive, assess whether it is necessary and whether a smaller supported build configuration is appropriate.

Use C where it remains the practical implementation language for portable upstream code, but favour measured native/assembly implementations for low-level performance-sensitive operations.

The intention is to produce efficient software, not to optimise for source-code readability at the expense of performance.

### 6. Independent application updates

Clarice should not need to be rebuilt whenever an individual application is updated.

Develop a versioned external-program interface and installation mechanism.

Applications should be independently compilable, packaged and replaceable, with explicit runtime compatibility requirements.

Ensure the current bundled-tool installation behaviour cannot overwrite a newer independently installed tool during Clarice startup.

Native compilation on the Macintosh is a possible future milestone, but is **not required** for independently updating applications. Cross-compilation is acceptable.

### 7. Planned application milestones

The intended initial product scope includes:

- A coherent and well-tested base Unix-style userland.
- Completion of One True Awk, including `system()` and command pipes.
- Live pipelines with bounded buffering and correct cancellation behaviour.
- Genuine ncurses support.
- GNU nano.
- A faithful, suitably lightweight vi implementation.
- A usable Telnet client.
- A maintained OpenSSH client supporting modern secure connections.
- SFTP file transfers.
- A documented SDK allowing third parties to port additional applications without modifying Clarice itself.

SSH server functionality, background services, native compilation and BlueSCSI-assisted cryptography remain separate future possibilities.

Do not attempt to implement everything simultaneously.

### 8. Development and credit efficiency

Keep implementation tasks focused and independently reviewable.

- Read the existing implementation before proposing changes.
- Reuse working code, tools, libraries and tests.
- Avoid speculative abstractions.
- Do not rebuild the entire project or run every historical test suite for documentation-only changes.
- Add focused regression tests where implementation changes create relevant risks.
- Do not claim that a successful compilation proves guest execution or hardware compatibility.
- Document important trade-offs, particularly those affecting performance, memory or functionality.
- Keep a concise current-state document so later sessions can resume without rereading every historical report.
- Do not push commits, publish releases or start subsequent roadmap tasks without explicit authorisation.

At the end of implementation tasks, provide a concise summary covering:

1. Changes made.
2. Tests performed and their results.
3. Measured performance and memory use where relevant.
4. Remaining limitations.
5. Architectural decisions requiring review.
6. Recommended next implementation step.

---

# TASK 1 — Architecture and Roadmap Reconciliation

**Execute Task 1 only.**

The purpose of this task is to ensure Clarice's existing implementation, architecture documentation and future roadmap accurately reflect the requirements above.

## A. Establish the current implementation state

Inspect the current source, README, `AGENTS.md`, architecture notes, command ABI documentation, relevant third-party ports and recent test evidence.

Identify what is:

- Fully implemented and tested.
- Partially implemented.
- Planned but not implemented.
- Documented incorrectly or inconsistently.

Do not infer missing functionality solely from older documentation. Verify the actual code where necessary.

## B. Produce a Unix portability capability matrix

Document the existing support and important gaps in:

- POSIX-style file operations and stdio.
- File metadata and directory handling.
- Process and command execution.
- Memory management.
- Pipes and streams.
- Terminal modes and interactive display.
- Shell behaviour.
- Runtime library and mathematical functions.
- Networking.
- Application packaging and updating.

Identify which missing capabilities are required by nano, ncurses, awk, vi, Telnet and OpenSSH.

Distinguish essential shared services from optional Unix features we can reasonably leave unsupported.

## C. Review architectural feasibility

Investigate, without implementing major subsystems:

1. How the existing command loader can evolve into two execution backends: separate MultiFinder/System 7 applications and an in-process single-Finder fallback.
2. How executable applications can obtain adaptive working memory without unnecessarily enlarging Clarice's idle footprint.
3. How the terminal can support genuine ncurses efficiently.
4. How command execution and live streams could eventually operate without a Unix kernel, including execution-context and stack requirements.
5. What facilities a maintained OpenSSH client would require, and which dependencies belong only to the server.
6. How a stable versioned application ABI could allow independent software updates.

Identify architectural approaches likely to cause unacceptable startup latency, unnecessary memory use or substantial future rework.

Where an important question cannot be answered from inspection, propose the smallest practical experiment needed to answer it.

## D. Establish the staged roadmap

Create or update a concise roadmap with the following intended order:

**Phase 1 — Foundations**
1. Architecture reconciliation and capability matrix.
2. Independently updateable applications.
3. Invocation isolation and adaptive memory management.
4. Portable file and stream services.
5. MultiFinder/System 7 worker execution with single-Finder fallback.

**Phase 2 — First interactive application**
6. Reusable terminal services and genuine ncurses.
7. GNU nano, including the three-second launch target.

**Phase 3 — Userland completeness**
8. Nested command execution.
9. Bounded live pipelines.
10. Complete One True Awk compatibility.
11. Review and close agreed base shell/userland gaps.
12. Publish and validate the first external porting SDK.

**Phase 4 — Further applications**
13. vi.
14. Networking and Telnet.
15. OpenSSH client.
16. SFTP.
17. Integrated release and real-hardware validation.

These are proposed milestones, not instructions to implement them all.

Challenge the order if existing code or dependencies provide a compelling technical reason. Document the reason rather than silently changing the plan.

## E. Deliverables

For this task, produce:

1. An updated, concise engineering roadmap.
2. A current Unix portability capability matrix.
3. A short list of architectural risks and recommended feasibility experiments.
4. Documentation corrections where older claims are demonstrably stale.
5. A recommendation for the smallest practical implementation step to begin next.

Keep documentation organised. Avoid duplicating lengthy historical reports.

**Stop after Task 1. Do not begin implementing the remaining roadmap, refactor the runtime, rebuild large dependencies, or publish anything.**

Report what you found, what documentation you changed, and any decisions requiring product-owner review.