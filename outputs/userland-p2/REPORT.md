# Shell/runtime P2/P3 correctness — build 2026111960

All eleven findings confirmed and corrected; none disproved or deferred.

| Finding | Correction and focused evidence |
| --- | --- |
| Pipeline control flow | Restore parent flow/status; stages execute in script scope. Dash break/continue/return/exit cases; actual Snow pipeline continues correctly. |
| HOME | Default HOME, environment listing and tilde use /home; namespace ensures directory exists. Scoped overrides retained; host absolute/relative resolution and Snow cd/child override pass. Internal native spool root remains unchanged. |
| Substitution status | Restore enclosing status after substitution, retain separate assignment-only substitution status. Dash and Snow show prior $? and assignment failure correctly. |
| Unquoted $@ | Reuse field-boundary expansion with unquoted splitting, including custom/empty IFS. Dash and Snow agree; existing quoted-$@ cases retained. |
| Spool rewind | Check pipeline and here-document seeks; abort affected execution and clean up. All three spool forms fail injected rewinds, retain zero allocations and allow next command. |
| Remote logout bit | Mask command status to8 bits before adding explicit logout flag. Host and Snow retain connection after child exit256,511,−1. Explicit logout coverage retained. |
| Blocked read cancellation | Check cancellation when read fails before reporting I/O error. Host and actual inbound Snow Ctrl+C return130 and recover without read-error diagnostic. |
| Virtual device isatty | Recognise only terminal sentinels, not negative virtual-stream tokens. Host descriptor cases and repeated Snow /dev/null,/dev/zero probes pass. |
| Exclusive create | Remove only newly created file after failed open; never delete existing target. Preserve original error; report cleanup failure separately. Native fault tests include failed cleanup, existing file and successful retry. |
| Oversized PATH | Advance past oversized component iteratively. Host and Snow find /bin/cat after600-byte component. |
| ABI diagnostic | Derive advertised host version from actual runtime constants (1.11). Loader rejection diagnostic assertion passes. |

Host:48 focused dash cases/3 cancellation cases,54 existing POSIX cases,347
allocation-failure trials,91 loader/installer cases, remote session/service ownership
and authentication tests; resolver/startup/exclusive-create/device shims pass.
The old focused runner used macOS /bin/sh; it now explicitly uses /bin/dash.
These disagree on substitution status, so /bin/sh was not a valid reference there.

Normal 68000 Snow Plus,4MB,System6.0.8: local script, remote behaviour, cancellation
and prompt recovery pass. Ten virtual-device invocations: five sampled heaps
356118–356120bytes (2-byte alternating variation), largest354962 unchanged,
remaining command allocations0. Small probe load13–14ticks; no speedup claim.
Native failure injection remains host-only. No physical Macintosh acceptance.

Compared with1959: Core text+404,data+84,BSS+8bytes; package436352→436864(+512).
No new allocated persistent blocks or background work. HOME adds one directory
lookup to the existing startup ensure loop (10 rather than9), creating /home once
if absent. No separate cache, migration framework or recurring utility scan.

Offline HDA readback matches Core forks/type/creator/flags, Release and49utilities;
all tar payloads match the sealed kit. Publication verification covers canonical
HTTP/AFP, preserved optional vi/Telnetd, source archive and prior1959 distribution.
Unrelated files/recovery/worktrees retained; no GitHub push or hardware deployment.
