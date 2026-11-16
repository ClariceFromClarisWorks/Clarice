# Installing Clarice Core and standard utilities

The core application contains the terminal, shell, builtins and runtime. Standard
commands are separate CLX files. Nano is optional and is not in this collection.
Use a complete distribution for first installation; a core-only update does not
install, inspect or replace utilities.

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

Quote paths containing spaces. A core-only update does not install or scan the
utility collection. After installation, try `ls /bin`, `awk 'BEGIN {print "ready"}'`
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
native services, with no installer code or resource added to ordinary Core. It
contains no ROM or System software. It retains only the existing template’s
Apple Driver43 device header/driver; provenance is recorded separately. Supply
a compatible template path as the script’s first argument if necessary. The MacBinary core preserves both forks;
CLX utility files are ordinary data files. Copying the native app through a
non-Macintosh filesystem requires its MacBinary container or an HFS image.
