# Architectural risks and feasibility experiments

10 November 2026. Inspection and research only; proposed experiments below were
not executed in Task 1. Conclusions are recommendations, not verified new APIs.

## Two execution backends and memory

The loader can be reused, but its import table cannot simply become cross-process
calls: it exposes host pointers, native structs, global descriptors and newlib
state. Recommend a stable portable service contract with separate in-process and
worker implementations. Keep one application source; measure CLX-in-worker versus
direct-linked APPL packaging before standardising the binary envelope.

Apple's System 6 launch example distinguishes sublaunch under MultiFinder from
launch that replaces the calling application. System 7 has separate launch/control
facilities and capability indicators; do not assume their availability on System 6.
This supports the two-backend proposal but does not establish an IPC method.
[Apple TN126 example, archived in THINK Reference](https://ioi-xd.net/os9docs/pdf/launching_another_application_under_system_6.pdf),
[Inside Macintosh Process Manager summary](https://dev.os9.ca/techpubs/mac/Processes/Processes-52.html).

Minimum/preferred `SIZE` values govern partition admission; current Clarice sets
both to 768 KiB. A worker can request a different partition, but this is not
unlimited heap growth. Some System 6 MultiFinder versions do not reliably supply
application-died events, so completion needs an explicit protocol plus a tested
lost-worker path. [Inside Macintosh SIZE resource](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-87.html).

Adaptive allocation should combine command requirements with measured free and
contiguous app memory and explicit recovery reserves. First eliminate the uniform
96 KiB policy inside a safely admitted invocation; do not simply lift the cap.
Temporary memory outside the partition is a later OS-qualified experiment, not a
universal allocator: lifetime, locking and event-yield rules must be verified.
[Inside Macintosh Memory Manager](https://dev.os9.ca/techpubs/mac/pdf/Memory/Memory_Manager.pdf).

## Live execution without a Unix kernel

Use byte-stream endpoints with bounded buffers, close/reference ownership, EOF,
partial I/O, readiness, error and cancellation states. For a worker backend,
evaluate batched messages/shared-memory rings or a minimal file mailbox prototype;
no cross-heap pointer ownership is assumed. A file mailbox can prove launch/status,
but is not evidence for low-latency terminal transport or live pipes.

Single-Finder live pipelines need stackful cooperative command contexts (or an
upstream-supported resumable execution model). Unmodified C call stacks cannot be
paused/resumed by ordinary recursive calls. Audit 68000 saved registers, A5/world,
stack alignment, newlib errno/FILE/locale globals and nested longjmp cleanup before
any context-switch assembly. Shared buffers alone do not solve this. System 6
has no assumed thread facility; keep memory and runnable-context count bounded.

Nested `system()` may run a child synchronously after state isolation; command
pipes additionally need producer/consumer progress. A global cancellation bit must
become a defined invocation/pipeline policy. Do not let a child's exit unwind or
free its parent. Keep shell process semantics explicitly limited.

## ncurses and nano performance

Reuse the actual ncurses 6.5 demo and native row compositor. Advertise only
implemented capabilities; resolve raw/canonical timing, flush/drain, output
processing and resize semantics before generalising the ABI. Keep screen state
command-owned and outside ordinary scrollback; restore cursor, terminal modes and
shell on errors/cancel. No full curses implementation rewrite is warranted.

Current loader scans the complete image for checksum, imports by repeated string
comparison and validates/relocates records. Both image and metadata exist at load
peak. The demo's 338/277 tick load measurements exceed the entire future nano
budget; they do not prove which stage dominates. Possible measured remedies:
index runtime imports, reduce relocations, use a compact format or direct-linked
worker, batch I/O. Avoid permanent code caches, deleting validation or trading
cold-launch performance for undocumented persistent RAM.

## Maintained OpenSSH client: feasible scope still unproven

Upstream requires compiler/libc/headers. Its current build documentation makes
libcrypto and zlib optional (without crypto libraries the algorithm set is
restricted); FIDO adds further dependencies. Select a maintained version and
interop/security profile before choosing library configurations. Server privilege
separation accounts/directories are sshd requirements, not a reason to invent a
local user database for the client. PAM/account login, server listeners, chroot,
server privilege separation and server PTY allocation are outside this milestone.
[Upstream INSTALL](https://raw.githubusercontent.com/openssh/openssh-portable/master/INSTALL),
[upstream build overview](https://github.com/openssh/openssh-portable).

Engineering gates for the client: native TCP/DNS and nonblocking readiness;
cryptographically secure entropy (never TickCount or rand); authenticated key
exchange/ciphers, key parsing, host-key verification/persistence; config and
private-key file I/O; timeouts, cancellation and secure cleanup. HFS has no Unix
owner/mode enforcement: explicitly document local key-protection limits and adapt
checks instead of manufacturing uid/mode success. Remote login name is profile/
connection data, not a local root account. Do not imply that disabling locale
removes these dependencies. Plus/4 MB viability and acceptable crypto latency are
unproven; preserve the full baseline core while qualifying any later-CPU extras.

The SSH client itself has local identity, configuration and runtime assumptions.
Proxy commands and related helper paths use child execution/IPC; they can be
explicitly excluded initially rather than requiring fork cloning for a basic
connection. [Upstream ssh.c](https://github.com/openssh/openssh-portable/blob/master/ssh.c),
[sshconnect.c](https://raw.githubusercontent.com/openssh/openssh-portable/master/sshconnect.c).

SFTP is not just file-copy UI: upstream normally starts SSH through pipes or a
socketpair and waits for it; local shell escapes also spawn commands. Adapt that
boundary to Clarice child execution, or separately review an in-process transport
binding with preserved protocol behaviour. Do not silently promise unmodified
fork semantics. [Upstream sftp.c](https://raw.githubusercontent.com/openssh/openssh-portable/master/sftp.c).

## Smallest practical experiments, in priority order

| Risk | Small experiment when authorised | Decision evidence |
| --- | --- | --- |
| Startup overwrites tools | One independently replaced CLX tool; simulate restart, old/new bundle, foreign metadata, corrupt file and failed replacement. | Local version survives; explicit recovery restores a valid complete image; no silent downgrade. |
| Unstable ABI | Independently compile one command against a minimal frozen service manifest; run it on old/new hosts and reject missing capabilities before entry. | Compatibility matrix, status/error clarity; no host rebuild for a compatible command update. |
| Fixed memory / shared invocation state | One allocation-pressure probe and parent→child→parent test with differing cwd/env/streams; instrument image, stack, shared and owned allocations separately. | Recovery reserve and no leaks/state escape after failure/cancel; budget based on available memory, not 4 MB assumption. |
| Worker launch / transport | Tiny worker echoes an invocation ID and exit status; then exchange bounded 1/4 KiB batches while Finder/terminal stay responsive. Test System 6 MF, System 7, launch failure and single-Finder fallback. | Launch cost, bytes/sec, cancellation latency, partition footprint and failure detection; choose envelope/transport. |
| Portable I/O corruption | fdopen ownership/update/seek, exclusive-create collision, short write, locked/read-only/full disk and interrupted save on disposable HFS files with both forks. | Exact errors, no double-close or truncation of prior good data; complete required stdio semantics. |
| Nano startup | Instrument empty and 8 KiB document first/repeat runs, including first rendered frame and input acknowledgement; no interaction time in startup figure. | ≤180 ticks at 60 Hz, or cost breakdown and focused proposed fix; label emulator separately. |
| Live-stream deadlock / stack cost | Two independently suspended commands through a tiny bounded ring; slow consumer, early close, both directions and cancel. | Correct EOF/backpressure/status, bounded memory, stack high-water and restored parent context; test both execution backends. |
| SSH viability | Only after networking: maintained upstream crypto known-answer tests plus a local TCP/SSH handshake probe and entropy assessment. | Correct interop, justified randomness source, memory/latency budget; client feature/dependency decision before full port. |

## Product-owner review

Recommended: default startup never overwrites an existing program, including an
unrecognised/corrupt one; explicit repair/replacement is separate. Agree the first
ABI compatibility boundary before promising third-party binaries. Decide worker
transport/partition policy from measurements, not now. For OpenSSH, agree the
initial optional feature and key-storage policy after feasibility evidence.
The three-second nano target remains a hard acceptance goal; no exception is
proposed or silently granted. None of these decisions authorises subsequent work.
