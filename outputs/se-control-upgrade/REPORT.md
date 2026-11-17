# SEControl burst input and PowerBook preparation

16 November 2026. Development utility only; Clarice production source unchanged.

## Changes

- Optional `control.py type TEXT --fast`: eight printable ASCII events per
  roundtrip, versus one per character. Removes per-character host delay and
  receives a complete burst in one native network read. Return remains separate.
- Native event-queue backpressure: BUSY posts nothing; malformed batches are
  rejected before posting. A native posting failure reports the exact partial
  count. No ambiguous input is replayed, no retained keyboard queue or OS patch.
- Main-display dimensions and mouse bounds are no longer fixed to 512×342.
  Temporary monochrome snapshots use direct mono copying or native QuickDraw
  conversion with a full-screen clip. Host validates dimensions/stride/length,
  strips padding for PBM/PNG, and never saves incomplete frames.
- Helper partition 224→256 KiB; MacBinary 96,640→97,920 bytes (+1,280).
  No permanent bitmap. Snapshot sizes: SE21,888; PB18032,000; 640×48038,400 B.
  Existing 16 KiB MacTCP receive ring and314 B mouse plan unchanged.

## Actual validation

- 16 host tests pass: framing, disconnect/reconnect, partial input, backpressure,
  modifiers, invalid input, padded 513/639/640-pixel captures, mouse bounds.
- Snow Plus/4 MB: eight characters `Abc123!?` delivered in order with correct
  modifiers. Immediately following burst returned BUSY. Invalid Return and
  malformed hex rejected. Capture succeeded; bounded cancellation and host-loss
  mouse release passed, button128/down0/error0 at exit. See snow-burst.txt/png.
- Snow Mac II/4 MB: complete640×480 capture succeeded, including outside the
  helper window; remote movement to620,380 succeeded and clean exit followed.
  See snow-colour.txt/png. This exercises Color QuickDraw's PixMap path, not
  a PowerBook LCD or verified System7.1 installation. Capture buffer is released;
  native free heap grew as the Memory Manager expanded the heap, so before/after
  FreeMem alone is not a retained-allocation measurement.
- Snow foreground regression: existing fixture Clarice opened, received `pwd`
  through KEYS3, printed `/`, closed and reopened. Background capture, menu,
  cancellation and mouse release passed; free heap24992 stable at all five
  recorded post-capture checkpoints. See snow-foreground.txt/png. This fixture
  uses an older Clarice, separately from physical Core1550 acceptance.
- Physical SE/System6.0.8: separate SEControl Fast deployed remotely; original
  helper retained, old running instance quit cleanly after new connection worked.
  Matched 67-character line: legacy19.545s (3.43chars/s), burst4.062s
  (16.49chars/s), **4.8× faster**. Exact output verified against host POSIX cksum
  2262502815/48bytes. Longer130-character command and foreground-command
  typeahead also worked. This measures remote injection, not physical typing.
- Physical background captures, Finder clicks/drags, selection, cancellation,
  app closing/reopening and installer operation succeeded. Free heap27068 steady,
  5172 with21,888-byte snapshot. Capture CPU cost7–15ticks is worse than old
  helper1–3ticks; no capture-speed improvement claimed. Host capture1.3–2.5s.
  Fresh forced network-disconnect acceptance was not repeated on hardware;
  existing host/Snow disconnect/release evidence remains separate.

## Prepared private test distribution

`work/powerbook-180-kit/Clarice-PowerBook-Test.hda`: existing accepted1550 Core,
Install Clarice launcher,43 utilities, updated native SEControl, helper source
and licences. Application-only HFS disk; contains no operating system or ROM.
It includes the private local pairing configuration and must not be published.
Standalone MacBinary Core/helper and checksums are in the same directory.

Open Install Clarice → Install. Existing utilities/settings are preserved;
Core does not scan/install utilities at ordinary startup. Copy Remote Control
beside its config to an ordinary folder; keep the old helper recoverable.
PowerBook requires its own working MacTCP/network interface and a distinct IP.
The host must explicitly allow that IP (or use a separate listener/port/socket).
No implicit sharing of the SE's network identity.

## Pending

System7.1/PowerBook180 boot, greyscale LCD capture, keyboard/trackball handback,
MacTCP throughput, sustained burst input and disconnect testing. No usable7.1
fixture was found; do not label System6 Snow evidence as7.1 acceptance.
The new helper runs beside the recoverable original; physical System7.1 acceptance is deferred until the PowerBook arrives.
No release, main merge or additional push. Accepted checkpoint remains intact.
