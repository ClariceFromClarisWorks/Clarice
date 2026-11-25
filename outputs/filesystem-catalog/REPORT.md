# Batch3E — native catalog-name compatibility

Build2026111970, product3fc4251; baseline2026111969/product70ca55c. Normal68000 Snow Plus,
4MiB/System6.0.8, fixed maximum fast-forward; no physical hardware.

The shared encoder now quotes whole catalog names `.`/`..` as `%2E`/`%2E%2E`.
Raw Unix navigation is unchanged. Recursive rm/cp, mv fallback, find/du output,
native conversion, listing/completion share that encoder; shell globbing now
reuses it instead of duplicating slash/percent encoding. Native removal's logical
dot-operand guard applies only to Unix operands. No traversal/resolver redesign.
Percent escapes decode once; slash/percent and escape-looking names stay literal.
Native colon/NUL components remain rejected. Existing path bounds and no-follow
symlink protection remain in force.

Host:1969 negative control deletes the outside-root sentinel; candidate passes
ASan/UBSan,121 injected native failures,20 cancellation points,3 allocation
failures, retry and exact unaffected data/resource/metadata comparisons. Native
wildcard output and511/512-byte bounds pass. Prior3A–3D removal/unlink/depth,
232 fork-copy fault cases and find/du traversal regressions pass.

Snow:42 focused checks pass: nested literal dots, slashes, percent/escape-looking
names, native addressing, Unix navigation, wildcard expansion, find/du, recursive
copy/removal, supported directory moves, cross-volume file moves, symlink rejection,
outside-root forks/metadata, failure recovery, Ctrl+C130 and repeated cleanup.
Additional native `keep/` sibling protection, dot-suffix symlink guard and local
Command-period during actual copy pass; prompt/status130 and cleanup recovered.
An initial directory-move fixture requested the documented unsupported combined
parent+leaf rename; adjusted to a supported native move, without product changes.
Key-event testing required the existing explicit modifier-release chord helper.

Median execution guest ticks (total minus loader),3 identical fixture runs:

| Workload |1969|1970|Change|
|---|---:|---:|---:|
|cp100 files|8646|7698|−11.0%|
|rm100 files|2649|2278|−14.0%|
|find1000 files|682|692|+1.5%|
|du1000 files|621|625|+0.6%|

These are guest elapsed, not physical CPU timings. Copy/removal cold/cache variation
is substantial (candidate first cp8709/rm2800); no speedup is attributed to this
safety change. Small find/du slowdowns are recorded rather than hidden. Native
catalog operations and3C traversal structures are unchanged. No new lookups or
startup work. Repeated nested cycles: free heap321886/largest321718 bytes unchanged,
zero retained command allocations. No new allocation/frame fields/buffers.
Core CODE/1+12 bytes; padded MacBinary+128 bytes. Files code+2 bytes, CLX+4 bytes;
files BSS168 unchanged; inspect CLX14284 unchanged.

Baseline Core forks were read back byte-for-byte against the sealed1969 artifact;
the greeting alone was not used to establish build identity. Offline Core,
installer/helper forks/Finder fields,49 utilities,Release and USTAR contents match
the sealed build. Publication verification is recorded separately. Prior1969
recovery/distribution and unrelated working files remain preserved.
Physical acceptance is pending; no GitHub push. Stop after3E.
