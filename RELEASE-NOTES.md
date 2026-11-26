# Clarice 0.5.1

Build **2026111979**, product commit **d44e891**, installs BusyBox 1.37.0
`vi` as `/bin/vi`. The standard installer and managed updater now
provide 50 utilities. The editor loads only when invoked.

## Macintosh SE acceptance

The validated target is a stock Macintosh SE with an 8 MHz Motorola
68000, 4 MiB RAM and System 6.0.8. System 7 and PowerBook acceptance
remain deferred.

The physical SE completed a managed upgrade from build 2026111978.
Core relaunched and all 50 installed utility checksums matched the
candidate. Fresh installation and offline upgrade were verified in
68000 Snow. The physical session tested empty, 1 KiB, 4 KiB and 8 KiB
editor sessions, navigation, search, editing, undo, paging, save and
reopen, discard, cancellation and switching between Finder and Clarice.
A save preserved the resource fork, Finder information and creation
date. Safe replacement resets the HFS last-backup date. An unsaved
edit interrupted with Command-period was recovered byte for byte.

| Physical SE workload | Median guest time | Range, ticks |
| --- | ---: | ---: |
| Empty launch and quit | 138 ticks (2.30 s) | 137-152 |
| Open 8 KiB and quit | 185 ticks (3.08 s) | 184-186 |
| Open 8 KiB, write and quit | 312 ticks (5.20 s) | 309-317 |

Three trials per case measured scripted command completion. Native
cursor and redraw latency was not isolated. The final idle heap had
328,672 bytes free, with a 326,604-byte largest block, compared with
351,456 and 350,890 bytes before testing. After purging, the differences
were 888 and 2,308 bytes respectively. The nine timed sessions reported
zero retained editor-owned allocations.

## Installation and recovery

On the Macintosh, expand the `.sit.bin` with a StuffIt-compatible
extractor. Keep the extracted folder together, quit the running Core,
and open **Install Clarice**. Use **Install/Upgrade** to retain existing
Clarice Data. The ZIP provides the same applications in MacBinary
format for transfer and decoding. Use matching-version media for
**Repair**. If interrupted, retain the same kit for retry. For rollback,
use **Recover Clarice** from the installed Data folder's
**Recovery Previous** snapshot.

## Known limitations

`vi` edits byte-oriented text and supports a subset of ex commands.
Unsaved-text recovery needs writable free space and is best effort.
See `vi --help` for supported commands. The stock SE can take several
seconds for editor operations and substantially longer for managed
upgrades. The historical intermittent physical-SE "Coprocessor not
installed" exception remains unresolved. It was not observed during
this focused session, which did not exercise quitting Core.

The GPL-2.0-only kit includes third-party notices and matching
corresponding source. Keep that source available alongside the
installer.
