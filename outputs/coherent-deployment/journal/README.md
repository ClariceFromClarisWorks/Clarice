# Recovery journal decoding — 18 November 2026

Contained prerequisite work; coordinated installation is still unfinished.
No Core or standard utility was rebuilt/replaced. Physical SE untouched.

`release-journal.h` separates the existing record encoding from the native file
adapter and adds bounded validation/decoding. The experimental, never-deployed
v1 record lacked inventory names: identities could be applied to the wrong
command after an inventory reorder. v2 binds ordered names, trusted installation
paths, old/new file identities and a CRC. Record size 3,628 bytes (previous 2,476).
CRC detects damage, not authentication. No heap allocation or resident cache.

`release_read_journal` checks exact data-fork length, short reads and close errors,
and only changes the caller's transaction after full validation. Unknown versions,
malformed paths, incomplete identities and identical old/new identities fail.
Native adapter compiles against pinned Retro68/68000 headers. It is not hooked
into ordinary startup or the production update path yet.

Actual checks:
- Host ASan/UBSan: successful round trip, output/path aliasing, all 3,628 byte
  positions corrupted, every shorter length, oversized input, wrong installation
  roots/targets, malformed paths, inventory reorder and invalid identities.
  Failures leave output unchanged.
- Existing collection model: 42 injected failures and 42 interruptions still pass.
- Production CLX loader validates the final 21,224-byte test image and imports.
- Snow Plus/4 MB/System 6.0.8 at fixed fast-forward (~15x): same test body, sampling
  corruption positions but testing every shorter length, PASS/status 0. Free heap
  348,838; largest block 345,448; tracked remaining 0 after each of two runs.

Reproduce host codec: compile tests/release/journal.c with ASan/UBSan and execute.
Build guest fixture: `sh tests/release/build-journal.sh`; run the resulting
work/release-journal/journal-test as a disposable /tmp command. The build also
compiles the native reader probe. It does not rebuild Clarice or alter the bundle.

The first fixture incorrectly imported printf; loader correctly rejected it.
Corrected fixture uses existing say output and passes. One automated key-release
was delayed during accelerated input, producing a repeated character and a missing
command; the download succeeded and the corrected command was rerun. Neither
failed attempt is counted as acceptance.

Limitations: guest tests exercise the codec, not native journal I/O error injection
or recovery renames. Completion-marker handling, restart gating, native failure
and interruption tests, and installer/updater integration remain outstanding.
No power-loss atomicity or completed coordinated update is claimed.
