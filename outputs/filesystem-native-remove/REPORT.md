# Batch 3D — native HFS removal safety

Normal build2026111969, based on1968/0ab807e. Host and Snow accepted;
physical acceptance pending. No resolver/enumerator/API change.

The1968 negative control reproduces the exact data loss with production removal
and namespace conversion: native `keep/` is trimmed to sibling directory `keep`,
which is deleted with its sentinel while the requested file survives.
Removal now chooses `:` for explicit native paths and `/` for Unix paths, as
cli_remove already does. Native slash bytes remain literal; suffix rejection
still requires an actual directory and cannot be bypassed with -f.

## Acceptance

- Host ASan/UBSan: exact intended-entry deletion; unaffected nodes, both modeled
  forks and metadata byte-identical; leading/middle/trailing/repeated slashes,
  slash-named directory, Unix/native directory/link suffixes, dangling links,
  empty/missing operands, native faults, allocation failure, cancellation/retry.
  Existing Batch3A/B removal/unlink,25 deep-frame allocation boundaries, enumerator
  overwritten-result/bounds and dot-operand tests also pass (host.txt).
- Normal68000 Snow Plus/4MiB/System6.0.8, maximum acceleration:42 acceptance
  records (guest.json), including actual `native:...:keep/`, untouched sibling
  directory and sentinel, encoded namespace access, native slash-named directory,
  suffix protections, failed operations and successful subsequent commands.
  Existing native fsprobe compares meaningful resource bytes/maps, data fork,
  Finder/extended information and dates; host mocks additionally compare every
  unaffected byte. No new claim about opaque resource-fork reserved bytes.
- Remote cancellation returns130 and a usable prompt (~29ms host elapsed at
  acceleration, not physical latency). Partial disposable tree cleanup succeeds.
  Three10-copy/remove batches: free320312/largest320144 bytes every time;
  remaining command allocations0.
- Three alternating1968/1969 normal rm pairs on the unchanged100-file Batch3C
  fixture: median execution2259→2259 guest ticks; including CLX loading2332→2335.
  One initial baseline execution2778 is retained in raw evidence; subsequent
  baseline2257/2259, candidate2259/2259/2255. No traversal regression or speedup
  claim. The change is before traversal; cached IDs and enumeration are unchanged.
  inspect.clx is byte-identical, preserving the accepted find/du implementation.

## Cost and integration

Shared files CLX21084→21208 bytes (+124 including import/relocation packaging);
code13238→13274 (+36), BSS168 unchanged. Core package438656 unchanged, inspect14284
unchanged. No new allocations, frame fields, persistent memory or startup work.
Full build/import validation and seal pass. Offline Core/installer/helper forks
and Finder metadata, Release/49 utilities and51 USTAR members match sealed inputs.
Publication verification follows in verification.json. Previous1968 distribution
and recovery, unrelated work and other Snow instances preserved. No hardware
access or GitHub push. Stop after3D.
