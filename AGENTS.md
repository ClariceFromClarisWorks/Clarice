# Clarice project requirements

Read outputs/ARCHITECTURE.md before implementation. Preserve the original briefs in references/.
Latest user requirements override the original brief:
- Baseline Macintosh Plus, 4 MB physical RAM, System 6.0.8. SE is additional coverage; PowerBook 180 uses System 7.1 or later.
- Normal movable terminal window under MultiFinder, with Finder and desktop accessible; no full-screen takeover. Retain single-Finder fallback.
- Use a suitable installed system monospaced font by default; allow choosing installed fonts and persist the preference. This supersedes the bundled custom-font requirement.
- Bash/zsh-inspired interactive editing, history, command/path Tab completion, quoting and variables are Phase 1 requirements. Do not claim bash/zsh compatibility without tests.
- POSIX sh is the first scripting compatibility target. Consider bash/zsh extensions or ports later, without changing portable sh behaviour. Compatibility is a target, not an achieved claim; verify language features and document runtime limitations.
- Assembly-first low-level runtime with a C-compatible ABI. Compare significant assembly routines against executable references; retain measured winners. Baseline instructions must run on 68000.
- Reuse SkiFree SE's existing pinned Retro68 Docker image. scripts/build-probe.sh records the verified invocation. Do not alter SkiFree, MacBench68 baselines, ROMs or original system disks.
- Architecture and feasibility precede runtime implementation. No speculative full framework, fork emulation, mandatory alternate filesystem or second TCP/IP stack.
- User-facing reports and evidence in outputs/, scratch and upstream research in work/.
- Never label build success as guest execution or emulator measurements as real hardware results.
- History is disk-backed and capped, with fixed small RAM caches. Scrollback is separate and bounded.
- Evaluate OS cache settings and, only on compatible later machines, RAM Doubler. Never assume they improve speed or enable/change system settings automatically.
- Auto-enable proven enhancements by CPU, OS/API and available-memory checks; retain baseline overrides and the complete Plus/SE core.
- HFS access is paramount. Internal app commands/assets may use an indexed bundle or resources if measured faster; compare with loose HFS files. User files must remain directly accessible on native HFS.
- Stretch goal: native GCC compile/assemble/link workflow inside Sys6Unix, including compatible headers/runtime/startup code. Assess historical versions for Plus/4 MB; scale optional toolchains to qualified later hardware. Keep compiler work separate from core delivery; cross-compilation alone does not satisfy this goal.
- Product framing: native System 6 command prompt with familiar Unix behaviour and reusable compatibility for faithful upstream utility ports, not a replacement/full Unix stack.
- GNU tools including grep/awk/sed and a faithful vi or nano port remain planned. Authors, copyright notices, licences and required source must accompany ports; changes/limitations explicit. Upstream nano tiny is acceptable if needed.
- Terminal snappiness is the absolute priority. ls -la must avoid redundant metadata lookups, stream output and yield/cancel; measure large directories.

- Interactive ls pages by screen and pauses enumeration; never page redirected/script output. Metadata uses the existing catalog record, including both fork lengths and Finder flags.
- Preferences persist installed font/size, monochrome colours, paging and window dimensions in a small native HFS file. Monaco 9 is the baseline; optional fill-screen retains menu access and restores the window.
- Current mrsh-based scripting is a bounded subset. Keep explicit compatibility gaps in README and test reports; passing selected fixtures does not establish POSIX conformance.

- Prioritise scrolling, bounded scrollbar history and a native Preferences dialog. Cache settings mean terminal rendering cache and scrollback only.
- Defer richer process enumeration/CPU accounting to System 7 supported Process Manager APIs; do not invent Linux process metrics on System 6.

- Product name is Clarice; all new user-facing UI and packages use this name. Root data belongs in Clarice Data inside the running System Folder. Preserve original briefs/historical evidence.
- Expose mounted volumes through /mnt. Use bounded shell symlinks, PATH defaults plus /etc/profile, and a cached basename-only directory > prompt with optional HOSTNAME. Eject/format remain deferred.

- Clarice uses original monochrome command-window icon artwork; do not copy the classic Macintosh-with-prompt silhouette.

- Up/Down navigate command history; retain the bounded disk-backed store. Remove the permanent shortcut-hint/status bar; paging may show a temporary More prompt.

- Move ordinary external utilities (ls, df, cat, grep and others) into independently loadable programs under /bin or /usr/bin, resolved by PATH. Retain shell-state builtins such as cd. Do not use placeholder wrappers to claim executable separation.
- Use a faithful, updateable upstream source port for grep, with licences and author credits. Consider awk and sed next. Preserve performance and bounded memory on the Plus.
- External-program work must cover standard streams, redirection, pipes and exit statuses with explicit tests and documented gaps; do not equate sequential temporary-file pipelines with concurrent Unix pipes.
