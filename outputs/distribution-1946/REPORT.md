# Integrated development distribution 2026111946

Accepted glyph implementation2ad0967 and evidence678da84 fast-forwarded into
local main, without conflicts. Build/publication source checkpoint **d14852d**.
No branch/worktree deleted; 18 worktrees with local changes remain preserved.
No Git push, tag, public release or physical deployment was performed.

Clean normal `scripts/build.sh` passed: Core395008 bytes and49 standard commands,
35 distinct command images (including bundled updater) validated by the production
loader/import checker. Optional vi rebuilt sequentially after the full build and
is byte-identical to accepted1943 (SHA e85d23e7e7c1b7b6bf6927ee0c79be3ebd613638a87442ae49696130febe10c7).
The first overlapping optional build failed because both build paths recreate the
same temporary BusyBox source tree; nothing from that failed attempt was published.
Future build order is documented. No production code changed beyond build identity.
Actual `draw` 68000 disassembly is identical to accepted1945; retain its568 guest
pixel comparisons and normal Snow acceptance. No new1946 guest/hardware run claimed.

## Publication

- One canonical directory: `work/publication/http/full` in this main worktree.
- Official index: **http://192.168.1.112:8089/full/index.txt**.
- AFP: **Macintosh Exchange/Clarice/index.txt**; host mount is
  `/Volumes/Private/Macintosh Exchange/Clarice`.
- 51 entries /36 unique payloads: Core1946 +49 standard tools1946 +vi1943.
  Actual HTTP downloads and AFP readback match every sealed size/SHA-256/CRC.
- AFP is an exact verified mirror, not a separately generated repository. Old
  packages/indexes retained; payloads and immutable index snapshots written before
  active index replacement. Failed mirror retains the previous current index;
  multi-server activation and power-loss atomicity are not promised.
- Source-1946.tar.gz, seal-2026111946.json and Clarice-1946.tar available in both
  locations. Source archive includes licences and genuine upstream BusyBox tarball;
  its src/scripts/assets digest exactly matches the sealed build inputs.
- Per the user's follow-up, HTTP has **no client IP allowlist**, remains bound to
  192.168.1.112, and uses tracked scripts/dev-http.py. This host-only setting changed
  after the1946 seal: the archived d14852d source remains the build authority, and
  existing immutable artifacts/seal were not rewritten or re-sealed. Next product
  build must use the normal clean build workflow. Verification was repeated through
  the unrestricted listener. Old fixture/legacy endpoints retained pending consumer
  confirmation; only8089/full is official (see INVENTORY.md).

## Validation and limits

- 460 focused host editor presentation comparisons pass; accepted568 guest pixels
  remain unchanged. Production compiler flags -Os -m68000; no profiling definitions.
- Repository/catalog, full validation, recovery and import checks pass:86 installer
  cases,482 replacement transaction cases, both-fork binary/error preservation.
- Packaging10 cases;49-command manifest corruption coverage;42 collection failures
  and42 interruptions. See saved focused logs; these are host tests, not hardware.
- Added mirror tests: corrupt/missing payload, failed copy, failed index activation,
  retry/repetition, downgrade/same-version mutation rejection, recovery/unrelated
  file preservation. HTTP tests pass unrestricted mode, optional restriction and
  directory-listing rejection. Failed tests during development were corrected before
  activation; no failed packages exposed as current.
- Core remains MacBinary (both forks/Finder metadata); existing HFS installer and
  recovery semantics unchanged. No changes to installed utilities/preferences or
  recovery copies on physical Macs. Optional nano remains independently installed.
- SE/SE30/PB180 compatibility targets retained; **no new hardware acceptance**.
  Intermittent updater error28 and prior diagnostic -50/-36 are not claimed fixed.

Ready for manual upgrade via either canonical source. Stop here; Q1-001 not begun.
