# Approved BusyBox package replacements — 18 November 2026

Integration branch codex/busybox-replacements-20261118, based on validated current
main-development checkpoint 1cb4844. Private build 2026111882. No physical SE use,
push or release. Genuine BusyBox 1.37.0 and the comparison adapters from a2a9595;
normal command names now select the approved wc/tr/sort implementations.

## Package behavior

The standard catalog maps wc/tr/sort to bbwc/bbtr/bbsort images. wc-sbase,
tr-sbase and sort-sbase retain the current integrated sbase implementations.
Compatibility commands have no automatic expiry: they remain normal installed
packages until explicitly retired. Together their current payloads occupy 88212 B
(about 86 KiB) on disk, with no idle RAM allocation. They are separate from
updater recovery copies.
Both offline kit and repository derive from this single catalog. Existing
updater versioning/recovery is reused; no shell dispatch hack or loader change.
Full source build uses current runtime1.10; BusyBox packages require runtime1.9.
Only these three BusyBox applets are built for the standard collection. Other
comparison adapters remain separable source, not installed commands. BusyBox vi
remains optional pending its recovery work; no vi default is changed here.

The source archive is verified against its pinned SHA256 and extracted freshly.
Authorship/licences are retained; newlib getopt retains its public-domain notice.
Help identifies the actual standard command name and points to compatibility
commands. No new resident allocations or startup caches are added.

## Compatibility

wc -m is byte-oriented; word rules differ from UTF-8 sbase. tr translates bytes,
with ASCII classes; no multibyte translation, [:graph:]/[:print:] or [x*N]. Byte
preservation is not MacRoman case folding or Unicode conversion. Use the explicit
sbase commands for their previous behavior; those are not fully MacRoman-aware.

sort is all-in-memory and releases invocation allocations on return/cancellation/
error. Use sort-sbase for disk spill and previous -o/-m/-C behavior. BusyBox rejects
-o/-M/-V/-m/-S/-T; redirect to a separate file. Redirection is ordinary Unix-style
output, not atomic editor replacement. No automatic retry or fallback is hidden.

## Validation

Fresh full 68000 build passes. The production loader validates all 35 distinct
images/imports; the sealed kit contains 50 commands. Ten release packaging tests
pass. Replacement/compatibility artifact checks confirm kit and HTTP payloads,
versions, hashes and BusyBox licence. All 50 installed catalog records match the
sealed version/size/CRC after updating the private guest from Core2026111823 to
2026111882; the native helper automatically relaunched the new Core and cleared
the incomplete-update marker.

**101 guest observations pass**, covering all six loader/help entries, BusyBox
identity on the standard names, matched output and three-run 8 KiB timings,
invalid options/status/stderr, empty streams/pipeline, Unicode count differences,
MacRoman byte preservation, NUL deletion, locked-file preservation, sbase -o and
disk spill, cancellation/status130, memory exhaustion/status2 and recovery.
All timed invocations report zero owned survivors. A 64 KiB sort-sbase spill run
completed in49.68s, peak25964B. BusyBox sort's256KiB input failed explicitly in
17.85s, peak300684B, remaining0; the next wc invocation succeeded.

The harness initially used an unsupported `status version` query; it was corrected
to `status runtime` and the complete run restarted. This was a harness error,
not a production or Telnet failure. No new Telnet defect observed. The optional
Telnet service in this fixture was separately updated to the already validated
peer-fix source; it is not added to the standard collection.

Matched 8 KiB medians (existing sbase / BusyBox seconds): wc-c2.45/1.63,
tr6.61/1.20, sort2.56/2.75. The previous comparison's large-file sort advantage
is not claimed as a new measurement here. Startup samples are not cold. These
are Snow results, not physical performance acceptance. Nano goals and separate
vi recovery remain outside this packaging task. Tests are in tests/busybox-replacements. The guest uses private
cloned disks, own Snow bundle org.clarice.ReplacementSnow, TCP23244 and HTTP8098.
Password is read from the existing untracked test fixture and is never logged.

## Delivery

Standard collection switches only wc/tr/sort; the explicit compatibility packages
remain until deliberately retired. The complete offline kit and private HTTP
repository are retained under work/distribution and work/replacement-snow/http
in this worktree. No public repository publication or physical deployment.

Recommended next step: physical acceptance of the approved package set when the
SE is available. Keep encoding and spill limitations visible; do not remove the
compatibility packages merely because a performance test passed.

Final no-change updater run reported all standard tools current and fetched only
the index. Private Telnet service disabled, Clarice quit to Finder, Snow workspace
closed and private HTTP server stopped. Test images and artifacts are retained.
