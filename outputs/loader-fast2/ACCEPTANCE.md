# Installed-fast loader acceptance — 16 November 2026

Build 2026111534 changes one loader policy branch: a matching installation receipt
and file identity skip the repeated import/relocation metadata hash. One cancellation
poll remains. Layout, exact length, runtime requirements, imports, entry alignment,
relocation sites/targets and non-overlap checks still run. Full verification remains
for installation/check/strict and unknown or detectably changed files. No receipt
schema, executable format, allocator, image cache or nano change.

## Why it improved

Physical stock SE successive-build comparison (60 ticks/s): cold 356 → 215 ticks;
warm 190 → 85/86. The old metadata/event interval consumed 138 cold and 107–108
warm ticks; the new interval takes 2. This explains essentially the improvement;
small read/preparation differences are run variation, not another optimisation.
Event costs are nested and must not be added again. No bounds checks were removed.

64 focused host scenarios passed under ASan/UBSan with zero leaked allocations or
descriptors. Coverage includes truncated/incompatible/structurally invalid files,
changed identity, failed installation/replacement/recovery, strict verification,
receipt revocation and cancellation. Snow's disposable relocation fixture returned
original=0, tamper=0, fast=0, strict=126, revoked=126, repaired=0, checked=0.
The deliberate in-range corruption is the documented policy risk, not a safety
claim: unchanged metadata cannot establish that executable contents are intact.

Existing production build hash and exact phase counters: measurements.json.
Physical nano runs returned owned=0 and stable free heap; no extra persistent
allocation was introduced. Normal application update/relaunch succeeded and its
previous application was retained. Existing independent-command installation and
rollback mechanisms were not changed.

## Complete nano observations

A new physical session confirmed the build greeting before measurements. Return
submission, screenshot requests/completion and test-key actions are recorded in
se-ready.json and se-ready8.json. These are host-observed bounds including Wi-Fi,
SEControl scheduling and roughly 1–2 seconds of screenshot transfer. They are not
substitutes for exact internal startup-stage instrumentation.

| Case | Fully drawn screen captured by | Input evidence |
|---|---:|---|
| First empty buffer after application launch | 12.26 s | Modified buffer with inserted z at 19.66 s |
| Repeat empty buffer | 9.35 s | Modified buffer with inserted z at 16.36 s |
| Warm 8 KiB, two long lines, extended observation | 19.88 s | Modified buffer with inserted z at 30.53 s |

Input was intentionally sent later than the first frame; those times do not imply
that keyboard readiness itself was delayed that long. Initial six-frame 8 KiB
observations ended during progressive first rendering; do not count their partially
drawn frames as complete startup. The extended observation captures the complete
screen and successful input. All test edits were discarded, never saved.

The reused 8 KiB fixture has two long lines, not an ordinary short-line document.
A cold 8 KiB and a separately characterised short-line fixture remain unmeasured
in this follow-up. The three-second whole-editor target remains unmet. The loader
improvement is accepted independently of that unmet editor target; no nano rewrite
or additional optimisation is claimed.

The new cold empty run loaded in 222 ticks (3.70 s), warm runs in 87 (1.45 s),
consistent with the earlier 215/85/86 samples. Preparation was 139 cold versus
3–4 warm. Heap after nano was 420982 then 420992 bytes, owned remaining zero;
empty peak 74572 bytes and existing 8 KiB peak 101736. Lifetime ticks include the
observation/test delays and must not be quoted as startup time.

## Remaining decisions

Retain receipts in this checkpoint: they are the current checked-versus-unknown
policy gate. Their warm cost is about 0.25 s. Installation, replacement and recovery
need full candidate checking, staging and backups, not receipts intrinsically.
Removing receipts would broaden structural-only normal execution to unknown files
and needs owner review. See ../loader-review/SIMPLIFICATION.md.

Cold preparation remains the main loader opportunity. Existing NewPtr allocation
subphase instrumentation should distinguish image, metadata, stdio setup and final
identity/close work. Do not reserve permanent memory or change executable layout
without a measured justification. Native comparison is separate: see
../native-bench/RESULTS.md. No production process model was introduced.
