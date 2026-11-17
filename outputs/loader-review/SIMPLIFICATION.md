# CLX simplification assessment — 15 November 2026

Review following accepted checkpoint 7eb7064. No receipt removal, format change,
or process replacement is implemented. The already-authorised metadata-checksum
experiment (1534) has passed the focused loader acceptance; see the follow-up
results below and ../loader-fast2/ACCEPTANCE.md.

## Native-first reassessment (AGENTS 43eb445)

The new principle is present on the working branch and has been read. It does
not itself authorise changing integrity policy. Separate three kinds of work:

- Native responsibilities: HFS access/forks/catalog identity, application launch,
  native CODE/resource loading, memory partitions, allocation and scheduling.
  Clarice should call these facilities, not build replacements.
- Actual compatibility gaps: shell arguments/environment, standard-stream binding,
  exit status, cooperative cancellation and single-Finder returning execution.
  Keep only the glue those demonstrated requirements need.
- Optional policy machinery: installation receipts and repeated content hashes.
  These are not native memory-safety services; remove or retain them according to
  explicit product policy and measured value, not as a simulated security layer.

CLX already uses native HFS calls and NewPtr/DisposePtr; those adapters are not
replacement filesystems or heap managers. Its avoidable overhead is additional
work requested around them (sidecar I/O, repeated identity queries and hashing),
not evidence that native HFS or allocation should be rewritten.

CLX image/import relocation is custom loading which a native APPL backend could
replace, but its present returning single-Finder execution is a real capability.
Do not claim it is redundant merely because System 6 can launch an application.
A callable resource still needs an entry/global-state/service contract; a separate
APPL still needs command lifecycle and stream glue. Keep both comparisons small.

## Recommended division

| Mechanism | Recommendation | Reason |
|---|---|---|
| Magic/version, exact file length, overflow-safe layout arithmetic | Every launch | Reject truncated or impossible layouts before allocation/read. |
| Entry alignment/range; relocation site alignment/range/order/non-overlap; target/import bounds | Every launch | Prevent loader writes outside its image and invalid dispatch. |
| Terminated import names, available symbols, runtime/capability requirements | Every launch | Runtime bindings can change independently of an installed file. |
| Read/close/allocation errors, cancellation, cleanup | Every launch | Resource recovery and predictable failure. |
| Header checksum (44 bytes on format 2) | Keep initially | Measured zero/one tick; inexpensive malformed-header detection. |
| Complete payload checksum | Installation, repair, explicit strict/check | Detect damaged transfer/install; stage and verify before replacing, retain rollback. |
| File identity/receipt matching and receipt CRC/path lookup | Candidate removal | Supports the checksum policy, not relocation bounds or executable compatibility. Adds HFS work, sidecars and invalidation paths. |
| Repeated relocation-metadata checksum | Candidate removal from normal launches | Duplicates installation content verification; structural checks still apply. |
| Receipt metadata checksum stored for future launches | Remove if receipts go | No remaining consumer in the simpler policy. |
| Replacement backup, validated staging and recovery | Keep | These prevent loss of the installed working version; independent of launch receipts. |

A simple candidate policy is: normal launches perform structural/runtime checking;
appinstall validates the complete candidate before replacement; `appinstall check`
and `CLX_VERIFY=strict` perform complete verification without depending on receipts.
Use the same existing CLX format. No replacement receipt/database or resident cache.
An explicit strict policy can still verify every invocation. Unknown paths would
receive the same structural-only normal policy: this is a material policy change
and is proposed, **not implemented**. Existing sidecars can initially be ignored;
there is no need to delete unrelated configuration during migration.

Neither a receipt nor matching HFS identity proves absence of silent corruption
or tampering. Structural checks cannot distinguish an in-range corrupted pointer
or machine instruction from intended code. Classic native applications likewise
are not protected by a memory-safety boundary. Installation checksums detect
accidental corruption, not authenticity. Removing receipts broadens that existing
risk to all normal launches, including independently copied commands; strict/check
is the diagnostic escape hatch, not a claim of sandboxing.

## Measured costs and limits

Accepted physical SE 1533: cold loader 356 ticks (5.93 s), warm 190 (3.17 s).
Warm metadata 107–108 ticks; receipt work 14–16; imports 4; relocations 22;
reading 35–38; preparation 4–5. Events are nested, not additive.
Removing the complete metadata interval would give 1.37–1.38 s arithmetically;
removing record work as well suggests roughly 1.1 s. These are **estimates**,
not measured native or simplified-loader results, and required event servicing
must remain. Receipt-related final identity work also sits in preparation, so
it cannot be precisely attributed from these counters.

The local 1534 experiment retains receipts but omits metadata rehash: actual Snow
cold 224 ticks (3.73 s), warm 102/102 (1.70 s); read 62–63, metadata/event poll 1,
imports 4, relocations 19–20, preparation 126 cold / 4 warm, record 11.
Free heap 421188 on all three exits, owned remaining zero. This is loader-only;
editor lifetime includes automation delays. Physical SE final experiment: **cold 356 → 215 ticks (5.93 → 3.58 s)**;
warm 8 KiB **190 → 85/86 ticks (3.17 → 1.42/1.43 s)**, about 55% less warm
loader time. Same stock SE and fixtures, successive builds, not alternating A/B.
Metadata/event stage is 2 ticks; warm read 37/38, imports 4, relocations 23,
preparation 3/4, receipt 16/15. Cold preparation remains 132 ticks.
All three exits: heap 420984 bytes, owned remaining zero. No new allocation;
heap is 24 bytes above the prior sample, with differing fragmentation. Source
and fixtures are preserved in the development checkpoint; the authorised
experiment is retained, without an additional receipt-removal change.
Evidence: ../loader-fast2/measurements.json and se-nano-{0,1,2}.png.
64 host installer/loader cases pass, including valid-shaped corruption allowed
in fast mode then rejected by strict/check with receipt revoked, structural
corruption rejected, changed identity fallback, cancellation and recovery.
Snow disposable metadata fixture also passes: original=0, mutation=0, fast=0,
strict=126, revoked receipt next-run=126, explicit replacement=0, check=0.
Only an unused relocation target was changed, with original HFS identity restored;
no instructions or user files were modified. Evidence: ../loader-fast2/snow-metadata.png.
No new claim that all corrupted executables can fail safely.

## Native APPL, hybrid and elastic memory

Reuse the prior [native review](REVIEW.md) and its primary-source/toolchain evidence.
Apple describes native loading in terms of CODE resources, A5 jump tables and
LoadSeg locking/address patching ([Segment Manager description](https://dev.os9.ca/techpubs/mac/runtimehtml/RTArch-118.html)),
not a Clarice-style receipt database. This is a design comparison, not a proof
that every historical loader rejected every kind of damage. Native loading is
simpler at the executable-container layer; it does not provide
Clarice argv/environment, terminal streams, completion status or cancellation.
Retro68 native startup still relocates. A hybrid APPL that loads CLX retains its
loader costs and adds launch/communication costs. A direct-linked APPL is the
meaningful comparison, with whole round-trip and partition memory measured.

The existing updater's transfer launch exits its caller: its success is not a
matched command benchmark. A minimal returning MultiFinder probe still needs a
bounded invocation/completion mechanism; the follow-up probe now measures it. Do
not claim native speed gains from omitting that work. Single Finder should retain the
in-process backend; switching to native processes globally is inappropriate.

Current memory already grows by demand, with 2 KiB invocation-owned small-object
pages and native large allocations. The obsolete 96 KiB runtime cap is gone.
The 32 KiB free-headroom policy is not an allocated reserved buffer. Genuine
limits are the application partition, contiguous blocks and stack. A separate
APPL can select a larger partition at launch but cannot borrow arbitrary free
system memory transparently; it also duplicates startup/runtime state. Evaluate
larger workloads and total 4 MB coexistence, not just command heap size.

## Next decision

Recommend structural-only normal CLX loading, complete installation/strict
verification, and removal of receipt machinery **as a proposed policy**, after
review of the broader unknown-command corruption tradeoff. The already-started 1534 physical update/relaunch and loader experiment completed
safely; previous application recovery remains. Keep cold allocation preparation
as a separate measured investigation. Do not replace CLX until a matched native
round-trip demonstrates a benefit large enough to justify its command transport.

## Receipts by operation — follow-up

- **Absent-only startup installation:** destination existence controls installation;
  a receipt neither authorises overwriting nor protects the backup.
- **Candidate installation and independent update:** full candidate verification
  and runtime compatibility remain necessary. A receipt is only a later fast-launch
  optimisation, not evidence that a candidate may replace an existing application.
- **Repair/replacement:** explicit intent, verified staging and native HFS rename/
  rollback preserve the old file. Receipt invalidation prevents stale fast trust;
  it is necessary only while receipt-gated verification exists.
- **Recovery:** retained prior files are the recovery mechanism. Receipt invalidation
  forces restored/changed content through full verification; receipts cannot repair
  a file or establish that an old backup is good.
- **Ordinary execution:** receipts provide the distinction between previously checked
  unchanged files and unknown/changed files. This is their remaining independent
  value, and costs 15–16 physical ticks (~0.25 s) per warm nano launch, plus some
  identity work in preparation. Removing them changes the unknown-file policy.

Retain receipts for this accepted-policy experiment. Recommend removing them only
if the owner accepts structural-only ordinary execution of unknown as well as
installed applications. Do not replace them with another trust database. Do not
remove staging, explicit replacement, complete installation checks or backups.

The minimal native probe now returns successfully in Snow: CLX 25/27/27 ticks,
native including driver 126/127/126 ticks. See ../native-bench/RESULTS.md for the
transport and timing boundaries. Physical follow-up: CLX 34/34/33 ticks versus native 157/155/155;
no native speed advantage for this small-command/file-exchange workload.

Cold preparation remains 132 physical ticks versus 3–4 warm. Existing allocation
instrumentation can separate image and metadata NewPtr from stdio and final HFS
identity/close work. First investigate native heap compaction/purge and allocation
order with the same clean-launch fixture. Do not retain images or preallocate a
permanent arena. Combining allocations may reduce allocation calls but prevents
releasing metadata before execution; that tradeoff needs measurement and is not
implemented. There is no evidence yet that replacing NewPtr would eliminate its
cold cost safely.

Deployment observation, 16 November: physical Finder contained many prior `.cu-*`
applications and an unclear current application name (`Clarice Updatastute`). A
scrolled directory initially hid the current copy. Build 1534 was found and its
welcome/version verified remotely before resuming tests. The owner requests a
clear current application plus identifiable retained recovery copy and cleanup of
obsolete staging. This is recorded as an updater usability issue, not permission
to delete ambiguous backups during the loader investigation. Updating an existing
copy is already the intended workflow; no new update system is needed.
