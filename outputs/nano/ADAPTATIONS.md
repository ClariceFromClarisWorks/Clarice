# Nano and ncurses adaptation ledger

Reconciled 10 November 2026. Experimental work on `codex/ncurses-port`; not part of the published v0.3.0 release.

## Dependency policy

Applications beyond the base utilities retain upstream source and their normal upstream dependency graph. Required libraries are ported rather than replaced with application-specific imitations. Optional dependencies may be disabled using supported upstream configuration options, with the resulting omissions recorded here. Versions are pinned for reproducibility; “alignment” does not mean automatic upgrades, Linux binary compatibility, or POSIX conformance.

For each update record the upstream version/source checksum, configuration, dependency decisions, adapter changes, licences and guest tests. Keep platform changes isolated so later upstream updates can be compared and retested.

## Sources and configuration

| Component | Source | SHA-256 | Current configuration |
| --- | --- | --- | --- |
| GNU nano 9.2 | https://www.nano-editor.org/dist/v9/nano-9.2.tar.xz | `05ecb99247b782e8a5b3a25ed4101dd034b0236902f7449bc9795b717642f7e9` | Evaluation: `--enable-tiny --disable-nls --disable-utf8` |
| ncurses 6.5 | https://invisible-island.net/archives/ncurses/ncurses-6.5.tar.gz | `136d91bc269a9a5785e5f9e980bc76ab57428f604ce3e5a5a90cebc767971cc6` | 68000 static, narrow characters; no shared/C++/Ada libraries, programs or disk terminfo database; embedded Clarice terminal description |

Upstream copyright, author credits and complete licences must accompany distributed source/binaries. Nano uses GPL terms; ncurses has its own permissive licence. No experimental editor binary is distributed yet.

## Macintosh adaptations in progress

- Real upstream ncurses retains its window, refresh and keyboard machinery. The native adapter supplies screen output, keyboard input and terminal dimensions.
- A bounded, byte-oriented monochrome terminal description advertises only implemented capabilities. It does not claim complete VT100 or Unicode support.
- Screen storage is allocated only while an editor owns the window, with maximum 40 rows by 132 columns and a 128-byte input queue. The current initial implementation holds window dimensions fixed during ownership.
- Library allocations use Clarice's tracked, capped port allocator. The existing 96 KiB limit remains; useful nano file sizes have not yet been measured.
- Virtual termios/ioctl interfaces describe this application terminal, not physical serial hardware. select currently supports terminal-input readiness only. Time support is intended for keyboard deadlines, not civil time.
- Raw Ctrl+C is delivered to the screen-owning application; ISIG can call its local handler. Demo cancellation/restoration has guest evidence; nano cleanup does not. The activity pinwheel is suppressed while an application owns the screen.
- Tiny, non-NLS, non-UTF-8 nano is an upstream-supported candidate configuration. Its feature omissions must be enumerated against the selected version before shipping; this is not a claim of full modern nano functionality.

## Evidence and remaining gates

Nano 9.2 built on the host in the candidate configuration (`host-version.txt`,
`build-host.log`). The first cross configuration failed for missing curses
(`configure-68000.log`); that is historical, not the current blocker.

The real ncurses library and demo subsequently linked and executed on the Plus.
See [NCURSES-TEST-REPORT.md](NCURSES-TEST-REPORT.md) for exact probe versions and
memory/load observations. `scripts/build-ncurses.sh` now records source verification,
configuration and staging; clean end-to-end reproducibility remains a gate.

Nano's CLX image subsequently linked (`work/nano/link.log`, `work/nano/nano.clx`),
using actual bundled GNU regex/getopt/strcasestr/dynarray helpers with newlib's
wide-character interface (`_REGEX_AVOID_UCHAR_H`), not a substituted editor. The
probe script depends on prepared ignored `work/nano/config` and source/header
staging; it is not yet a standalone reproducible nano build/package. Nano source,
GNU helper licences and complete adaptation provenance must accompany distribution.
No nano editing/saving or timed startup result has been observed. Submitting
`nano --version` to a guest without observing the result proves neither launch
success nor responsiveness.

Terminal adapters now include a bounded private environment and cooperative
signal handlers, raw/canonical input, limited timeouts and readiness. Later
sigaction/masking and fdopen/file changes exceed the earlier demo test coverage.
Some termios fields are accepted without full semantics. Nano-specific stat/access
and copied-value putenv are not complete POSIX services. Full directory APIs are
not implemented merely because dirent.h exists. See [capability matrix](../PORTABILITY.md).

Host screen/platform test output exists; the earlier runtime regression log reports
66 grep comparisons plus bounds/cancellation/output-failure checks. None substitutes
for nano guest file-integrity or low-memory tests. No tests were rerun in Task 1.

The revised roadmap defers more editor work until update safety, invocation/memory
and portable I/O foundations are addressed. Preserve this experimental branch.
Three-second empty/8 KiB nano launch acceptance remains unverified, with existing
ncurses load times already indicating a performance risk.
