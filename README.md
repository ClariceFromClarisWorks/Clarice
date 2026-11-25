# Clarice

Clarice is a native command-line environment for 68k Macintosh computers
running System 6 or System 7. It uses HFS, the Macintosh Toolbox and the
installed system fonts. The 0.5.0 local release draft is **build 2026111978**,
product commit **f571f7c**. Its tested emulator baseline is a normal 68000,
System 6.0.8 and 4 MiB RAM. System 7 and physical acceptance of this exact
build are pending. Earlier physical-SE results apply to build 2026111965.

## Install and maintain

The local installer ZIP contains three MacBinary applications, the release
manifest, all 49 standard utilities and recursively packaged notices.
[Start Here](RELEASE-START.md) explains how to unpack it, transfer the files
to HFS and decode the applications with both forks intact. Open **Install
Clarice** in Finder and keep its files together. On upgrade, quit the running
Core first. The installer defaults to the startup-volume root for Core and
the System Folder for Clarice Data. Alternative destinations must share one
HFS volume. Keep the location record and the selected Data folder intact.

No network is required for this setup. Use matching-Core media for Repair.
For rollback after an interrupted or failed upgrade, use the snapshot's
**Recover Clarice** inside **Recovery Previous**. [Installation and recovery
details](INSTALL.md). The updater's USTAR archive is a separate input with
the existing Core, Release and Utilities layout. Apple driver-containing
private images are outside the installer ZIP.

## What you can use

- Shell built-ins with directory, variable and history state, plus a tested
  POSIX-sh subset through the mrsh parser and a Macintosh execution adapter.
  Use `help NAME` for built-ins and `NAME --help` for external commands.
  Finite pipelines are sequential HFS spools. General Unix executables,
  process/job control and concurrent pipelines are unsupported.
- HFS listings and traversal, per-fork file operations, Clarice soft links,
  and inspection/change of type, creator, Locked and Invisible with
  `hfsattrib`. HFS volumes are available under `/mnt`. Use `df` to see
  filesystem allocation space. It does not report raw device capacity.
- One True Awk, sbase grep/sed and everyday tools. Standard `wc`, `tr` and
  `sort` are BusyBox port builds, with distinct `*-sbase` alternatives.
  `wc -m` on the BusyBox build counts bytes; its `tr` is byte-oriented.
  The complete list is in [the installer inventory](src/standard-commands.inc).
- `ipconfig`, ICMP `ping`, HTTP `wget`, passive binary `ftp`, and the
  `telnet` client use the available native Macintosh networking stack.
  Core itself can work offline. HTTP, FTP and Telnet are plain text. The
  collection also includes `serial` and E-Kermit. Remote server modules
  require independent installation and configuration.
- Optional BusyBox `vi` and GNU nano are independently built and installed.
  They are outside the 49 standard names. See [editor limits](INSTALL.md)
  and [attribution](THIRD_PARTY.md).

## Terminal and capacity

The normal desktop window is 66 columns by 24 rows with Monaco 9. File >
Preferences (Command–Comma) controls geometry, colours, installed font,
scrollback, and the control-key mapping. Command–Period interrupts a
cooperative command. Up/Down recall history, and Shift–Up/Down page scrollback.
The input buffer holds up to 1,023 bytes, and scrollback retention is 32,
64 or 128 lines. Disk history keeps up to 128 records.

Core requests a **768 KiB application partition**, including a 64 KiB stack
reservation. Commands share that partition, and load and allocate on demand.
Available heap and the largest contiguous block matter more than free disk
space for memory-heavy commands. `free` reports memory and `df` reports HFS
space. Shell script, parser and invocation sizes are bounded by addressable
storage, available memory and implementation nesting limits. They do not
have the historical small fixed limits documented for the early prototype.
Scripts and utilities are not a blanket claim of POSIX or GNU conformance.

## Source and evidence

[Third-party attribution and redistribution gates](THIRD_PARTY.md) ·
[Draft release notes](RELEASE-NOTES.md) ·
[Current state and acceptance](outputs/CURRENT-STATE.md) ·
[Architecture](outputs/ARCHITECTURE.md).

For local builds, use the pinned 68000 Docker image in `scripts/build.sh`,
then run that script and `scripts/build-installer.sh`. Development disk tools
under `scripts/` are for local testing. [Release staging and checks](RELEASE-PACKAGING.md)
describes reusing an accepted build for the ZIP without changing the updater.
No blanket redistribution grant covers original Clarice code. See
[THIRD_PARTY.md](THIRD_PARTY.md) for the remaining release gates.
