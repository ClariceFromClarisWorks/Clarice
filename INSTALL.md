# Canonical development distribution

Official private-LAN update index:
`http://192.168.1.112:8089/full/index.txt`.
The same verified manifest and immutable payloads are mirrored to
`Macintosh Exchange/Clarice/index.txt`; the share is mounted through Chooser.

```sh
clarice update --check --source http://192.168.1.112:8089/full/index.txt
clarice update -y --source http://192.168.1.112:8089/full/index.txt
# Or, with the share mounted:
clarice update -y --source "/mnt/Macintosh Exchange/Clarice/index.txt"
```

The configured default is the single URL or mounted path in
`/etc/clarice/update.conf`. HTTP-only old Core versions must use HTTP first.
Core remains a MacBinary package preserving forks/Finder metadata; tools are
CLX payloads installed by appinstall. Recovery copies and unrelated files remain.
The 68000 build targets SE, SE/30 and PowerBook 180 with supported System 6/7
configurations and enough free application memory. Compatibility is not fresh
hardware acceptance. No TCP/IP stack is required for mounted-share updates.
The internal HTTP listener has no client IP restriction, so changing Macintosh
addresses are supported. It remains bound to the MacBook's LAN address. Optional
`--peer` restrictions are available but are not enabled for this distribution.

Maintainer workflow (one source, no independent AFP builds):

1. Integrate accepted source and tests; allocate a new build ID for changed bytes.
2. Run `sh scripts/build.sh` (fresh output directory, complete inventory, seal).
   Run optional vi builds sequentially after the full build: both build paths
   prepare the same temporary BusyBox source tree.
3. Publish into a fresh staging directory with `scripts/repository.py`.
   Add accepted optional vi using `tests/busybox/publish.py` only after validating
   its CLX against the current imports; retain its version if bytes are unchanged.
4. Use `scripts/repository.py CANONICAL --mirror-from STAGE`. This validates
   length/CRC/content-name hashes, copies immutable payloads and versioned index,
   then atomically replaces the active index. Existing versions cannot mutate.
5. Mirror CANONICAL to the mounted `Macintosh Exchange/Clarice` using the same
   command. No rebuilding, repacking or editing the AFP index independently.
6. Verify every HTTP/AFP payload against the sealed build and optional-app hash.
   If AFP is unavailable or fails, report it as pending; do not claim parity.

The current host canonical directory is
`Clarice-Main-SelfUpdate-Merge/work/publication/http/full`; serve its parent
with `scripts/dev-http.py` on the existing port 8089. Other worktree repositories
are historical/test fixtures, not alternative official update channels. Do not
remove them or their listeners until their test/legacy consumers are checked.
Historical payloads and `idx-*.txt` manifests are retained for recovery. The
active index is a single-file activation, not a cross-server or power-loss
transaction; a failed mirror is retried from the same canonical distribution.

## Mounted update repositories (development build 2026111940)

The updater is bundled with Core. Its help must list `PATH_OR_HTTP_URL`.
An older HTTP-only built-in updater cannot gain share support by installing an
external `/bin/clarice`; first upgrade Core through a private HTTP source.
With the share mounted through Chooser, `/etc/clarice/update.conf` can contain:

```text
/mnt/Macintosh Exchange/Clarice/index.txt
```

Then run `clarice update --check` or `clarice update -y`. A one-time override is
`clarice update -y --source "/mnt/Macintosh Exchange/Clarice/index.txt"`.
Mounting and authentication remain Macintosh OS responsibilities. Packages are
copied into owned temporary files, validated and installed through the existing
recovery mechanism. Interrupted updates retain completed packages; repeat the
command to resume. Do not delete recovery copies to force an update.

# Installing Clarice Core and standard utilities

The core application contains the terminal, shell, builtins, runtime and self-update
capability. Standard utilities are separate CLX files. Nano remains optional. Use the complete offline
distribution for the first installation; repository updates subsequently maintain
the standard tools and update Core last.

**Install Clarice** opens a native welcome window before creating any installation
files. Cancel leaves the installation unchanged. Core and Data buttons choose folders before confirmation (recommended:
Core at the startup disk root; Clarice Data inside the System Folder). Both must
be on the same HFS volume. Existing installation locations are retained. Install
then shows the current operation and completed installation steps. Cancel or
Command-period during installation stops safely at the existing backend's polling
points; completed utilities are retained. Run the same installer again to resume.
No shell prompt, profile execution or terminal setup is needed for confirmation.
Installer B is tested in System 6 Snow; System 7 testing is deferred.

## Private repository updates

`clarice update` is built into Core. It does not require an installed
`/bin/clarice`, external wget or separately downloaded updater. The native helper
that replaces the running application is supplied automatically. Existing old
updater files are preserved but do not override the builtin command; old repository
entries for an external `clarice` command are ignored.

Configure `/etc/clarice/update.conf` with one private HTTP index URL, for example
`http://192.168.1.112:8089/full/index.txt`, then run:

```
clarice update --check
clarice update
clarice update -y
```

`--check` lists proposed changes without installing or updating local records.
The normal command asks `[Y/n]`; `-y` accepts that proposal. `--source URL` overrides
the configured source for this invocation. Newly listed standard tools are added,
older installed versions are upgraded, and known newer versions are not downgraded.
Repository-listed names are managed tools: on first adoption an existing different
file is proposed for replacement. Unlisted commands, optional nano, preferences
and user files are left alone.

Each successful tool is recorded in `/etc/clarice/installed.catalog`, a small text
catalog. `/etc/clarice/update.incomplete` means the update has not completed.
After a timeout, cancellation or failure, fix the reported problem and repeat the
same command/source. Completed tools remain installed and are skipped; the updater
does not roll back the collection. A partial final catalog append is discarded on
retry. Malformed complete records are reported, not silently accepted.

Tools use the existing fully validating `appinstall` implementation. Downloads
are length/CRC checked before installation. Core is a fork-preserving MacBinary
package; its existing helper replaces it only after Clarice exits, retaining the
previous application and automatically relaunching. The new Core records completion
and clears the marker after relaunch. Ordinary startup checks only for this marker;
it does not scan or hash `/bin`.

Build 2026111948 keeps one managed previous installation in
`/etc/clarice/Recovery Previous`: Core, catalog, managed utilities and an independent
**Recover Clarice** application. The older snapshot remains until its replacement
has been copied and verified. Extra space is temporarily needed during rotation.
Repository `.repo-NAME.old` files are transaction slots, removed only after a
verified snapshot and successful completion. Manual `appinstall` `.clx-*` backups,
legacy `.previous-*` archives and historical application copies are not pruned.
This is resumable HFS replacement, not an atomic collection or power-loss guarantee.
A corrupt catalog, damaged filesystem or unrecoverable disk error can require manual
repair. Explicit `appinstall check` remains the full executable diagnostic. The
catalog is a version record, not continuous detection of same-size file corruption.
Private HTTP CRC checks detect accidental corruption; they do not authenticate the
server or packages. Public Internet/HTTPS updates remain outside this milestone.

### Hosting the repository

From a complete build made by `scripts/build.sh`:

```
python3 scripts/repository.py work/private-repository
python3 scripts/dev-http.py --bind HOST_IP --port 8089 --directory work/private-repository
```

The publisher verifies the build seal, emits content-named CLX/MacBinary files,
then publishes `index.txt` only after all payloads exist. It rejects changing an
already published package/version. Increment the build identity for changed source.
No hand-maintained command manifest or separate installer is required.

Pre-1.0 development builds do not have a supported legacy upgrade path. Replace
the Clarice application directly if an old updater cannot read the repository.
Current builds include their updater; no separate updater installation is needed.

Future ABI-breaking upgrades may require a Core bootstrap: tools are validated
against the running runtime before Core replacement. This change does not bypass
those checks or introduce a general dependency resolver.

## Networking and optional applications

Networking clients and diagnostics are **Standard Unix Utilities**, installed as
external commands in Clarice Data (`/bin`), not part of Clarice Core. This includes
`ipconfig`/network status, `ping`, hostname-aware `wget`, `ftp` and `telnet`.
The 0.5.0 candidate contains 47 standard commands. This is a development candidate:
consolidated physical-SE acceptance is pending; no release has been published.

Core works without an installed or configured networking stack. Network commands
use the available native Macintosh stack and report unavailable services; no
specific TCP/IP stack, BlueSCSI hardware, DaynaPORT or MoofPort is required by
Core. Shared transport and DNS support is loaded on demand with the commands,
without networking startup work or idle-memory overhead in Core.

Use the same offline installer, `appinstall`, external repair sources and recovery
rules below for these utilities. Nano and future editors remain optional,
independently installed applications. A future Telnet server would be an optional,
disabled-by-default service, not a permanently resident Core component.

## Offline installation — no commands or file copying required

1. Open the **Clarice** distribution disk in Finder.
2. Double-click **Install Clarice**. Keep the recommended folders or choose
   **Core...** and **Data...**, then click **Install/Upgrade**.
3. Wait for the file names and checking messages. Clarice opens automatically
   when installation finishes. Allow several minutes; do not reset the Mac.

The recommended destinations are **Clarice** at the **startup-volume root** and
**Clarice Data** in the **System Folder**. Folder selection uses the native Save
dialog; the application/data names remain fixed and nothing is created before
Install. Core and Data must share one HFS volume. The System Folder file
**Clarice Locations** records the selected native paths. Without it, legacy
System Folder discovery applies. Do not move or rename installed locations or
edit this checksummed record; relocation is not supported in 0.5.0.
You can eject the distribution disk afterwards; keep it for repair.

No network, command-line knowledge, archive extractor or preinstalled utility
is needed. The installer uses the Utilities folder beside it, so it also works
if the disk label changes. Keep the disk's files together.

Upgrade verifies the kit and installed catalog. It replaces unchanged older managed
tools, adopts exact matching legacy tools, and preserves verified newer compatible
tools. Changed or unmanaged conflicting tools stop ordinary upgrade without
overwriting them. Unknown files, preferences and user data stay in place.
The installer does not run your startup profile; normal Clarice runs it after launch.

For missing or changed standard tools, select **Repair missing or changed standard
tools** using media matching the installed Core version, then upgrade separately.
Repair preserves the verified previous snapshot and retains replaced originals
as `.clx-NAME.old` (including both forks and Finder metadata). It does not silently
remove an existing manual backup; an unrelated backup conflict requires inspection.
For older media without this Repair control, use its existing `appinstall repair`
procedure first. Repair and upgrade are deliberately separate operations.

For an upgrade, quit installed Clarice first and run Install Clarice again.
The managed previous snapshot preserves both forks, Finder metadata and creation
dates. Migration retains the original application until the root-level replacement
has relaunched and completion has been verified. Preferences and user files stay
in place. An unrelated root-level application or mismatched catalog stops migration;
it is not overwritten or silently adopted. Equal/newer Core builds are not downgraded.

To roll back if Core cannot launch, quit Clarice and open **System Folder →
Clarice Data → etc → clarice → Recovery Previous → Recover Clarice** in Finder.
At custom destinations, start from the selected Clarice Data folder instead.
Confirm Restore. This restores the managed files and catalog, then launches the
recorded Core destination. New V3 snapshots record that destination; V1/V2 snapshots
retain their original startup-volume-root restoration behaviour. It does not require a working Clarice application or networking.
Do not rename the recovery directory or remove its inventory. Damaged snapshots
are rejected; keep offline installation media as a separate repair source.

Older Core versions execute their older updater when installing this candidate.
That bootstrap handoff retains its legacy recovery files. The new recovery policy
applies to subsequent updates initiated by the new Core, or its offline installer.

### Cancel and resume

Click Cancel before starting, or press **Control-C** during installation. Completed
utilities remain installed. Open **Install Clarice** again with the same kit and selected operation to
resume; verified completed files are kept. The current Core is kept until its replacement
is fully copied, closed and checked. Ordinary cancellation needs no manual copying.

After an abrupt reset or disk error, retained transaction files may need explicit
recovery (below). The collection is resumable, not an atomic transaction; there is
no guarantee against power loss.

### Advanced/manual installation

The existing command remains available for another installation source:

```
appinstall setup /mnt/Clarice/Utilities
```

Quote paths containing spaces. This manual setup installs missing tools only;
use `clarice update` for repository upgrades. After installation, try `ls /bin`, `awk 'BEGIN {print "ready"}'`
and `help`. Ordinary startup never repeats installation.

## Individual installation, checking and repair

```
appinstall install /mnt/Clarice/Utilities/grep grep
appinstall check /bin/grep
appinstall repair grep /mnt/Clarice/Utilities
appinstall replace /mnt/MyUpdates/grep grep
appinstall recover grep
```

`install` never replaces anything. `repair` and `replace` explicitly authorise
replacement; they do not compare version numbers. Check your chosen source before
replacing an independently updated command. The previous file is retained as
`/bin/.clx-grep.old`, with both forks intact. Its Finder information is copied to
the new CLX data file; its resource fork remains in the backup. Recovery restores
the old file and retains the displaced candidate as `.clx-grep.new`.

Existing `.old` or `.new` files block replacement. Inspect and move recovery files
to a safe folder before another replacement; never delete the only working copy.
A recovery copy is restored as-is, including any pre-existing damage.

## Failed or interrupted setup

Ctrl+C cancels between safe steps. Already installed commands remain usable.
Fix the reported source, disk-space or memory error and rerun the same setup
command: existing files are skipped. A failed staged write is cleaned up where
possible. An abrupt interruption can leave `.clx-NAME.new` or `.old`; inspect these
before retrying, using `recover` when a previous version is retained. The entire
collection is not one atomic transaction. No power-loss atomicity is promised.

User files, configuration and unrelated commands are not part of setup. A missing
command produces an installation hint. Damaged commands fail on execution or
`appinstall check`; startup intentionally does not scan the collection.

## Building the offline disk

`scripts/build.sh` builds the core and utilities and stages `work/distribution/kit`.
`scripts/distribution-disk.sh` makes a clean native HFS application disk containing the native
application, Install Clarice launcher, Utilities, installation instructions and
third-party notices. The launcher is a separate build of the existing shell and
native services. This offline launcher remains optional; ordinary Core contains
its own on-demand online updater. The disk
contains no ROM or System software. It retains only the existing template’s
Apple Driver43 device header/driver; provenance is recorded separately. Supply
a compatible template path as the script’s first argument if necessary. The MacBinary core preserves both forks;
CLX utility files are ordinary data files. Copying the native app through a
non-Macintosh filesystem requires its MacBinary container or an HFS image.

## BusyBox command replacements

Private build 2026111882 selects genuine BusyBox 1.37.0 for `wc`, `tr` and
`sort`. The ordinary updater replaces their existing packages. The standard
kit also installs `wc-sbase`, `tr-sbase` and `sort-sbase` as explicit compatibility
commands; each remains independently packaged.

`wc -m` now counts bytes, and `tr` translates bytes with ASCII character classes,
not Unicode characters. These tools do not convert MacRoman to UTF-8. Use the
sbase alternatives when their previous UTF-8 behavior is required. BusyBox tr
also lacks `[:graph:]`, `[:print:]` and `[x*N]` repetition syntax.

BusyBox sort holds input in available application RAM and releases it after
success, cancellation or failure. Use `sort-sbase` for disk-spilling larger jobs
or previous options such as `-o`, `-m` and `-C`. BusyBox sort rejects
`-o/-M/-V/-m/-S/-T`; redirect output to a separate file. A failed or cancelled
redirected command may leave partial output; never redirect over its input.

BusyBox vi remains independently installable, not part of the standard utility set.

## Optional BusyBox vi

The upstream BusyBox1.37.0 editor is an independent CLX runtime1.9 application,
not part of Core. Build with `scripts/build-busybox-vi.sh`; install
`work/busybox/vi.clx` through the existing repository as `vi`, or use
`appinstall install /mnt/DISK/vi.clx vi` for an absent command. Replacing an
existing vi is an explicit update/replacement and retains its recovery copy.
See [installation and tested limitations](outputs/busybox-completion/README.md).

Use `vi FILE`, `i`, Escape (or Ctrl-[), `:wq`; `:q!` deliberately discards edits. Abnormal
invocation exit attempts to preserve unsaved text in `/etc/clarice/vi-*.recover`.
Inspect `.part` files as incomplete. Recovery needs writable disk space and is
not protection from OS crashes or power failure. Normal saves continue to use
Clarice's fork-preserving safe replacement.

## HFS attribute command rename (build 2026111949)

Use `hfsattrib FILE` to inspect metadata, or
`hfsattrib ./thing.sit SIT!:SITx` to set its four-byte, case-sensitive type and
creator together. Existing `--type`, `--creator`, `--locked` and `--invisible`
options remain available. Malformed shorthand is rejected before any writes.

The standard collection now contains `hfsattrib`, without an `hfsattr` alias.
Successful upgrade completion removes the old executable only when its identity,
forks and metadata still match the managed previous-release snapshot. The offline
installer also recognises the exact1948 bundled image, whose tools were not
individually catalogued. Unknown/manual legacy copies are preserved and reported;
modified managed copies block cleanup rather than being deleted. The previous
snapshot retains the old command for rollback. Historical catalog rows are
retained but no longer treated as current repository members.
