# Userland reuse audit — 17 November 2026

Scope: Clarice accepted base `0686839` and the separate Clarice-Network working
branch. Existing provenance, utility compatibility, 0.5 performance, shell,
Telnet and network reports were reused. The release-candidate checkout was not
modified. This is a source-level inventory and first remediation batch, not a
claim that every alternative has been ported or benchmarked.

## Inventory and decisions

Classification: **U** genuine upstream with thin adapters; **M** materially
modified upstream; **I** independent Unix-compatible implementation; **N** native
Macintosh integration. Mixed classifications describe real boundaries.

| Commands/components | Provenance and meaningful limits | Decision / alternative |
|---|---|---|
| grep, wc, head, cut, tr, uniq, tee, basename, dirname, cmp, cksum, sleep, printenv | U: pinned sbase, musl regex/UTF helpers where used; C/byte locale, documented options rather than GNU extensions | RETAIN genuine ports. Shared paths/stdio are Mac adapters; GNU option count alone is not a replacement case. |
| sed | M: sbase, isolated append/substitution and cancellation adaptations | RETAIN measured fixes; upstream source remains available. GNU `-i` is not an accidentally removed sbase feature. |
| sort | M: sbase plus disk-spill merge implementation; bounded batches, not a fixed total input ceiling | RETAIN spilling instead of reverting to whole-file memory use. Compare richer key options only against demonstrated workflows. |
| tail | U/M: sbase with Clarice stream/cancellation adaptations | RETAIN; cancellable `-f` is useful native integration, not a reason for a Unix process layer. |
| awk | M: One True Awk, elastic allocation and native environment/stdio adapters | RETAIN. Process-spawning functions remain platform-limited; explicit recursion safeguards differ from removed arbitrary record/environment limits. |
| dd | U/M: sbase plus checked size/offset and native file adaptation | RETAIN. Native file offsets, seek/output constraints and unavailable recovery options remain documented; no silent widening of HFS limits. |
| mktemp | U/M: sbase with native exclusive temporary-file creation | COMPLETE later: `-d` is disabled although native directory creation exists. This is a real removed capability, unlike deliberately rejected unsafe `-u`. Reuse upstream frontend and add exclusive directory adapter with collision/failure tests. |
| ls | I/N: HFS catalog listing, flags/forks/creation time; catalog ordering | RETAIN native enumeration; selectively complete Unix ordering/options when useful. A plain sbase/BusyBox replacement would need the same HFS metadata adapter. |
| cat, df | I/N: simple streaming cat, native volume accounting | RETAIN: cat's portable core is small; df must report actual Macintosh volumes, not fictitious Unix mounts. GNU transformations are not required for correctness. |
| cp, mv, touch, rmdir, mkdir, rm, ln, readlink | I/N: native fork-preserving operations, elastic traversal, Clarice links | RETAIN native operation layer. Upstream frontends may add options later; wholesale POSIX filesystem wrappers risk losing forks/metadata. No Unix ownership/hard-link emulation. |
| find | I/N traversal and expression subset; previously custom wildcard matcher | COMPLETE NOW with genuine musl fnmatch. Keep native traversal. Still conjunction-only; `-o`/`-exec` remain unsupported, not claimed complete POSIX find. |
| du | I/N: native fork/allocation accounting and traversal | RETAIN native walking/accounting; share proven traversal, not a second filesystem implementation. |
| file | I/N: Finder type/creator inspection, not content magic | RETAIN accurately labelled native inspection for now. A genuine libmagic port would materially add content identification, but database, dependency and RAM costs need measurement before replacement. |
| od | I: limited dump formats | COMPLETE/PORT candidate: traditional/sbase od frontend over existing streams. Practical benefit is more standard format/offset handling; target build size and RAM not yet measured. |
| more | I/N: bounded native pager | RETAIN for current purpose. A richer upstream pager is justified only with navigation/search demand and verified terminal support. |
| hfsattr, free, benchls, benchscan | N: HFS attributes, application heap, diagnostics | RETAIN; Unix chmod, /proc free and synthetic benchmarks are not equivalent native replacements. |
| serial | N: Macintosh Serial Driver client | RETAIN native full-duplex/cancellation/terminal integration. Unix termios is not the host interface. |
| kermit | M: E-Kermit 1.8, BSD-3-Clause; short packets, one window, CRC16, bounded adapter and safety fixes | RETAIN genuine protocol engine. Long packets/attributes/batch/text modes are configured out; evaluate those actual losses when transfer use requires them. Preserve baseline and upstream source. |
| wget | I/N: HTTP client over native transport; length-delimited successful responses, limited HTTP behavior | COMPLETE using established parser/chunk decoder: picohttpparser is the strongest small candidate. Redirects/chunking/status handling materially matter. Shared updater download contract makes this a separate tested batch, not a blind parser swap. TLS remains separate scope. |
| clarice (update), appinstall | N: staged Macintosh application/CLX installation, receipts and recovery | RETAIN native safety mechanism. Not substitutes for a Unix package manager. HTTP improvements must preserve limits, cancellation, both forks and rollback. |
| ipconfig, ping | N: installed stack status/ICMP, system DNR | RETAIN; do not replace MacTCP with a socket/TCP stack. External standard utilities, no Core idle networking cost. |
| ftp | I/N: EPSV/PASV binary transfer/listing, staging, native transport | COMPLETE selectively or port ftplib after transport/size prototype. Existing ftplib adds mkdir/rename/etc but directly depends on sockets/select/DNS and inspected version lacks EPSV. Do not lose peer-address safeguards or cancellation to gain option count. No replacement yet. |
| telnet | U/N after libtelnet port: RFC1143 option state and parser over native transport | PORT completed in preceding batch; RETAIN. Recovery client preserved. Host and Snow accepted; new engine still awaits physical acceptance. |
| VT100/ANSI model | I/N: tested bounded cell model and native renderer | Replacement requires review: libvterm screen cells alone cost 72,000 B at 80x25 versus roughly 11 KiB current model. Parser-only adaptation could retain cells but is a substantive compatibility bridge. Do not introduce a second state machine or replace rendering. |
| nano 9.2 (optional) | U/M: upstream tiny configuration, thin HFS safe-save/environment adapters | RETAIN; tiny configuration removes useful functionality (including richer editing features). Selective re-enabling needs measured memory/startup acceptance, not a second editor rewrite. |
| ncurses 6.5 | U/M: genuine library, Clarice terminal entry and native I/O boundary; narrow build, reduced database | RETAIN; configuration is explicit, not an imitation curses API. |
| mrsh parser/AST/arithmetic | M: upstream MIT, elastic construction/failure fixes | RETAIN validated parser; custom shell executor/expansion is not a full upstream mrsh port. |
| shell execution/expansion and builtins | I/N: portable subset over single-Finder execution; dash comparison suite | COMPLETE demonstrated practical gaps using upstream components. Full ash/dash execution requires process/descriptor assumptions; no parser replacement justified. Globbing remains limited to final path component. |
| newlib, libgcc, musl regex, sbase UTF/support | U/M: toolchain C library plus isolated wrappers and measured byte/UTF fast paths | RETAIN source/licences. Shared native runtime overlays are necessary, not a claim of complete libc/POSIX. |
| CLX loader, stdio/FD, heap, HFS paths/links, safe saves, terminal/input/progress | N with upstream libc helpers | RETAIN native boundaries and established safety tests. Unix kernel emulation would add cost without host support. |
| MacTCP/MoofTCP transport and DNR | N/reused Macinclaude MIT transport with adapters | RETAIN installed-stack integration. HTTP/FTP/Telnet remain independently replaceable protocol engines. |

Builtin inventory covered: env, time, command, appinstall, cd, pwd, echo,
printf, export, unset, set, shift, read, sh, `.`, test/`[`, true, false, `:`,
break, continue, return, exit, history, prefs, status, clear, help, selftest,
checks and scrollbench. Native UI/control builtins should remain native;
printf/test/expansion gaps need practical dash comparisons, not Bash extensions.
All 43 standard external names are represented above, plus four Network additions
(ipconfig, ping, ftp, telnet). Nano and development ncurses probes are optional.

## Ranked next work / escalation

1. **Completed:** replace find's incomplete wildcard component with musl.
2. **Contained next batch:** restore upstream mktemp directory mode through a
   native exclusive-directory adapter; collision, cancellation and cleanup tests.
3. **High practical benefit:** picohttpparser response/chunk handling for HTTP.
   Keep transport and updater safeguards. Caller-owned header storage must grow
   safely; do not exchange one parser for arbitrary silent header limits.
4. **Moderate:** genuine upstream od capabilities; measure a target build before
   selecting replacement. No speed/size claim from source inspection alone.
5. **FTP:** prototype ftplib transport boundary before deciding wholesale port;
   current EPSV, safe staging and peer checks are regression requirements.
6. **Review required:** libvterm wholesale screen replacement has a major memory
   increase. Prefer a scoped parser/state integration assessment if expanded
   terminal compatibility is requested. No renderer replacement authorised here.

## First batch: find / musl fnmatch

Pristine musl implementation, pinned to the existing musl revision, with separate
C-locale byte adapter. Adds bracket ranges, negation, ASCII classes and escapes.
MacRoman bytes match literally; no UTF-8 or locale alphabetic folding claim.
Upstream handles unmatched `[` literally; Darwin fnmatch differs, so tests use
pristine musl as the behavioral oracle. Matcher is nonrecursive and allocates
nothing. Existing HFS traversal, cancellation and cleanup remain unchanged.
Recovery: prior source is in parent Git history; local matched before/after CLX
images retained in ignored work/reuse-audit. Production RC has not changed.

Host: 256 pristine-musl comparisons under ASan/UBSan, high-byte cases and long
adversarial pattern pass. Current traversal harness passes two-volume handling,
encoded/raw names, links, forks, allocation failures, cancellation, catalog
failure and recovery. Older utility-correctness traversal harness has stale mock
HFS definitions; it was not used as evidence or broadly rewritten.

Snow: Plus/4 MB fixture, accelerated fixed fast-forward (~16x), actual System
6.0.8 fixture (the known greeting reports 6.0.7). Guest script completed; extracted
outputs independently checked. Ranges, negation, classes, escaped wildcard,
unmatched bracket, help, missing-path error/status, ordinary and 1,000-file
traversal pass. Cancellation/fault injection is host evidence for this batch;
no new physical-SE or guest cancellation claim.

| Cost | Before | After |
|---|---:|---:|
| CLX file bytes | 10,152 | 12,892 |
| Loaded image bytes | 6,832 | 9,172 |
| Small traversal total ticks, three runs | 93 / 87 / 86 | 96 / 91 / 91 |
| Small loader ticks | 62 / 57 / 56 | 67 / 60 / 61 |
| 1,000-file total ticks | 1342 / 1331 / 1330 | 1385 / 1370 / 1369 |
| 1,000-file loader ticks | 57 / 63 / 61 | 60 / 61 / 60 |
| Post-command free heap | 382984 stable | 382972–382974 alternating |
| Largest block | 382816 stable | 382790 stable |

About 3% more total guest ticks for large traversal, 2,740 B executable growth,
2,340 B image growth; no permanent matcher allocation or progressive heap loss
in six matched invocations. Native heap accounting differs by 10–12 B between
executables, not a growing per-run loss; do not claim byte-identical heap.
Tracked command allocation count is zero but is not a measurement of every
native allocation. Timing is comparative accelerated guest ticks with full
verification for disposable executables, not physical seconds or installed-fast
performance. Functional improvement justifies a development checkpoint; final
physical performance acceptance remains pending.

## Sources / licences / unmeasured costs

- [sbase](https://git.suckless.org/sbase/): MIT; exact pin in
  src/vendor/USERLAND-SOURCES.json. Traditional small utilities rather than GNU
  compatibility are intentional; avoid turning missing GNU options into defects.
- [musl fnmatch](https://git.musl-libc.org/cgit/musl/tree/src/regex/fnmatch.c): MIT,
  Rich Felker algorithm attribution retained; pin 7813e9119eb5fe681b6750b8a033a0490bca0941.
- [One True Awk](https://github.com/onetrueawk/awk): preserved permissive notices,
  pin 72e9b56caddb4a6f8d1405315a5183c26b8650a4.
- [libtelnet](https://github.com/seanmiddleditch/libtelnet): public domain,
  authors retained. Existing LIBTELNET-PORT report records 17,924→24,920 B CLX,
  574 B calculated target protocol state and demand-grown subnegotiation storage;
  no new permanent Core allocation.
- [picohttpparser](https://github.com/h2o/picohttpparser): MIT/Perl dual licence,
  inspected dd573d8fa9264c646a400e96ce9f07dea6f156e4. Caller-buffer parser and
  in-place chunk decoder; target executable/CPU/RAM costs not yet measured.
- [ftplib single-header distribution](https://github.com/mattiasgustavsson/libs/blob/main/ftplib.h):
  Thomas Pfau / Mattias Gustavsson, inspected Artistic-2.0 notices. Do not confuse
  this version with older differently licensed releases. Target costs unmeasured.
- [libvterm](https://github.com/neovim/libvterm): MIT, Paul Evans; inspected
  934bc2fbf21800ac3458a499df8820ca5fb45fd3. Actual m68000 compiler sizeof(ScreenCell)
  is 36 B, hence 72,000 B before other state or alternate screen. Not a complete
  linked port benchmark.

Existing BusyBox/NCSA Telnet comparison remains in outputs/telnet; do not repeat
or transplant GPL code into differently licensed components without preserving
its terms. No claim that every BusyBox/ubase alternative has been benchmarked.
