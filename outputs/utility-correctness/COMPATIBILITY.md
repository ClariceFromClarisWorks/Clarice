# Portable shell and utility acceptance — 16 November 2026

Build 2026111540 development candidate. **Snow only**, Plus/4 MB; guest reports
System 6.0.7. No new physical/System 6.0.8 acceptance. No POSIX certification.

**Fully supported** below means the explicitly named form, within stated capacity
bounds, matches the focused portable fixtures. **Partial** means useful support
with gaps. **Unsupported** means absent or rejected. **Platform-limited** means
System 6/HFS semantics deliberately differ; it does not excuse silent success.
The reference is dash plus portable utility behaviour, not Bash extensions.

## Shell matrix

| Area | Status | Behaviour and limits |
|---|---|---|
| Simple commands, sequences, `&&`, `||`, `!`, `$?` | Fully supported (named forms) | Left-associative and/or; short-circuiting and final pipeline status tested. Cancellation is 130. |
| Single/double quotes, escapes, concatenation | Partial | Tested strings with spaces, empty arguments, literals and adjacent expansions. Embedded quoted `$@` is explicitly unsupported; separate `"$@"` works. |
| Variables, IFS, positional parameters | Partial | Assignment, unset, `$0`/`$1`/`$#`, `set --`, shift, quoted `$*`, IFS whitespace/non-whitespace splitting tested. 16 variables, names 31 B, values 127 B; export/locality is not a complete POSIX environment model. |
| Parameter expansion | Partial | Defaults/alternates/assignment/error/length and prefix/suffix removal exist. Special/braced parameter grammar is not comprehensive. No Bash arrays or indirect expansion. |
| `for`, `while`, `until`, `if`, `case` | Partial | Ordinary forms, nested loops and unnumbered break/continue tested. Numeric break/continue levels rejected. Existing syntax/execution nesting limits remain. |
| Functions, `.`, `sh -c`, script operands | Partial | Function arguments, return, saved positional parameters, source effects and copied redirected bodies tested. Redirection attached to a function *definition* is rejected; redirecting its call or body works. |
| Compound-command redirects | Fully supported (fds 0–2/forms below) | Groups, subshells, loops, if/case; scope lasts through completion/break/continue/return. Uses the same ownership/cleanup as simple commands. Nested and pipeline cases tested. |
| `<`, `>`, `>>`, `2>`, `n>&m`, `n<&m`, closure, here-documents | Partial | Descriptor 0–2 mapping and left-to-right ordering; quoted/unquoted here-documents tested. `<>`, general numbered descriptors and complete here-doc edge cases are not supported. Output open/truncate follows shell semantics, not editor safe-save semantics. |
| Finite pipelines and command substitution | Partial / platform-limited | Native HFS spools; finite combinations tested with genuine grep/sed/awk/pipeline ports. Sequential, not concurrent pipes: no infinite producers, SIGPIPE or live IPC. Substitution limited to 2047 B. |
| Subshell grouping | Partial | Environment/functions/cwd snapshot, no separate OS process. Positional parameters/options are not a complete process-isolation model. |
| Arithmetic and `test`/`[` | Partial | Integer arithmetic and common string/numeric/file tests; not all operators/test expressions. Overflow and advanced arithmetic syntax not comprehensively audited. |
| `read`, `printf`, echo | Partial | One-variable `read [-r]`, IFS trimming, escaped newline and EOF; >127 B fails after consuming record, preserving old variable. printf supports `%s`, `%d`, `%%` and a limited escape set, not the full POSIX formatter. |
| Pathname expansion | Partial / HFS | Final-component globs sorted; parent-directory wildcards rejected. HFS case/31-byte names and encoded `/`/`%` differ. Host glob shim is not evidence of native glob behaviour. |
| Shell options, traps, exec, readonly, getopts | Unsupported/incomplete | No full `set -e/-u/-f`/trap/exec facility; `set --` is supported. Do not silently assume scripts using these features are portable here. |
| Jobs, Unix process IDs, fork, signals, users/permissions | Platform-limited | Not simulated. Cooperative Ctrl+C, no process groups/job control/sudo/Unix permission ownership. chmod is not implemented; native metadata uses hfsattr. |
| Resource bounds | Partial | 8 KiB script, 256-token guard, 32 arguments/4096 B argument storage, 16 functions; command line/paste 1023 B. Bounds are explicit restrictions, not POSIX limits. Parser-wide OOM hardening remains incomplete; targeted new allocation failures are tested. |

## Utility acceptance inventory

Every production bundled name is covered below; benchls/benchscan are diagnostics,
not userland compliance claims. No newly installed or rebuilt package is inferred
from a source edit. `guest-tool-hashes.json` identifies the exact 34 external
packages exercised with `--help` in the current disposable guest. This does not
claim exhaustive option/failure testing or revalidate communications/updating.

| Commands | Status / present acceptance | Remaining differences or limits |
|---|---|---|
| ls, df, cat | Partial; current guest listing/dots/metadata, invalid option, backing-volume df, cat through scripts | ls streams HFS catalog order, not Unix alphabetic default; HFS flags/forks/creation time. Multi-volume regression evidence is retained from QOL-1, not freshly repeated here. |
| grep, sed, awk | Partial; current real report pipeline, missing inputs, prior large-record/failure/cleanup evidence | sbase grep/sed and One True Awk. C/byte-oriented locale; sed not GNU -i; awk system()/command pipes unsupported, expression/parser depth bounds. Installed hashes recorded; not all packages have identical runtime build dates. |
| wc, head, tail, cut, tr, sort, uniq, tee | Partial; rebuilt current runtime packages, real multi-stage guest pipeline; host comparisons/spill/cancel/failure checks retained | sort spills, tail -f cancellable; native memory/file limits remain. Unicode processing is not a Unicode terminal. GNU extensions and tail -m unsupported. |
| cp, mv, touch, mkdir, rmdir, rm, ln, readlink, hfsattr | Partial / HFS; current guest create/copy/move/link/delete/locked refusal and metadata | cp/rm demand-driven DFS; both-fork fault/recovery evidence retained. No hardlinks/Unix chmod. Directory trees are not transactional; partial work on error is reported. Native path limits remain. |
| find, du, file | Partial / HFS; current tree/find and native allocated-space/type/creator output | DFS elastic, no arbitrary depth 8. find predicate subset, no -exec/-o; file is native metadata, not libmagic; du accounts both forks. |
| od, more | Partial; exact dumped byte values and redirected pager tested | od layout/word endianness native 68000; limited formats. more has bounded history, not full backward/search navigation; prior interactive Q/Ctrl+C evidence retained. |
| basename, dirname | Fully supported (declared single-path forms) | Tested in portable file workflow; shell argument/path bounds remain. |
| serial, kermit | Partial / transport-specific; help fresh, bidirectional/cancel/recovery evidence retained from Serial milestones | No new serial link session this task. Data-fork Kermit transfers; complete Mac files need a container. No general VT100 or Unix tty subsystem. |
| wget, clarice update | Partial; help fresh, prior private-LAN download/update acceptance retained | Numeric local HTTP, not general TLS/Internet authentication. Recovery safeguards retained; updater delay/error 28 remain known. Not exercised against real SE here. |
| nano/ncurses | Partial; prior editor/save/cleanup/pixel evidence retained | Experimental separately installed port, not a newly rebuilt bundled command. Startup target unmet; no new editor/hardware claims. |
| appinstall, prefs/status/help/history/clear, cd/pwd | Native builtins, partial Unix equivalence | Existing installer/recovery and UI evidence retained. Builtins are not external packages. No new integrity policy or preference change. |

## Evidence and remaining work

- `scenarios-host.json`: twelve complete portable scripts, production parser/adapter
  with real host upstream tools; host subprocess bridge exists only in tests.
- `scenarios-guest.json` and `workflows/`: equivalent native execution, exact output
  except od comparison normalises only presentation whitespace (same byte values).
- `compound-failures.json`: scope/control-flow/cancellation and descriptor cleanup.
  `compound-oom.json`: 18 fault-position trials: 10 injected failures at new parser/copy ownership sites
  plus 8 beyond-allocation controls.
- Earlier utility depth/stream/large-record tests remain in RESULTS.md and AUDIT.md.
  These establish particular capabilities, not every possible Unix option.

Next correctness priorities: shell capacity/parameter grammar, complete printf/read
semantics, export/subshell isolation and additional error cases. They are an
inventory of gaps, not authorisation for a shell replacement or OS facilities.
