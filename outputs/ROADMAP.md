# Clarice engineering roadmap

10 November 2026. Proposed milestones, not blanket implementation authorisation.
Task 1 was reviewed and accepted. Task 2 implements step 1.2; [completed Plus guest acceptance and validation boundaries](task2/TEST-REPORT.md) are recorded. Stop after Task 2; later steps require authorisation.

| Phase / step | Milestone | Exit evidence |
| --- | --- | --- |
| 1.1 | Architecture reconciliation (this task) | Current state, capability matrix, risks and decisions traceable to code/evidence. |
| 1.2 | Independently updateable applications | Separate package/build workflow, declared runtime requirements, startup cannot downgrade local tools; safe explicit replacement/rollback tests. |
| 1.3 | Invocation isolation and adaptive memory | Owned state/cleanup, negotiated budget and stack reserve, nested-state design; low-memory and repeated-run tests. No universal 96 KiB policy. |
| 1.4 | Portable file and stream services | Descriptor/stdio ownership, seek, metadata, directories, errors and safe writes tested on HFS; unsupported operations fail explicitly. |
| 1.5 | MultiFinder/System 7 workers; single-Finder fallback | Shared application implementation, measured launch/completion/cancel transport, isolated partitions and UI responsiveness; both OS paths tested. |
| 2.6 | Reusable terminal services and genuine ncurses | Preserve existing probe; publish supported modes/capabilities, ownership/restore/input correctness and measured dirty-row rendering. |
| 2.7 | GNU nano | Faithful configured source, editing/search/save/error tests and releaseable dependencies; ≤3 s submission-to-responsive display for empty/8 KiB input, first/repeat separately. |
| 3.8 | Nested command execution | Child arguments/environment/cwd/streams/status isolated; parent restored on every outcome; recursion/stack/resource bounds tested. |
| 3.9 | Bounded live pipelines | Producer/consumer interleaving, backpressure, EOF/close, broken pipe and whole-pipeline cancellation; both backends, no unbounded queue. |
| 3.10 | Complete agreed One True Awk compatibility | Real `system()` and command pipes/pclose, statuses and nested cleanup; upstream comparison suite and explicit residual gaps. |
| 3.11 | Close agreed shell/userland gaps | Explicit compatibility inventory, negative tests and performance evidence; no blanket POSIX claim. |
| 3.12 | First external porting SDK | Independent clean build/install/update of an example without modifying/rebuilding Clarice; ABI tests and licences. |
| 4.13 | Faithful lightweight vi | Select upstream/dependencies, preserve behaviour and notices, edit/recovery/performance tests. |
| 4.14 | Native networking and Telnet | Installed-stack TCP/DNS adapter, readiness/timeouts/cancel; Telnet negotiation and terminal integration. Serial is a distinct optional transport. |
| 4.15 | Maintained OpenSSH client | Chosen maintained version, secure randomness/crypto and host-key handling, connected client tests, measured memory/latency; explicit disabled features. |
| 4.16 | SFTP | SSH transport plus local metadata/seek, transfer/recovery tests and explicit unsupported attribute semantics. |
| 4.17 | Integrated release and real hardware | Full regression and packaging, Plus/SE and later-machine coverage; physical results separate from emulator data. |

Order retained. Existing ncurses/nano work is a reusable head start, not grounds
for bypassing update safety, isolation and file correctness. Step 1.2's manifest
must leave room for two backends, but a full scheduler or worker implementation
is not needed to fix installer overwrite. Step 1.5 needs a minimal stream/control
transport; arbitrary live pipelines can still wait for 3.9. Shared invocation
state must be designed in 1.3 before 3.8, to avoid a second incompatible ABI.

Three-second nano target: record command submission, loader completion, runtime
and curses initialisation, first committed frame and first serviced input.
Use an empty buffer and a documented ordinary 8 KiB fixture, cold/first and
repeat runs with identical emulator/storage/font settings. Report all samples
and memory peaks; elapsed interaction time is not startup time. If missed,
attribute costs before choosing assembly, format changes or a bounded cache.
Do not keep nano/ncurses resident merely to hide first-launch cost.

Separate future possibilities: SSH server, background services, native compiler
and BlueSCSI-assisted cryptography. None is required for independent updates.
