# Whole-codebase P1 acceptance — build 2026111959

All six candidate corrections accepted in normal 68000 Snow Plus/4MB/System6.0.8:

- Temporary-assignment frames unlinked before freeing; nested/local/remote shell
  execution and host exhaustion/recovery pass.
- CLX relocation ownership cleared before FSClose; 91 host loader/installer cases,
  including injected close failure and subsequent execution, pass under sanitizers.
  Snow rejects a malformed image and continues. FSClose fault injection is host-only.
- Encoded HFS glob bounds checked before writing. Native guest matches/literals pass;
  sanitizer boundary test rejects an oversized encoded result. Creating the extreme
  guest fixture through touch returned -37 before globbing; that boundary is host-only.
- Escaped pattern characters retain lexical quoting in glob/case/parameter trimming;
  bracket matching consumes the escape correctly. 39 dash comparisons and3 host
  cancellation cases pass; guest escaped-star, bracket and trim results pass.
- Telnet dispatch removes bytes consumed by read. Actual inbound Snow queued read
  consumes `touch /tmp/P1-BAD` as data; file remains absent and following command runs.
- Native append writes use EOF-relative PBWriteSync on every write, including after
  seeking; a+ descriptor adaptation preserved. Five guest probes produce exact
  `abXYefGH`, including stdio and O_APPEND seek/write/reopen checks.

Host:347 adapter allocation-failure trials; five parser recovery fixtures
(298/226/246/174/324 trials); remote ownership/service/auth/cancellation tests pass.
Baseline reproductions independently demonstrate dangling frame, escaped-pattern
misbehaviour and sanitizer double-free. Native write failure and ordinary overwrite
regressions also pass. No production changes were needed during final acceptance.

Snow: local Ctrl+C, Command-period and remote Ctrl+C return130; subsequent commands
and prompt restore. Five local file probes: heap367494–367496bytes (2-byte alternating
variation), largest free block367410 unchanged, owned peak4208, remaining0 each run.
`free` before/after:356KiB free/largest, including after purge. Remote recovery also
reports remaining0. Existing optional service allocation is separate from command
ownership. Guest probe total100–108ticks/load33–34; not a matched speed benchmark.

Core comparison with1958: text+648, data+68, BSS−12bytes; package435584→436352(+768).
No new permanent allocation/startup work. Pattern quoting uses temporary owned
storage; append intent uses the existing opaque file token, no per-open heap block.
Normal optional telnetd40964bytes is published separately, not auto-installed as a
standard utility. Prior optional vi remains at accepted1951. No physical acceptance
or GitHub push; unrelated working files and historical worktrees retained.

Publication: sealed Core/49 standard utilities, preserved optional vi, offline tar
and HDA with fork/metadata readback, committed-source archive and optional telnetd.
See verification.json for canonical HTTP/AFP equality and prior1958 retention.
