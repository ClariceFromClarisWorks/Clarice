# 0.5.0 candidate preparation — build 2026111701

17 November 2026. Development candidate, not final release acceptance. Physical
SE is unavailable by user instruction. Main, release tags and existing RC checkout
are unchanged. Recovery source checkpoint: 74b5132.

## Review and decisions

Retain the small batch FTP adapter for this release. Review of Thomas Pfau /
Mattias Gustafsson [ftplib](https://github.com/mattiasgustavsson/libs/blob/main/ftplib.h)
(Artistic 2.0) found useful broader remote operations, but its socket/select layer,
PASV-only connection path and server-advertised address use do not improve our
bounded GET/PUT requirement over the current native EPSV/PASV/control-peer design.
This is not a claim of general FTP-client feature equivalence. No LIST, interactive
remote management, TLS, resume or active mode. Data forks only.

Two real defects reproduced in host tests and fixed: STOR disconnect before150
now warns that the remote file may be partial (the server may truncate before
replying); SIZE421 now reports the server's shutdown response. Existing destination
refusal, staged local writes, completion226/250 and cleanup remain unchanged.

Retain the bounded VT model with genuine libtelnet protocol engine and unchanged
Core renderer. Reviewed [libvterm](https://github.com/neovim/libvterm), snapshot
934bc2fbf21800ac3458a499df8820ca5fb45fd3 (MIT). Its richer screen cells cost72KB
at80x25 on the target versus our ~11KB model. Parser-only reuse needs adapter and
bounds/exhaustion work, without demonstrated release benefit. Do not transplant
its screen layer. Established ESC intermediate/final parsing exposed a real
stray-character defect: unsupported ESC%G and multi-intermediate forms are now
consumed completely. Truncated terminal controls at peer EOF fail explicitly and
restore the terminal. No new permanent allocation, renderer or protocol framework.

Host evidence: `host.txt`: address/undefined sanitizers, FTP binary/failure/cancel/
resource cases, Telnet ownership/faults/restoration,200000 VT fuzz bytes,11 upstream
libtelnet fixtures, native DNS/ICMP mocks, HTTP framing/errors;482 updater transaction
failure/rollback cases. Added tests failed before the FTP/ESC fixes. Host ordinary
Telnet owned peak11744B, unchanged. Host measurements are not Macintosh RAM totals.

## Integrated build and packaging

`scripts/build.sh` builds native networking as independent CLX files; existing
installer list now47 entries. Build inputs remain pinned Retro68/68000. Reproduce
with build.sh, then distribution-disk.sh with an existing approved Apple Driver43
HDA template. No System software/ROM/private configuration ships in the kit.
Core MacBinary265344B; CLX ipconfig1756, ping3944, wget23244, FTP19176,
Telnet25048B. Manifest records exact per-command sizes/hashes. Core embeds none
of these payloads. No new Core startup networking or persistent memory.
Offline Read Me, third-party notices and libtelnet/pico licences included.
Nano remains separately installed, not in this candidate collection.

## Actual Snow acceptance

Plus/68000/4MB; existing System6.0.8 networking fixture lineage, with native API
reporting6.0.7. Maximum uncapped execution ~15–16x displayed; synthetic key entry
temporarily normal speed to prevent repeat events. No host-time speed claims.

- Fresh System Folder with no Clarice/Data: graphical Install Clarice installs
  Core and47 commands, automatically launches1701. Both installed Core forks
  match the source (resource file volatile header excluded). Relaunch works.
- Five network command full checks and help exit0.47 commands enumerated before
  test-only flush utility. Pipeline sort/uniq/grep, awk and mktemp-d pass.
- Three4096-byte FTP GET/PUT round trips contain all256 byte values, compared
  byte-for-byte on host. Existing destination returns1, remains unchanged.
- Chunked HTTP download returns0 and guest cmp against FTP fixture returns0.
- FTP post-pair native free heap379554,379552,379554B; largest379298 throughout;
  tracked remaining0. Complete upload ticks96/92/103, loader31/32/32. Network
  asynchronous/accelerated timings are observations, not hardware benchmarks.
- Real restricted MacBook Bash/VT100 connection, Vim9.1 edit/write/exit, less
  first-page and paging, remote exit, reconnection and Command-period local
  cancellation/terminal restoration observed. Snow Escape mapping required
  Option-[ as in earlier acceptance. Initial automation inserted unwanted text
  into the disposable Vim fixture; no production file touched. Saved fixture
  and shell logs retained locally. Numeric cancellation/heap capture from that
  run was not retained; do not claim those counters passed anew.
- Separate cloned fixture with MoofTCP removed and Ethernet target absent:
  Core launches, pwd/ls/awk work; ipconfig reports .IPP missing(-43), status1;
  subsequent awk prints OFFLINE_OK. Screenshot evidence included locally.

Initial stopping of Snow left some file/catalog updates buffered; extracted logs
were incomplete. The repeated batch's closed result was sent over FTP and a native
installation operation explicitly flushed the volume. `guest-results.txt` is the
actual complete repeat, not reconstructed output. `binary-integrity.json` records
its byte comparisons. This is not power-loss acceptance.

## Review boundaries and remaining acceptance

Focused source review checked FTP temp ownership/refused overwrite, protocol
limits and peer isolation, terminal ownership/error cleanup, and unchanged
installer/update rename/rollback. No new data-integrity defect identified in
these paths. Prior accepted installer interruption/independent-update and HFS
fork/metadata tests remain evidence, not rerun results for this candidate.

Reuse prior Apple MacTCP/MoofTCP and DaynaPORT/MoofPort matrix evidence: native
transport unchanged. This integrated guest run uses MoofTCP/DaynaPORT; no new
complete four-combination matrix, DNS-on-hardware or physical throughput claim.

Final blockers: consolidated stock-SE acceptance of the integrated candidate,
including cancellation counters/repeated Telnet heap, native forks/metadata and
installer recovery, followed by dedicated post-acceptance review and approval.
PowerBook7.1 smoke pending. No physical test requested while SE unavailable.

Known nonblocking limitations: unencrypted FTP/Telnet and HTTP only; HTTP no
redirect/close-delimited body; documented VT100 subset; native callback drain can
delay cancellation. SysEnvirons still reports6.0.7 on6.0.8; greeting says reported.
Updater28 and slow handoff are not reproduced or declared fixed. Transaction
failure tests preserve both versions; no new end-to-end self-update acceptance.
Nano three-second target remains unmet and outside this frozen release task.
