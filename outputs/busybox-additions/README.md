# BusyBox additions — isolated port evaluation

Branch `codex/busybox-additions-20261118`, based on `6a0546a`, in
`Clarice-Busybox-Additions`. Seven optional independent CLX packages from genuine
BusyBox 1.37.0; no standard-catalog, Core, Main-branch or physical-SE change.

## Scope and build

Ported diff, base64, xxd, patch, gzip, gunzip and tar. Upstream code, authorship
and GPL licence are retained; the source archive SHA256 is
`3311dff32e746499f4df0d5df04d7e108bb9250e7b519b837043a4`.
The adaptations and exact supported options are described in
[port notes](../../src/ports/busybox-additions/README.md) and each command's `--help`.

Build: `sh scripts/build-busybox-additions.sh`; validate:
`python3 tests/busybox-additions/packages.py`. The script pins Retro68's Docker
image, builds for 68000 and packages against existing runtime 1.10. No new loader,
resident service, fake Unix permissions or process implementation was introduced.
Existing file/stream, temporary-file, allocation and native safe-save services
sufficed. This avoids a new resident RAM/CPU cost; package image and invocation
allocations are incurred only when invoked.

| Package | CLX bytes | Loaded image bytes |
|---|---:|---:|
| diff | 30076 | 20720 |
| base64 | 22020 | 14608 |
| xxd | 31840 | 21984 |
| patch | 28196 | 18368 |
| gzip | 29348 | 20736 |
| gunzip | 26908 | 19232 |
| tar | 25080 | 17088 |

Combined package payload: 193,468 bytes. [Exact hashes and imports](packages.json).
Versions: diff/base64/gzip/gunzip 2026111891, patch/xxd 2026111892, tar 2026111893.
The test-only forksum diagnostic is not a proposed userland package.

## Validation and cost

68000 compilation and package/import/bounds validation passed for all seven.
Host ASan/UBSan option-parser regression passed. A narrow staged newlib parser
correction makes a lone `-` an operand, including stdin; the vendor source and
older standard BusyBox build remain unchanged. This was a parser defect, not Telnet.

The ports were installed through Clarice's actual update mechanism into a
separate Snow Macintosh Plus / System 6.0.8 / 4 MB fixture, Core 2026111882.
Guest tests use authenticated Telnet on isolated host port 23245. They exercise
help/version, options, exit status, streams, binary/high-bit data, round trips,
corrupt gzip CRC, concatenated gzip, truncated archive, patch mismatch and locked
files. [Basic results](guest.json): 50 passing observations at version 1891.
[Extended results](extended.json): 102 passing observations, including final
patch/xxd and tar 1892. Tar's subsequent extended-header rejection has a separate
final check. Tests are recorded at their actual package versions, not retroactively
claimed as all rerun on the last build.

Three runs per case; first timed run is recorded separately in [timings](timings.json).
These are warm-session complete-command timings, not cold-boot launches.
Snow ran with variable approximately 1.3–1.5× fast-forward. Host time includes
submission and prompt return; guest time is module ticks / 60. Neither establishes
stock 8 MHz SE acceptance. Inputs are bytes 0..255 repeated (highly compressible).

| Command/input | First host s | Median host s | Median guest s | Peak owned bytes |
|---|---:|---:|---:|---:|
| base64 8k | 2.07 | 2.07 | 1.87 | 5716 |
| xxd 8k | 10.08 | 10.22 | 13.40 | 7764 |
| gzip 8k | 2.44 | 2.28 | 2.17 | 242240 |
| gunzip 8k | 1.77 | 1.80 | 1.43 | 63200 |
| diff 8k | 1.82 | 1.82 | 1.65 | 6796 |
| base64 64k | 7.54 | 7.54 | 9.57 | 5716 |
| xxd 64k | 71.24 | 73.69 | 101.40 | 7764 |
| gzip 64k | 5.80 | 5.38 | 6.80 | 242240 |
| gunzip 64k | 3.17 | 3.17 | 3.93 | 57312 |
| diff 64k | 4.31 | 4.20 | 5.55 | 6796 |

All timed cases returned invocation allocations to zero. Peak owned bytes exclude
CLX image, stack and Core-owned safe-save allocations; they are not total peak RAM.
`diff` here compares equal binary files; changed-text costs are recorded separately.
`xxd -p` is notably slow: around 74 host seconds for 64 KiB. Keep existing `od`
for routine inspection; xxd's present value is reverse conversion/C-array output.
No speed-win claim against an existing utility is made from these measurements.

## Explicit compatibility limits and next decisions

- diff is file-only, byte-oriented; stdin is spooled to an existing temporary-file
  service because upstream seeks. No directory recursion or Unicode case folding.
- patch handles one explicit existing text file, with native safe replacement.
  No multi-file patching, create/delete or missing-final-newline support. Dry-run
  requires writable staging storage. Scanned lines containing NUL are rejected.
- gzip/gunzip are stream tools: `-c FILE` or stdin/stdout, plus gunzip `-t`.
  They never delete inputs or automatically replace HFS filenames. gzip's peak
  owned allocation is about 237 KiB and is returned after invocation.
- tar is an initial archive reader: listing and stdout extraction of uncompressed
  POSIX archives only. No creation or filesystem extraction yet. GNU/PAX extended
  names are rejected. It is data-fork interchange, not a Macintosh backup tool.
- base64 and reverse xxd retain upstream permissive decoding; not input validators.

Keep these optional pending review. Recommended next work is focused xxd profiling
and a separately reviewed safe tar filesystem-extraction design; neither a broad
runtime redesign nor automatic promotion is justified by this port evaluation.
Physical SE performance and MultiFinder/System 7 execution remain unmeasured.

## Resource-fork diagnostic

An initial raw-fork FNV assertion failed despite identical lengths. Ordinary
`cp` reproduced the difference, ruling out a patch-specific conclusion. Apple's
[Inside Macintosh: Creating an Empty Resource Fork](https://leopard-adc.pepas.com/documentation/mac/MoreToolbox/MoreToolbox-36.html)
explains that File Manager writes scavenging information into the resource header,
so copies need not be byte-identical. The
[resource-fork layout](https://formats.kaitai.io/resource_fork/) separates the
16-byte header, 112 system-reserved bytes and 128 application bytes.

The corrected test compares **every byte outside offsets 16–127**, including the
entire application area, data and resource map. A 279414-byte resource fork after
patching had 38 differences confined to that system-reserved area and **zero
outside it**. The fingerprint normalizes only those 112 bytes. The initial raw
failure is preserved in `safety-initial-raw-header.json`; this is an explained
test correction, not a Core change or an ignored payload mismatch.

## Telnet observation

One immediate reconnect after the previous test client closed ended with EOF
before the password prompt. A later connection authenticated successfully. This
is recorded as a reconnect/rearm observation for the ongoing Telnet work, not
an established cause; active command sessions otherwise completed. No Telnet
code was changed here. The lone-dash operand problem was independently reproduced
and fixed in the staged option parser, not attributed to networking.

[Final safe-save checks](safety.json) passed: forward/reverse patch, preserved
TEXT/TEST Finder codes and resource payload, Ctrl-C status 130, unchanged data
and resources after cancellation, zero remaining invocation allocations, and a
successful subsequent save. The cancellation target had a 256 KiB data fork
and 279414-byte resource fork. This tests the real native transaction boundary
in Snow; it does not claim exhaustive disk-full/power-loss or physical validation.

## Reproducing the private fixture

The test scripts intentionally target the isolated port 23245 and a local private
HTTP repository on 8099; adapt these and the password-file location for a new
fixture. They are not public distribution tooling. Build the seven packages,
run `packages.py`, build the test-only diagnostic with `build-fixture.sh`, then
stage the private repository with `serve.py`. Its baseline payloads come from the
validated BusyBox-Replacements fixture. Run Clarice update locally (the updater's
existing remote restriction remains intact), then run the guest, extended,
safety, diff-cost and final scripts sequentially. Allow the Telnet listener to
rearm between connections. `final.py` checks installed version/length/CRC records
against the private repository, including unchanged baseline tools.

These tests use disposable `/tmp/ba-*` and `/tmp/add-*` guest files. Keep the
fixture isolated; repeated updater tests preserve old CLX backups rather than
removing fallback packages. No installed SE package or other agent's Snow disk
was used.

## Changed-text operation costs

Three runs on 8 KiB / 256-line input with one edited line:

| Operation | First host s | Median host s | Median guest s | Peak owned bytes |
|---|---:|---:|---:|---:|
| diff | 5.85 | 5.85 | 7.32 | 17500 |
| patch | 2.97 | 2.93 | 3.40 | 11880 |

Patch results were compared after each forward and reverse application. These
cases have ordinary empty resource forks; the resource-bearing safety tests
exercise a different, more expensive copy workload. [Raw results](diff-cost.json).

The 256 KiB changed-text diff exceeded available memory: status **2**, explicit
`out of memory`, 91.65 host seconds / 129.85 guest seconds, **290944 bytes** peak
owned and **0 remaining**. A subsequent 8 KiB comparison succeeded. This is a
real workload limitation, not a successful large-file comparison; upstream diff
keeps line/index structures in memory and this port adds no disk spill.
NUL-bearing target and malformed-hunk rejects preserved the original. Guest gzip
output also decoded byte-for-byte with Python's host gzip implementation. The
[changed-text suite](diff-cost.json) contains 38 passing command observations,
including expected failure statuses.

Final tar 1893 checks pass: ordinary USTAR list/stdout extraction round trip,
GNU/PAX extended-header rejection and verbose-extraction rejection. All **60**
latest installed version/length/CRC records match the private repository
(Core, 50 baseline tools, existing optional telnetd, seven additions and
test-only forksum). See
[final results](final.json) and [catalog verification](installed-catalog-check.json).

Cleanup: guest remote access disabled, Clarice quit to Finder, isolated Snow
workspace closed and private HTTP server stopped. Test disks and fallback CLX
files are retained for reproducibility. No push or release was performed.
