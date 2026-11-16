# Upstream reuse batch: mktemp directories and HTTP — 17 November 2026

Separate Clarice-Network development; Core remains build 2026111550. The physical
SE and Clarice 0.5 release-candidate checkout were not accessed or modified.
Recovery implementations remain in Git history and ignored work/http-upstream.

## Implemented

- Restored genuine sbase `mktemp -d` frontend. Thin adapter resolves the native
  parent and uses exclusive Macintosh directory creation, including collisions
  with files/directories. Existing 32-attempt naming strategy and `-u` refusal
  remain. No Unix permission or cryptographic unpredictability claim.
- Found and fixed a cancellation leak after creation but before printing the
  name. A mktemp-only runtime cleanup hook owns the unpublished result until
  stdout flush succeeds. The path is allocated before creation, removed on
  error/cancellation, then relinquished on success. No other utility enables
  this hook; no new permanent Core allocation. Cleanup failure reports the path.
- Adopted unmodified picohttpparser response parser and chunk decoder, revision
  dd573d8fa9264c646a400e96ce9f07dea6f156e4, MIT option with original notices.
  Native MacTCP/MoofTCP/DNR, same-volume staging, close/rename commit, timeout,
  cancellation and updater maximum decoded-byte limits remain shared.
- HTTP headers grow from 512 to 8192 bytes, preserving the existing wire bound;
  descriptor count comes from received lines, not a fixed field-count ceiling.
  Both allocations are freed before body writing. Chunk decoding is in-place in
  the existing 2048-byte transfer buffer; 68000 decoder state is 24 stack bytes,
  header descriptors 16 bytes each (actual target sizeof measurement).
- Ambiguous CL+TE, duplicate framing headers, malformed chunks, unsupported
  content coding, folded headers and truncated bodies fail without committing.
  Known-length and chunked binary downloads both work. Unknown totals use the
  existing progress display and become known at completion.

## Evidence

Host ASan/UBSan: mktemp ordinary/directory creation, 32 occupied names preserving
existing contents, upstream sbase option/outcome comparisons, missing parent,
invalid template, full/read-only media, allocation exhaustion, output failure
and cancellation before creation and during stdout publication all pass.
Tracked invocation allocations return to zero. Existing eight dd reference
copies and failure/cancellation tests remain passing because they share helpers.

HTTP host: binary chunks over fragment sizes 1–65, extensions/trailers, empty and
fixed-length bodies, every header growth/descriptor allocation failure, 200 fields,
8 KiB cap, invalid fields/framing, duplicate lengths/encodings, HFS write/close/
rename/create failure, cancellation, timeout, cleanup error reporting and exact
updater download limits pass. All allocated header bytes are released. Upstream's
own eight test groups pass with assertions and sanitizers enabled; progress
width/throttle/unknown-total regression passes. Existing native network adapter
and command tests also pass. Target builds omit only upstream's unreachable
corrupt-internal-state assertion; malformed-input checks are unchanged.

Snow: Plus/68000/4 MB, existing MoofTCP+DaynaPORT System6.0.8 image (known greeting
reports6.0.7), fixed fast-forward selected, observed ~15–16.5x. Slow input injection
used normal speed to avoid synthesized key repeats; fixtures ran accelerated.
Real host HTTP through installed guest networking, no transport stub:
- Length and chunked 4096-byte binaries, three repeats and post-cancel recovery
  compare byte-for-byte with all 256 byte values repeated16 times. SHA256:
  c8f5d0341d54d951a71b136e6e2afcb14d11ed8489a7ae126a8fee0df6ecf193.
- Truncated, malformed, CL+TE and404 responses return1, leave no destination or
  .wget staging residue. Live chunked transfer cancelled with Command-period,
  status130 captured; subsequent transfer and comparison succeed.
- Production mktemp creates real HFS directories, `test -d` succeeds, repeated
  create/remove succeeds, missing volume returns1. Native heap fluctuates within
  a small range rather than declining progressively; largest block stays379442.
- The initial `/dev/full` error fixture was invalid: Clarice treated it as an
  ordinary path, returned0 and correctly retained the reported directory. It is
  NOT evidence of output-failure cleanup. Dedicated guest fault images test the
  real native create/remove path with only runtime stdout/yield injected; see
  fault-result and fault-hfs-listing: write2, cancel130, recovery0; only the four
  result/listing/name/output files remain, with no abandoned directories. The
  injected130 ends the script, so status/recovery were checked at the next prompt.

## Costs (comparative guest ticks, not physical seconds)

| Metric | Before | After |
|---|---:|---:|
| mktemp CLX bytes | 16396 | 18092 |
| mktemp loaded image | 10380 | 11264 |
| mktemp file total, final matched runs | 102 / 100 / 106 | 103 / 109 / 103 |
| mktemp file loader | 54 / 57 / 64 | 60 / 66 / 60 |
| mktemp directory total | unavailable | 94 / 88 / 94 |
| mktemp tracked peak/remaining | 5716 / 0 | 5716 / 0 |
| wget CLX bytes | 19328 | 23244 |
| wget loaded image | 14200 | 17464 |
| wget4096-byte length total | 140 / 181 / 173 | 168 / 178 / 176 |
| wget loader | 44 / 61 / 70 | 65 / 68 / 66 |

Full verification used for disposable CLX files. HTTP timings contain host-network
scheduling and accelerated guest-clock effects: no CPU speedup/regression claim
from these three samples. Loader/image growth is real. Final three chunked runs
return identical native free heap379486/largest379442; tracked count0 is not a
measurement of all native allocations. Final mktemp file heaps385256/385246/385258
and directory heaps385376/385364/385376, largest379442 throughout; no progressive
loss observed. Host ordinary three-line HTTP header peak608 B corresponds to
560 B with target descriptor size; worst-case header storage remains bounded by
8 KiB plus one descriptor per received line, with transient double-buffer growth.
No idle Core memory increase, whole-body buffering or additional bitmap/cache.

## Reproduction / limits

- `CLARICE_TEST_COMMANDS='mktemp dd' sh tests/ports/build-pipelines-host.sh`
- `python3 tests/commands/dd-mktemp.py`; `sh tests/http/run-host.sh`
- `CLARICE_BUILD_COMMANDS=mktemp sh scripts/build-pipelines.sh`
- `sh scripts/build-wget.sh`; `sh scripts/build-update.sh` (both normal builds pass).
- `tests/http/chunk-server.py --bind HOST_IP` serves only fixed disposable fixtures;
  guest scripts in tests/reuse-audit take HOST_IP as their first argument and record output/status/runtime. Optional matched
  baseline images are generated from the pre-change sources. Do not install the
  test-only mktemp-fault images as production commands.
- `python3 tests/reuse-audit/check-next-guest.py` verifies extracted guest evidence.

No physical acceptance yet. Requests retain HTTP/1.0 with Connection: close; HTTP/1.1 chunked responses are now understood.
No TLS, redirects, interim responses, EOF-delimited
bodies, content decompression or resume added. This is a parser/framing batch,
not full GNU wget. The rebuilt updater uses the same bounded downloader, but no
application replacement/relaunch cycle was performed in this batch. Existing
network cleanup may retain a driver-owned buffer if the driver refuses release;
that safety behavior was not weakened. No main/tag/release/push changes.
