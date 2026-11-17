# Greeting and updater review — build 2026111702

17 November 2026. Focused follow-up to candidate1701; no physical-SE testing,
release, main change or push. Previous candidate artifacts remain available.

## OS greeting — fixed

The fixture's System file contains `vers`1 = `06 08 80 00 00 00` (6.0.8),
whereas the existing SysEnvirons/Gestalt path reported6.0.7. Hardware discovery
now reads the System file's version through native Resource Manager reference0
and `Get1Resource`, avoiding application-resource shadowing. It restores the
resource-file context and handle state; absent/invalid resources retain the API
fallback. No hardcoded OS version, persistent cache or networking work added.
[Apple's Resource Manager documentation](https://leopard-adc.pepas.com/documentation/mac/MoreToolbox/MoreToolbox-32.html)
documents reference0 for the System file.

Changed production files: `src/hardware.c`, new `src/system-version.h`, and build
identifier `src/update/version.h`. New focused test: `tests/system-version/host.c`.
ASan/UBSan host decoding covers6.0.8,7.1, missing/truncated resources and invalid
BCD. Normal68000 build passes. Snow startup/relaunch displays6.0.8; `free` and
`appinstall check /bin/wget` subsequently return to the prompt without errors
(`resource-restore.png`). System7.1 decoding is host-tested, not guest acceptance.
Core resource fork265190→265350 bytes (+160); no permanent allocation introduced.

## Update error28 — investigated, not reproduced or fixed

Historical reports describe a Macintosh System error28 during relaunch after
replacement, rather than an updater transaction error. They include diagnostic
→normal handoffs; later normal→normal updates succeeded. Existing Core reserves
64KiB stack before MaxApplZone. The small native helper uses a160KiB partition.
Target-compiler stack-use inspection gives main2292B and fork hashing3140B;
a nested validation path is approximately6292B before Toolbox/runtime/interrupt
frames. These static numbers do not establish actual stack margin or causality.
No speculative partition/stack changes were made. If the failure recurs, capture
the exact launching application, handoff phase and live stack/heap margin.

Two independent disposable Snow Plus/4MB/System6.0.8 images updated normal1701
through private-LAN HTTP to1702 using `clarice update -y --source URL`. Both
completed download, verification, exit, replacement and automatic relaunch at
maximum acceleration (screenshots `update-running.png`, `repeat-after.png`).
No error28 appeared. These are functional checks, not stock-SE timing claims.

Offline extraction after guest exit confirmed both installed forks match the
new package and both previous copies match the1701 application. Resource-fork
comparison excludes native reserved/scratch bytes16–127. Type, creator, Finder
flags and creation date are preserved. See `recovery.json`, `first-files.txt`
and `repeat-files.txt`. Recovery remains `.cu-00000010-78c402d6.old` in these
fixtures. No failed-transaction or power-loss safety claim is added; existing
transaction fault-injection evidence remains applicable to unchanged code.

## Build for user hardware testing

- `build/Clarice-2026111702.hda`: offline application, Install Clarice,47 utilities,
  Read Me and licences; existing native-device packaging/driver reused.
- `build/Clarice.bin`: fork-preserving MacBinary Core application.
- `artifact-hashes.json`: sizes and SHA-256 hashes for the delivered artifacts.

Use the HDA as an additional BlueSCSI disk with an unused SCSI ID, open Read Me
and Install Clarice. Preserve the existing working application/recovery copy.
The original1701 candidate and real SE were not modified. Error28 and slow
physical handoff remain known limitations; hardware acceptance remains pending.
