# Batch 3A — filesystem P1 corrections, build 2026111966

Product **15f2c3e**, integrated local main. HTTP/AFP readback passed all51 entries
and36 unique payloads; previous1965 index/payloads remain verified.

Both defects reproduced against accepted 8d136a1 with production function bodies
and ASan/UBSan. Oversized cp/mv operands overflowed the destination stack buffer;
trailing separators made the real namespace resolver follow a final removal link.

The correction is confined to files.c: reject encoded destinations >=512 bytes
before native lookup/source work; strip trailing separators for the existing
no-follow removal lookup, requiring a real directory. The latter rejects even
with -f. Ordinary directories, contained links, final links without separators,
and existing dot/dotdot rejection retain their behaviour. No namespace rewrite.

## Acceptance

- `python3 tests/filesystem-p1/host.py`: baseline negative controls; 511/512/1024
  destination boundaries, percent/slash encodings, real resolver/removal decisions,
  native failure, allocation failure, cancellation, recovery and cleanup.
- Existing copy fault suite:116 cases (native failures, short writes, cancellation,
  forks/source/destination preservation); deep cp/rm25 allocation boundaries;
  dot/dotdot, locked-source/native-move and cwd regressions all pass under sanitizers.
- Normal 68000 Snow Plus/4MiB/System6.0.8, fixed maximum fast-forward mode (~15x):
  43 checks in guest.json. Actual511-byte encoded destinations work;512/600 reject
  without mutation. Links with slash and dot/slash suffixes reject with target and
  link intact; ordinary slash directories and contained links work. Host fault
  injection covers exhaustion; no claim of new physical-hardware testing.
- Local Ctrl+C and Command-period and remote interrupt return130, subsequent
  commands/prompt recover. Three20-copy/delete batches: free321292/321300/321292,
  largest321124/321132/321124 bytes; remaining command allocations0. No progressive
  loss. Final settled free324756/largest323098; remote service remains loaded.
- Valid upstream-Rez resource fixture: data and meaningful resource bytes, Finder
  type/creator/flags and dates preserved after copy/move. Rejected removal target
  and cancelled-copy destination retain *every* fork byte. Native copies alter
  only system-reserved resource bytes48..125, matching prior documented behaviour.
  An initial opaque synthetic resource fixture showed the same native scratch
  updates; it is not used to claim byte-exact opaque-fork preservation. See
  forks-guest.json/forks.json and `tests/filesystem-p1/check-forks.py`.
- Offline HDA: Core, installer/helper forks, metadata, Release and49 utilities
  match build inputs; all51 USTAR members match the sealed kit.

Shared files image20480→20636 bytes (+156; code+132, relocation/package+24).
Core MacBinary438528 unchanged; Core code/data/BSS and files BSS168 unchanged.
No new heap allocations or startup work. One operand strlen and one initial
removal-kind check; no performance speedup claimed or physical timing inferred.

Publication uses the existing sealed repository/immutable payload process;
verification.json records HTTP/AFP readback, previous1965 retention and product
commit. Unrelated working outputs preserved. No GitHub push or hardware access.
Stop after Batch3A; the separate Batch3 P2 findings remain outside this change.
