# Valid resource file validation

Follow-up to the file-progress test's full-resource-fork comparison gap. The
original fixture contained arbitrary bytes, with no valid resource header/map.
Its differences at offsets 48–125 lie inside the documented system-reserved
area, offsets 16–127. Treating those bytes as application payload was an invalid
acceptance condition. This does not permit ignoring changes in resource contents,
resource maps, the header, or the application-data area at offsets 128–255.

## Result

Resolved on Snow/System 6.0.8 with unchanged baseline and progress-enabled utility
binaries. All 24 status checks and 12 native Resource Manager reads pass across
create, replacement, same-volume rename, cross-volume move and return copy. The
source is read again at the end and remains unchanged. Every resource's type,
ID, name, attributes, length and payload matches. Resource headers, maps,
application-data bytes and all other bytes outside the documented system-reserved
area match exactly. Data forks, type/creator and timestamps match too.

The checker rejects deliberate corruption outside the system-reserved area. This
is a format-aware preservation check, not a claim of whole-fork byte identity.
The raw hashes and every changed offset remain in [validation.json](validation.json).
See [guest log](guest.txt), [summary](summary.txt), [build](build.txt), and
[artifact identities](artifacts.json).

No production changes, extra runtime allocations, new startup work, or performance
cost are introduced by this follow-up. The prior copy/progress timing and memory
results are unchanged. Native compilation and Snow execution are verified;
physical SE and System 7/MultiFinder checks remain separate acceptance work.
The resource-file gap itself is closed; the recommended next step is review of
these staged validation tests/evidence, then the outstanding hardware checks.
No follow-up commit, merge, push or deployment was performed.

## References and method

- Apple, [Inside Macintosh Volume I](https://vintageapple.org/inside_o/pdf/Inside_Macintosh_Volume_I_1985.pdf),
  Resource Manager, resource-file format starting at I-128: 16-byte header,
  112 system-reserved bytes, 128 application-data bytes, followed by resource
  data and resource map.
- [CiderPress II resource format notes](https://ciderpress2.com/formatdoc/ResourceFork-notes.html),
  Macintosh format, independently describes the same boundaries and map layout.
- The fixture uses the existing pinned Retro68 toolchain's Rez compiler to
  construct the resource map. No production parser, copy routine or OS behavior
  is replaced. The offline checker follows the map format already parsed by
  `scripts/release.py`, adding names and attributes for this test.

The valid file has an 8476-byte data fork and 66232-byte resource fork. It contains
three named resources: STR /128 (33 bytes), BLOB/129 (65792 bytes), and TEST/-123
(7 bytes, purgeable). The large resource crosses both the 4 KiB copy buffer and
64 KiB boundary. The binary resource includes NUL and high-bit bytes.

The native guest probe uses OpenRFPerm, GetResource, HomeResFile, GetResInfo,
GetResAttrs and GetHandleSize. It reads each payload and verifies an independently
generated expected hash. HomeResFile prevents fallback to a resource from another
open file. The offline check parses every resource and compares SHA-256 payloads,
names, IDs, types and attributes; it also requires exact byte equality everywhere
outside offsets 16–127, equal data forks and preserved type/creator/timestamps.
Corruption at offsets 0, 128, 255, 256 and the last byte must be rejected.

Initial probe development used a later Resource Manager trap that produced an
“unimplemented trap” dialog on System 6. That failed run is excluded. The probe
was corrected to the older API above. A second probe assertion incorrectly
compared the whole short returned by GetResAttrs: the System 6/toolchain result
contained 0x3b00 for a zero-attribute resource. Resource attributes occupy eight
bits; the probe now compares the defined low byte and retains failure diagnostics.
Offline map attributes were already exact. Each rerun rebuilds disposable media
from the untouched baseline. No production utility was changed for either probe
issue.

## Reproduction

From Clarice-File-Progress, after the original file-progress build/media setup:

1. `sh tests/file-progress/resource/build.sh`
2. `python3 tests/file-progress/resource/prepare.py` (only with its disposable
   test disks closed; clones the previous tested media, never original disks).
3. Boot those two disks in Snow with the existing Plus ROM and 4 MB. Launch
   Clarice from GlyphFull; the test profile runs `guest.sh`. Wait for
   `RESOURCE-TESTS-DONE`, which follows explicit FlushVol calls on both volumes.
4. Close the test emulator, then run `python3 tests/file-progress/resource/extract.py`
   and `python3 tests/file-progress/resource/check.py`.

This run uses the existing Snow 1.5.0 core through the small test-only `snow.rs`
frontend, normal video pacing, no network, the stock Plus ROM and System 6.0.8.
The frontend exposes frame capture and mouse clicks without changing emulator
CPU, disk or Macintosh implementation. Build it as a snow_core example with
`cargo +1.95.0 build --offline --release -p snow_core --features mmap --example clarice_resource_check`
after copying it to that checkout's `core/examples/clarice_resource_check.rs`.
Pass the ROM, cloned system disk and cloned second disk as three arguments.
Its stdin `frame PATH` writes a PPM; `click X Y COUNT` sends guest mouse events;
`quit` stops the test frontend after guest FlushVol. This helper is external test
tooling, not shipped Clarice infrastructure.
