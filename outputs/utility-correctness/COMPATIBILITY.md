# Portable shell and utility acceptance — 16 November 2026

Build 2026111546, focused utility close-out (1545 checkpoint ee2faf8 preserved). **Snow only**, Plus/4 MB; guest reports
System 6.0.7. No new physical/System 6.0.8 acceptance. No POSIX certification.

**Fully supported** below means the explicitly named form, within stated capacity
bounds, matches the focused portable fixtures. **Partial** means useful support
with gaps. **Unsupported** means absent or rejected. **Platform-limited** means
System 6/HFS semantics deliberately differ; it does not excuse silent success.
The reference is dash plus portable utility behaviour, not Bash extensions.

## Shell matrix

| Area | Status | Behaviour and limits |
|---|---|---|
| Simple commands, sequences, `&&`, `||`, `!`, `$?` | Fully supported (named forms) | Left-associative and/or; short-circuiting and final pipeline status tested. Cancellation is 130, including interrupted startup profiles and loop conditions at script boundaries. |
| Single/double quotes, escapes, concatenation | Partial | Tested strings with spaces, empty arguments, literals and adjacent expansions. Quoted `$@` preserves argument boundaries, empty arguments and attached prefixes/suffixes. Empty quote groups follow dash, including `"$@$@"`; separate `"$@"` with no parameters disappears. |
| Variables, IFS, positional parameters | Partial | Assignment, unset, `$0`/`$1`/`$#`, `set args`, `set - args`, `set --`, shift, quoted `$*`, IFS whitespace/non-whitespace splitting tested. Variable names/count/values and positional storage now grow on demand; 80 variables, 100 arguments and 64 KiB records tested. Export attributes survive reassignment and snapshots; new sh invocations inherit exported values and no functions. `export -p` round-trips quoted values and exported-but-unset names. Native HOME/PWD defaults remain Macintosh-specific. Rebuilt awk imports grow on demand; the editor adapter also imports on demand, preserves only exported values and forces TERM=clarice. Host 80 x 4096-byte and Snow 40 x 2048-byte imports pass with cleanup; genuine exhaustion fails explicitly. |
| Parameter expansion | Partial | Defaults/alternates/assignment/error/length and prefix/suffix removal exist. Braced special parameters `${@}`, `${*}`, `${#}` and `${?}` work. Default/alternate words preserve nested quoted `$@` fields. Operations on `$@`/`$*` other than ordinary expansion explicitly fail; scalar assignment/error operators do not provide full vector expansion. No Bash arrays or indirect expansion. |
| `for`, `while`, `until`, `if`, `case` | Partial | Ordinary forms, nested loops and unnumbered break/continue tested. Numeric break/continue levels rejected. Existing syntax/execution nesting limits remain. |
| Functions, `.`, `sh -c`, script operands | Partial | Function arguments/return, temporary assignment restoration, shared variable/export effects, sourced definitions/variables/positional changes and copied redirected bodies tested. Dot with explicit arguments retains Clarice’s scoped-argument extension; no-argument dot shares the caller’s positional parameters. Redirection attached to a function *definition* is rejected; redirecting its call or body works. |
| Compound-command redirects | Fully supported (fds 0–2/forms below) | Groups, subshells, loops, if/case; scope lasts through completion/break/continue/return. Uses the same ownership/cleanup as simple commands. Nested and pipeline cases tested. |
| `<`, `>`, `>>`, `2>`, `n>&m`, `n<&m`, closure, here-documents | Partial | Descriptor 0–2 mapping and left-to-right ordering; quoted/unquoted here-documents tested. `<>`, general numbered descriptors and complete here-doc edge cases are not supported. Output open/truncate follows shell semantics, not editor safe-save semantics. |
| Finite pipelines and command substitution | Partial / platform-limited | Native HFS spools; finite combinations tested with genuine grep/sed/awk/pipeline ports. Sequential, not concurrent pipes: no infinite producers, SIGPIPE or live IPC. Substitution and expansion storage now grow on demand; 8 KiB substitution tested, with explicit memory/I/O failures and no silent short reads. |
| Subshell grouping | Partial | Environment/functions/cwd and positional-parameter snapshots; `set --`/shift in a subshell or command substitution no longer escape. No separate OS process or complete shell-option isolation. |
| Arithmetic and `test`/`[` | Partial | Integer arithmetic; arity-sensitive string/numeric/file tests, iterative `!` and grouping. Bad/empty/overflowing integers return 2, without confusing false (1) with error. Use shell `&&`/`||` rather than legacy test `-a`/`-o`; Unix ownership/permission predicates remain unsupported. Overflow and advanced arithmetic syntax not comprehensively audited. |
| `read`, `printf`, echo | Partial | Multi-variable `read [-r]`, IFS splitting, escaped newline and EOF; record storage grows on demand. All destination values are staged before assignment, so allocation failure preserves old values. Failure may leave unread input; callers must check status. NUL-containing records are explicitly rejected; native CR is also a line terminator. printf supports `%s`, `%b`, `%c`, `%d/%i/%u/%o/%x/%X`, `%%`, literal width/precision, integer flags, octal/control escapes, `%b` stop escape, format reuse and missing operands. Streaming padding is cancellable; embedded NUL output is preserved. Floating formats and dynamic `*` width/precision remain unsupported. Native integers are 32-bit; host long width is not a portability promise. |
| Pathname expansion | Partial / HFS | Final-component globs sorted; parent-directory wildcards rejected. HFS case/31-byte names and encoded `/`/`%` differ. Host glob shim is not evidence of native glob behaviour. |
| Shell options, traps, exec, readonly, getopts | Unsupported/incomplete | No full `set -e/-u/-f`/trap/exec facility; Positional `set` forms are supported; shell-option controls remain incomplete. Do not silently assume scripts using these features are portable here. |
| Jobs, Unix process IDs, fork, signals, users/permissions | Platform-limited | Not simulated. Cooperative Ctrl+C, no process groups/job control/sudo/Unix permission ownership. chmod is not implemented; native metadata uses hfsattr. |
| Resource bounds | Partial | Source, token and function storage grow with available heap. Source/execution recursion 8/16, parser recursion 32 and deep AND-OR/arithmetic chains 64 protect the native stack. Interactive input/paste stays 1023 B; completion has its existing path-sized buffer. Parser/AST and arithmetic failures now have deterministic host/guest recovery coverage. No full POSIX resource guarantee. |

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
| find, du, file | Partial / HFS; current tree/find and native allocated-space/type/creator output | DFS elastic, no arbitrary depth 8. find supports multiple roots and conjunctive -name/-path/-type (* and ? patterns), with explicit -maxdepth; no -exec/-o/bracket patterns; file is native metadata, not libmagic; du accounts both forks. |
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
  `compound-oom.json`: 18 legacy fault-position trials: six current injected sites plus controls; the
  parser-safety sweep covers 8,268 AST-copy failure positions.
- Earlier utility depth/stream/large-record tests remain in RESULTS.md and AUDIT.md.
  These establish particular capabilities, not every possible Unix option.

Remaining compatibility gaps include the formatter/test subsets above, shell options,
function-definition redirects, parent-component globs and catalog-order ls.
The completed expansion/export/scope batch has [host and Snow evidence](../shell-posix2/RESULTS.md). They are an
inventory of gaps, not authorisation for a shell replacement or OS facilities.

Elastic batch evidence: [host/guest results and memory accounting](../shell-elastic/RESULTS.md). The interactive 1023-byte input limit is unchanged. The original parser-construction crash is fixed. [Parser safety evidence](../parser-safety/RESULTS.md) covers construction, partial cleanup, function copying and real guest exhaustion. Large source parsing can still be slow; lexical scanning does not yet poll cancellation.

## Practical upstream scripts (1544)

Complete unchanged musl specs generator and ncurses 6.5 definition/parameter
generators are exercised with default and space-containing paths, using the full
upstream ncurses_defs and Caps inputs. This is script/text-generation acceptance,
not acceptance of whole Unix build systems. See [practical evidence](../shell-practical/RESULTS.md).
The default parameter generator exposed and motivated `set - Caps` support.
Trap/PID-dependent ncurses helpers remain unsupported; echo-backslash-dependent
helpers are not treated as an unambiguous portable oracle.

Malformed quotes and compound syntax now retain their syntax diagnostics rather
than being relabelled OOM during cleanup. Genuine injected allocation exhaustion
still reports OOM and recovers. The editor environment restrictions were removed
and failure/cleanup tested in build 1545; see ../heap-audit/RESULTS.md.

Build 1545 fixes the native awk heap decline: upstream rewrites argv entries, so
the loader now lends commands a temporary pointer-vector copy while preserving
shell ownership. Matching new-awk runs settle at 362452 B free; ten empty runs
remain at 362352 B. This is a focused leak correction, not a claim that every
workload has invariant heap usage. See ../heap-audit/RESULTS.md.

## Focused close-out (1546)

`ls`/`df` operand collection no longer writes beyond a 32-pointer stack array.
It rescans the borrowed arguments without allocation or mutation: 130 operands
on the host, 100 each in Snow. Directory output remains streaming catalog order;
use `ls -1P | sort` where lexical ordering matters. Parent-component globs remain
explicitly unsupported; use `find` with quoted `-path` patterns for tree filtering.
`find` predicates now all apply rather than replacing earlier predicates. It is
still a filtering/printing subset, not a general ordered action-expression engine.
Startup status/cancellation and full utility pipelines have new focused evidence
in [utility-closeout/RESULTS.md](../utility-closeout/RESULTS.md).

## Utility expansion (1547, Snow 6.0.7 only)

- `cmp [-l|-s] file1 file2`: byte comparison; `-` is stdin. Status 0/1/2
  equal/different/error; no GNU skip operands. Data forks only.
- `cksum [file ...]`: POSIX CRC/length, stdin default, multiple operands.
  No cryptographic authentication or resource-fork coverage.
- 68 actual guest help invocations passed basic availability/format; exhaustive
  option coverage remains pending. Unknown awk options now fail (status 2).
- 1548 adds null/zero standard streams and portable stdio operands; no HFS
  catalog entries, seek or regular-file size. Direct-HFS tools require redirection.
- `command -v [--] name`: one builtin/function/reserved word/PATH name; no
  execution form or -p/-V. `free [-b|-k|-m|-h]` reports current Mac heap metrics.
- 1549 completes the second batch with upstream `dd`, `sleep`, `printenv`,
  `mktemp`, and scoped shell `env`/`time`. See utility-expansion/RESULTS.md.
  `env -i` requires an explicit PATH or an absolute external-command path.
  `time` reports elapsed native ticks only, not fabricated user/system CPU time.
  `dd` is data-fork copying, not a safe editor save; partial output remains on
  failure/cancel. Signed 32-bit offsets; seek needs of=; no conv=noerror.
  `mktemp` uses exclusive native creation and rejects unsafe -u; -d is deferred.
- Final help inventory: 43 bundled names, 32 builtin topics, experimental nano.
  All 76 pass actual guest output/status/format checks; 46 command capability
  contracts plus builtin-help assertions cover advertised content. Exhaustive
  combinations and new physical serial/network/editor execution are not claimed.
