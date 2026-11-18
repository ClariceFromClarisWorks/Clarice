# Telnet observations — BusyBox utility comparison

Isolated Snow only, Core 2026111823 / telnetd 2026111814. This fixture predates
concurrent Telnet rework, so these results do not validate that newer version.

- Authenticated command execution, redirected binary output and status retrieval
  worked during the 190-observation functional regression run.
- ETX cancelled a CPU-only BusyBox sed branch loop, returned status 130 and
  restored the prompt. A subsequent command worked; invocation allocations
  returned to zero.
- No new Telnet defect identified in those checks. wc Unicode counts, tr NUL
  handling differences and sed final-newline behavior are utility differences,
  not attributed to Telnet. Data comparisons use HFS files, not screen text.
- Updater deployment was performed locally because this fixture intentionally
  restricts remote updates. Test commands use Telnet; no shared screen takeover
  or physical SE testing was used.

All 282 timed invocations and 43 robustness observations completed. BusyBox sort
out-of-memory reporting returned a usable prompt; subsequent commands passed.
No new Telnet defect was observed. Private listener disabled and guest closed.
