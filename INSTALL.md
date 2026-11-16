# Installing Clarice Core and standard utilities

The core application contains the terminal, shell, builtins and runtime. Standard
commands are separate CLX files. Nano is optional and is not in this collection.
Use a complete distribution for first installation; a core-only update does not
install, inspect or replace utilities.

## Offline installation

Mount the Clarice distribution disk. Copy the Clarice application to your chosen
folder in Finder and launch it. Its normal first launch creates `Clarice Data`
inside the active System Folder, preserving existing preferences and user files.

At the prompt run:

```
appinstall setup /mnt/Clarice/Utilities
```

Substitute the mounted disk label if different; quote paths containing spaces.
Setup streams files from the disk into `/bin`, verifies each using the existing
CLX loader, and installs only absent names. **Every existing file is preserved,
even a damaged or incompatible one.** Setup does not declare those files valid.
No network or preinstalled external command is needed. Retain the utility disk
for repair. A folder of the raw CLX files works equally well.

After completion, try `ls /bin`, `awk 'BEGIN {print "ready"}'`, and `help`.
Neither restarting Clarice nor updating its core repeats setup.

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
application, Utilities, installation instructions and third-party notices. It
contains no ROM or System software. It retains only the existing template’s
Apple Driver43 device header/driver; provenance is recorded separately. Supply
a compatible template path as the script’s first argument if necessary. The MacBinary core preserves both forks;
CLX utility files are ordinary data files. Copying the native app through a
non-Macintosh filesystem requires its MacBinary container or an HFS image.
