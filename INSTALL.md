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

## Private repository updates

`clarice update` is built into Core. It does not require an installed
`/bin/clarice`, external wget or separately downloaded updater. The native helper
that replaces the running application is supplied automatically. Existing old
updater files are preserved but do not override the builtin command; old repository
entries for an external `clarice` command are ignored.

Configure `/etc/clarice/update.conf` with one private HTTP index URL, for example
`http://192.168.1.112:8083/index.txt`, then run:

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

Per-tool `.clx-NAME.old` retains the previous file, including both forks. On another
repository replacement, older recovery files are preserved as `.previous-CATALOGID`.
These archives consume disk space; review them before deleting older recovery copies.
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
python3 scripts/dev-http.py --bind HOST_IP --peer CLIENT_IP --port 8083 --directory work/private-repository
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
2. Double-click **Install Clarice**, then click **Install**.
3. Wait for the file names and checking messages. Clarice opens automatically
   when installation finishes. Allow several minutes; do not reset the Mac.

The installer puts **Clarice** in the active **System Folder**, beside
**Clarice Data**. To open it later, double-click System Folder → Clarice.
You can eject the distribution disk afterwards; keep it for repair.

No network, command-line knowledge, archive extractor or preinstalled utility
is needed. The installer uses the Utilities folder beside it, so it also works
if the disk label changes. Keep the disk's files together.

Existing utilities, including newer, customised or damaged files, are kept
unchanged. Installation does not declare existing files valid. Preferences and
user files are preserved. The installer does not run your startup profile;
normal Clarice runs it when launched after installation.

For an upgrade, quit installed Clarice first and run Install Clarice again.
An older **System Folder:Clarice** becomes **Clarice Previous**, retaining both
forks and Finder metadata. The new Core retains its Finder information and
creation date. An equal/newer Core is kept rather than replaced. Earlier copies
elsewhere on your disk are left alone; use System Folder:Clarice afterwards.

If **Clarice Previous** or **Clarice Installing** already exists, the installer
refuses another Core replacement. Inspect/move those recovery files first; never
discard your only working copy. This is not an automatic cleanup or downgrade.

### Cancel and resume

Click Cancel before starting, or press **Control-C** during installation. Completed
utilities remain installed. Open **Install Clarice** again and click Install to
resume; existing files are skipped. The current Core is kept until its replacement
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

BusyBox vi remains an optional experiment, not part of this replacement set.
