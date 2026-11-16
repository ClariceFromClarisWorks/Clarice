# Clarice engineering requirements

Authoritative direction: [revised brief](references/engineering-direction-2026-11-10.md).
Read [current state](outputs/CURRENT-STATE.md), [architecture](outputs/ARCHITECTURE.md)
and [roadmap](outputs/ROADMAP.md) before implementation. Earlier instructions and
historical phase ordering are superseded by this brief. The clarified performance
policy below takes precedence where the older brief differs; it grants no new
implementation scope.

- Performance baseline: stock Macintosh SE, 8 MHz 68000, 4 MB, System 6.0.8. Preserve Macintosh Plus, later 68k and single-Finder compatibility.
- Clarice is a native Macintosh CLI, not a replacement OS. Target separate external application processes under MultiFinder/System 7 and in-application execution under single Finder, sharing implementations where practical.
- Use genuine upstream applications and required libraries; preserve authorship, licences and separable adaptations. Build reusable file, stream, invocation, terminal and networking services. Never fake success or invent user/group/kernel semantics to claim compliance.
- External applications must eventually be independently buildable, packaged and replaceable through a versioned runtime contract. Startup must not overwrite newer independent installations.
- Preserve newer local work. Inspect code/tests before assuming a feature is absent. Maintain concise current-state documentation and explicit compatibility gaps.
- Keep tasks focused. Do not rebuild for documentation-only changes. Use focused regression tests for implementation changes and distinguish compilation, host tests, emulator execution and hardware validation.
- Do not push commits, publish releases or begin subsequent roadmap tasks without explicit authorisation.
- Authorisation follows explicit user-approved tasks, not the historical **Task 2 / roadmap 1.2 only** stopping point. Later separately authorised work includes terminal/QOL fixes, Serial-1 through Serial-3, runtime and nano performance work, safe HFS saves, and bounded loader review and external hardware tooling. Consult dated entries in [current state](outputs/CURRENT-STATE.md) and the relevant task reports alongside the user's instructions; completed work is evidence, not continuing permission.
- Carry forward existing authorisation within its agreed scope without asking again, preserving unpublished editor and other local work. Honour each task's exclusions, pauses and stopping conditions; review-only work does not authorise implementation, and completed or paused tasks are not automatically reopened. New roadmap tasks or material scope extensions (including worker architecture, live pipes or a general SDK) require explicit authorisation; overlapping authorised fixes do not authorise the whole milestone. This policy update authorises no implementation, push or release.

At implementation-task completion report changes, tests, relevant measured costs,
limitations, decisions for review and the recommended next step.

## Performance-first 68k engineering and proportionality

Apply forty years of classic Macintosh, 68000 systems and demoscene lessons to
necessary work, not merely to making an expensive design run faster:

- Optimise in order: avoid unnecessary work and defer or batch required work where appropriate; improve algorithms, data layout and data movement; use documented native System 6/7 Toolbox and OS mechanisms; only then use measured 68000 assembly where it benefits an identified bottleneck. Retain practical C interfaces and upstream code.
- Before adding custom loaders, abstractions, dependencies, security mechanisms or validation passes, challenge their necessity and repeated costs. Give a short, concrete assessment of the requirement, native/reusable alternative, startup/CPU/RAM/I/O cost, benefit and smallest useful measurement. Avoid lengthy speculative research; do not invent large frameworks or requirements.
- Prefer documented native Mac APIs and established Unix-like semantics over rebuilding available functionality. Demoscene insight is an engineering discipline, not permission for OS patches, unsupported system-state manipulation or invasive hardware tricks in Clarice proper. Keep external development, profiling and hardware-control tooling separate from the shipped runtime.
- Protect a small idle memory and CPU footprint. Allow foreground programs adaptive working RAM according to need and safely available application memory, including contiguous-space constraints; retain headroom for UI, errors, cancellation and safe file operations. No arbitrary fixed allocation ceilings or resident caches to hide startup costs. Release invocation-owned resources.
- Performance, startup, interaction latency, memory and full-operation measurements are acceptance criteria for significant changes. Instrument before optimising; compare representative before/after cases, including first/repeat runs, peak memory and cleanup. Measure complete operations, not just a faster inner loop or loader phase. Use Snow for functional and reproducible regression evidence; physical baseline hardware is the final performance authority. Label emulator and hardware results separately and do not claim unmeasured acceptance.
- Nano must reach a fully drawn, keyboard-responsive editor within three seconds of command submission for empty and ordinary 8 KiB files, including loading, runtime/library initialisation and first rendering. Report first and subsequent launches separately. This goal remains unmet; identify measured bottlenecks and the next focused improvement rather than weakening the criterion.
- Preserve safe HFS saves/replacement, recovery, meaningful errors and exit status, and necessary security/authentication. Never remove needed protection or falsely stub success for speed. Match safeguards to the actual operation and threat model: private-LAN development and public distribution have different exposure, but LAN location alone does not establish authenticity. Avoid redundant passes; material changes to integrity or authentication policy require review before implementation.
- Keep ports faithful and reusable across single Finder and MultiFinder/System 7, sharing upstream implementations and practical services while respecting native execution differences. Preserve independent packaging and the documented third-party SDK/porting goal; build shared interfaces from demonstrated port needs, within authorised scope.
- Fix routine defects autonomously within the authorised task. Stop and escalate material architecture, compatibility, security, data-safety or performance/memory tradeoffs with concise evidence and a recommendation. Minimise unnecessary tool calls, rebuilds, test runs and credit use: reuse evidence, test focused affected cases, and broaden only for demonstrated risk or required acceptance.

## Native Macintosh First — An Environment, Not an Operating System

Clarice is a Unix-like userland layered over System 6/7, not a replacement
operating system. Extend the Macintosh with a command-line environment, faithful
utilities and practical compatibility services; build upon the host system
rather than duplicating its responsibilities.

- Prefer documented native Macintosh Toolbox, Operating System and Memory Manager facilities when they suitably meet the requirement. Where they do not, evaluate proven existing implementations before introducing Clarice-specific infrastructure, following Research Before Reinvention below.
- Native-first does not mean native at any cost. Measured algorithms, data-layout improvements or focused 68000 assembly may outperform a native API. Choose on demonstrated complete-operation benefit, compatibility and resource costs under the performance policy above, not on native status alone.
- Introduce custom mechanisms only for a demonstrated capability gap or a compelling measured advantage. Keep them minimal, interoperable and reusable; CLX's single-Finder execution role does not justify an independent operating-system layer or predetermine the eventual external-application design.
- Do not recreate modern kernel services, process management, memory isolation or security models merely because Unix systems have them. Provide only demonstrated, authorised capabilities and describe the host's actual limits truthfully. Preserve necessary structural and memory-safety checks, safe HFS operations, recovery and authentication; this principle does not authorise an integrity-policy change.
- Assess both new and existing infrastructure by practical benefit, complexity, startup and ongoing CPU/RAM/I/O costs, and compatibility with single Finder, MultiFinder and System 7. Challenge unnecessary machinery before optimising it; use the smallest useful measurements and retain the independent packaging and porting goals.

This principle guides decisions within authorised work; it grants no new
implementation scope and does not reopen completed or paused tasks.

## Research Before Reinvention

Before substantial features, ports or performance work, research proven historical
Macintosh/Unix implementations, modern systems algorithms, and demoscene and 68k
techniques for relevant starting points. Study available source for ideas, then
implement Clarice-specific solutions independently rather than copying verbatim;
this does not replace the requirement to retain genuine upstream code in ports.
Respect licences and attribute useful references. Keep research proportional to
the task, evaluate approaches against the stock 8 MHz 68000 / System 6 / 4 MB
baseline, and measure on physical hardware under the performance policy above.

## Unix familiarity and compatibility

Clarice is a native System 6/7 Macintosh Unix-like environment that should feel
familiar to Linux, BSD and macOS shell users. Apply the requirements above with
these interface and compatibility principles:

- Favor established Unix command names/options, environment variables, standard streams, redirection, tty behavior and semantics. Use discoverable `/dev` device names (for example, `/dev/tty.modem` for the modem serial port) so familiar Unix workflows transfer naturally.
- Expose HFS, resource forks, type/creator codes and Finder metadata as documented, discoverable Macintosh extensions rather than surprising replacements for familiar behavior.
- Report capabilities, unsupported operations and failures truthfully, with meaningful errors and exit status; apply the no-fake-success rule to every interface. Do not patch the OS or emulate a fictitious Unix permission model.
- Extend the reusable services above into consistent file, device, terminal and process interfaces across application ports, respecting the actual execution capabilities of each Macintosh environment.
- Validate real-world Unix-style tasks, including file operations, stream/redirection workflows and serial connections to other servers as support becomes available. Document and justify differences from Unix behavior; do not imply that planned capabilities already work.

- New production commands require accurate `--help` (or `help command` for
  argument-sensitive builtins), option/status/stream regression coverage and
  explicit Macintosh limitations before they are considered complete.
