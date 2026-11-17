# Core application and distribution investigation — 1549

Recommendation: **small native core + external standard utility kit + optional
applications/docs; explicit installation and repair, no per-launch utility scan**.
The gain in normal startup comes from removing work, not from a smaller file.
This is a review result, not an implemented distribution change. Production source
and accepted MacBinary/checkpoint e67c569 remain unchanged. No push or release.

## What is actually packaged

Measured accepted MacBinary, uncompressed logical sizes:

| Component | Bytes | KiB |
|---|---:|---:|
| MacBinary package | 1,045,632 | 1,021.12 |
| Application data fork | 0 | 0 |
| Application resource fork | 1,045,457 | 1,020.95 |
| 28 CLX1 resource payloads | 780,568 | 762.27 |
| CODE 0 + CODE 1 | 262,842 | 256.68 |
| All other resource payloads | 977 | 0.95 |
| Resource headers/map/padding | 1,070 | 1.04 |
| Installed 43 command data forks | 960,440 | 937.93 |

MacBinary adds 128 header bytes and 47 padding bytes. Removing CLX resources alone
projects a 264,433-byte resource fork / **264,576-byte (258.38 KiB) MacBinary**,
about 75% smaller, before any installer-code changes. HFS allocation overhead and
optional apps are excluded. Installed command bytes exceed bundled bytes because
multicall images are copied under separate command names; no hard links are used.

Exact per-image sizes, attributes, hashes and command mapping are in
[census1549.json](census1549.json). Embedded images are:

- core: ls, df, cat, benchls, benchscan;
- files: cp, mv, touch, rmdir, mkdir, rm, ln, readlink, hfsattr;
- inspect: find, du, file; misc: od, more;
- separate images: grep, sed, awk, serial, wc, head, cut, tr, uniq, tee, tail,
  sort, basename, dirname, kermit, wget, clarice, cmp, cksum, free, sleep,
  printenv, mktemp, dd.

Every standard installed command currently has a bundled source. Nano and the
ncurses demonstration are external-only optional/development CLX files, not
resources. Nano statically links ncurses and an embedded terminal description;
no separate disk terminfo database is required. Shared host libc/runtime services
are part of native core code, not another startup-installed library file.
`clarice.clx` also contains the 64,000-byte MacBinary updater helper plus its
network/update code; that helper is not a separate core resource.

## Loaded code versus storage

The build uses Retro68 `--mac-single`: CODE 0 is 24 bytes; CODE 1 is 262,818 bytes.
The native shell, terminal, builtins, runtime, installation helpers and builtin
help reside together. Linker accounting is text 219,736 + data 33,580 + BSS 41,276
=294,592 bytes, distinct from on-disk CODE size and total application heap use.
The requested preferred/minimum partition remains 768 KiB, with 64 KiB additional
stack reserve. Externalising data resources does not itself lower that request.

Healthy startup requests native CODE and system font/window/control facilities.
Menus are created in code. Preferences DLOG/DITL and error ALRT/DITL are requested
on demand. Icons/BNDL/FREF serve Finder; CLUP serves update inspection. No CLX1
resource has the preload attribute. Get1Resource/HLock is used only for a missing
command or explicit bundled repair; ReleaseResource follows. All ten measured
launches ended with **zero loaded CLX resource payloads**.

## Startup measurements

Disposable copied Snow Plus/4 MB, reported **System 6.0.7**, fixed maximum FFWD
(~15–16x). Guest ticks, not host wall seconds or physical-SE estimates. An isolated
source copy adds counters and a post-draw report; ordinary production files and
build inputs were not modified. Native CODE and every non-CLX resource are
byte-identical between the full/core diagnostic variants.

| Diagnostic condition | Samples | C-entry to ready | First draw complete | Install pass |
|---|---:|---:|---:|---:|
| Full app, all commands installed | 3 | 368 | 369–370 | 112 |
| Full app, installation scan bypassed | 3 | 257 | 258–259 | 0 |
| Core-only, scan bypassed | 3 | 262 cold, 258 repeated | 264 cold, 260 repeated | 0 |

Bypassing the scan saves **111 ticks (~30%)**, with no heap change. Merely removing
resources adds no measurable speed benefit. The 43 startup checks are native
catalog existence lookups: existing files are **not** read, checksummed, relabelled,
copied or repaired. Missing/corrupt contents are handled when invoked or explicitly
checked; even damaged existing files are deliberately not overwritten at startup.

Before/after normal scan free heap: 404,594 ->404,594. Core-only adds **344 bytes**
there, exactly the removed resource-map entries. At ready: 404,620 versus 404,954;
largest free block 401,014 versus 401,032. Resource census costs zero additional
heap in these runs. These are application heap figures, not machine-wide free RAM.
The probe adds about 905 CODE bytes and a common index-file read. Times begin at
C main, excluding Finder/Segment Loader work before it; the report write and resource
census occur after timing. They do not establish complete Finder-to-prompt latency.

Fresh diagnostic installation attempted 43 commands: 7,464 install ticks, 7,728
ready ticks, with **awk failing allocation** (42 installed). The accepted unmodified
1549 build reproduced that failure on an empty bin. A later launch with only awk
missing succeeded; explicit `appinstall replace /mnt/Audit/tools/awk awk` also
succeeded without errors. Recovery yielded all 43 names and working `/bin/awk
--help`. Thus this is peak/contiguity pressure during full bootstrap, not an
inability to execute awk. The bundle is locked while validation allocates another
image and relocation workspace: awk's 188,956-byte resource plus 137,332-byte
image and 51,592-byte metadata coexist. Removing that duplicate live resource is
worthwhile; exact peak heap fragmentation was not instrumented.

## Reuse and changes needed

| Operation | Reuse | Needed for the recommended arrangement |
|---|---|---|
| Fresh install | Namespace setup, CLX validator, staged command transaction | Explicit installer/script invoking an install-if-absent file-source operation; stream files rather than locking whole bundled resources; keep work out of ordinary startup |
| Independent update | appinstall replace FILE NAME, compatibility checks, receipts, .old backup | No new package format; retain explicit replacement and never infer permission to downgrade |
| Repair | Existing transaction and full verification | Select a repair file/kit source instead of Get1Resource; keep kit on distribution media rather than another permanent duplicate cache |
| Recovery | appinstall recover, same-directory rename/flush, .new/.old refusal | Retain existing rules and recoverable originals |
| Core self-update | Existing MacBinary HTTP staging, both-fork validation, helper handoff/relaunch and backup | Point the existing mechanism at the small core package; do not silently update the utility kit |
| Optional nano/docs | Existing PATH, HFS folders and file copying | Separate optional nano/source/licence package; preformatted manuals/examples in /usr/share/man/cat1 and /usr/share/clarice/examples; no man implementation in this task |

A standard installation can remain an ordinary HFS distribution disk/folder with
Clarice, independent CLX files and a small explicit setup front-end. No package
manager, new archive format, loader redesign or permanent cache is needed. The
current appinstall API already installs a supplied file when absent via replace,
but an installer must call its **install-if-absent** primitive without weakening
no-overwrite semantics; exposing that operation needs a small interface change.
Optional nano can use the existing /bin destination and be omitted from the base
kit. Existing /etc/clarice preferences/history and user profile stay untouched.

Command replacement preserves previous files with both forks in recovery; the new
CLX file gets existing Finder info but does not inherit the old resource fork.
That is suitable for current data-only commands, not a general Macintosh-application
installer. Core self-update already preserves full application forks, placement
and creation date. Keep these contracts distinct. Core updates would transfer and
verify roughly 75% fewer bytes, but actual updater timing was not measured here.
Startup installation currently calls SystemTask rather than interactive cancellation;
an explicit installer should provide cooperative progress/cancellation separately.

## Evidence and boundaries

[guest.txt](guest.txt), [variants.json](variants.json),
[production-check.json](production-check.json); tests/packaging/census.py,
prepare-probe.py and validate.py. The latter validates saved results, not rerunning
Snow. Probe reproduction: clone disposable fixture; use an empty bin once, install
awk externally after the observed failure; launches 1–3 scan, 4–6 bypass; rebuild
only the cloned resource list with Empty.r and run 7–9. No source change is retained
in src/. Normal-size census was checked against the accepted build hash. Existing
utility help executed from the core-only app; normal recovered /bin contains all
43 names. No new recovery fault-injection suite, self-update run or physical-SE
acceptance is claimed; those mechanisms were reviewed and their accepted tests
were not unnecessarily repeated. Await review before changing production packaging
or the automatic missing-command installation behaviour.
