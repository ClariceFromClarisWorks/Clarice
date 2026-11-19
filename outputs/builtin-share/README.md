# Built-in updater and mounted repositories — build 2026111940

Isolated integration of the accepted native-resource updater (ed74f56) and
mounted-source transport. No process/loader redesign or new networking stack.

## Cause

The PB180 ran Core1902 with its HTTP-only built-in updater. External updater
1891 on Macintosh Exchange included mounted-source support, but Core1902
correctly ignored that obsolete external component and refused a Core downgrade.
Installing optional vi1905 therefore did not update the built-in implementation.

## Change

Reuse update/source.h and its bounded 2 KiB native-file snapshot, preserve
HTTP transport and native-resource loading, CRC/length validation, recovery
helper, incomplete marker and catalogue. Both entry paths use the same source
validation. Repository leaves use 24 hex characters plus extension (28 bytes),
within HFS/AFP limits; complete input digests and collision checks remain.
No permanent allocation, startup scan or runtime ABI change. Mounted volumes
remain owned/authenticated by the Macintosh OS.

## Validation

Host ASan/UBSan mounted-copy failures/cancellation/cleanup, repository catalogue,
publication, 86 loader/installer cases, 482 transaction scenarios, extraction,
confirmation, import binding and native-resource lifecycle pass.
Snow and physical deployment evidence will be recorded after execution.

## Completed guest/hardware results
Snow HTTP1823→1940 automatically relaunched. Complete mounted repository
installed all49 utilities plus vi; catalogue matched every version/size/CRC.
Missing-file failure retained a successfully installed preceding package and
incomplete marker; retry cleared the marker. Corrupt CRC preserved the working
package. Mounted update.conf check/repeat passed, user fixture survived.
PB180/System7.1 HTTP1902→1940 automatically relaunched and displayed completion.
Final physical AFP check remains pending: control listener unavailable on next
turn. No SE tests. Historical error28 is not claimed fixed.
