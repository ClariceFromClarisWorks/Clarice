Clarice 0.5.1 development installer
================================

Local release draft. See Third Party.txt for redistribution status.
Use an existing System 6.0.8 HFS installation on a 68000 with 4 MiB RAM.
This local build 2026111979 includes BusyBox vi and 49 other utilities.

On the Macintosh, expand Clarice-1979-Installer.sit.bin with a
StuffIt-compatible extractor. The applications in that archive are
already decoded. Keep the extracted folder together and open
Install Clarice after quitting the current Core.

For the maintainer ZIP, follow these transfer steps:

1. Unzip the kit on a modern computer. Copy the complete Clarice 0.5.0
   folder to a writable HFS volume visible to the Macintosh. Transfer
   the .bin files, Release and Utilities in binary mode.
2. In that folder, use a MacBinary-aware tool to decode all three
   application .bin files. The Finder names must be Clarice, Install
   Clarice and Recover Clarice. Copy both forks when moving decoded
   applications. Do not open the enclosed Core as a substitute for setup.
3. Quit any running Clarice. Open Install Clarice from the same folder
   as Clarice, Recover Clarice, Release and Utilities.
4. Keep the suggested locations or choose Core and Data folders on the
   same HFS volume. Click Install/Upgrade, allow several minutes, and
   wait for Clarice to open. No network connection is required.

Defaults are the startup volume root for Clarice and the System Folder
for Clarice Data. Keep the selected locations and the installer folder
for maintenance. Repair standard tools with matching Core media. The
installer preserves user files and preferences. Uninstall defaults to
application-only removal; full Data deletion needs separate confirmation.

If an upgrade is interrupted, retain the installation files and retry
with the same kit. For rollback, open Recover Clarice inside the
installed Data folder's etc:clarice:Recovery Previous directory. Use
that snapshot's helper and files together. Retain a separate backup.

The updater USTAR is a separate machine-readable input. The ZIP is
the Finder installation kit. It contains no Apple driver or system media.
Matching source materials: @SOURCE_ARCHIVE@ (stage alongside this ZIP).
