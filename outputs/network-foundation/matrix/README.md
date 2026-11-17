# Snow stack/driver follow-up — 17 November 2026

Validation only: no production source changes/rebuild, RC changes, physical-SE
interaction, push or FTP/Telnet implementation. Existing command binaries reused.

## Results

| Pairing | Status / ping reply + timeout | Five named binary downloads / errors | Cancellation |
|---|---|---|---|
| Apple MacTCP “2.1” + MoofPort 1.0 | Pass | Pass | ICMP, recovery, zero tracked remaining |
| Apple MacTCP “2.1” + DaynaPORT | Pass | Pass | ICMP and active HTTP; partial output removed; recovery |
| MoofTCP 1.0 + DaynaPORT | Pass | Pass | ICMP and active HTTP; partial output removed; recovery |

Together with the original MoofTCP/MoofPort run this covers all four pairings.
The three `*-results.txt` files contain native API results, shell statuses and
heap measurements. `verification.txt` records byte comparisons and binary hashes.
All 15 downloaded 4,096-byte files match the host fixture exactly. Status is 0
for address/reply/download, 1 for ICMP timeout, NXDOMAIN and HTTP 404. Apple DNR
reports -23016 for the invalid name; MoofTCP reports -23045. Neither is success.

Five-run free heap: Apple/MoofPort 391,290–391,292 bytes, largest 391,182;
both DaynaPORT pairings 391,306–391,308, largest 391,218. Tracked remaining zero.
These are bounded repeatability checks, not proof against every system-stack leak.

Cancellation screenshots show real outstanding ICMP and an HTTP response already
streaming. Command-period returns to the prompt; `status runtime` reports zero
tracked remaining and subsequent `ipconfig` succeeds. HTTP cancelled at 12 KiB
(Apple) / 14 KiB (MoofTCP), before the advertised 128 KiB completed. Host logs
confirm disconnection. Offline HFS inspection found no `cancelled.bin`, failed
DNS destination or HTTP-404 destination. Cancel interrupts the sourced script,
so its trailing echo is not used as evidence of a returned status.

## Fixture and reproduction

Snow 1.5.0-a921b10, Plus/68000, 4 MB, MultiFinder, NAT Ethernet at SCSI ID 3;
independent images under ignored `work/network/matrix/{apple-moof,apple-dayna,moof-dayna}`.
Functional scripts use fixed uncapped emulation (~14–16x shown), normal speed
for automated typing/cancellation. Logged ticks are diagnostic only: asynchronous
DNS/network timers and host NAT make these unsuitable for hardware comparisons.

The guest System file's `vers` 1/2 says **6.0.8** (verification.txt). SysEnvirons
reports 6.0.7. This corrects the earlier report's inferred OS label without
modifying the OS or Clarice. System 7.1 and physical hardware remain pending.

DaynaPORT comes from the 7.5.3 installer, custom SCSI/Link component only; no
AppleTalk upgrade was selected. Native installer success is captured. Apple
MacTCP “2.1” is BlueSCSI's patched Apple 2.0.6, not stock Apple 2.0.6 acceptance.
Source: [BlueSCSI Wi-Fi setup](https://bluescsi.com/docs/WiFi-DaynaPORT), using its
linked bootstrap image. Download/extracted hashes matched MoofTCP's upstream
`tools/daynaport753.sha256`:

- Bootstrap ZIP: `20b00c5f32bdbe1df5cdf51aaa5066fa608a601569cb73c50e7aaf07e7a6b04e`
- DaynaPORT DC42 image: `70ab7aa5f1959c45471aa7cf04023a26cd876f08fe91ccf25163a1029344b2a6`
- Original MacTCP MacBinary: `1a152e3f6890dccd6d131df38e2d60e61bfc588011a264fc38706306c3a5a0c2`

Apple fixture initially selected LocalTalk despite a plausible address/mask,
so ping failed. Its stripped-down System Folder also lacked DA Handler, preventing
MultiFinder's Control Panel from appearing. Restored native DA Handler and General
from existing System 6 installation media, then used MacTCP's Control Panel to
select Ethernet, manual 10.0.0.2/24, router 10.0.0.1, default DNS 8.8.8.8 (domain
`.`). Cloned those native configuration files into the other disposable Apple
fixture. No production configuration writer was introduced. Preliminary
`apple-moof-ip.txt`/`apple-moof-ping.txt` are the failed setup attempt, superseded
by `apple-moof-results.txt`. Initial Dayna fixtures had older command binaries;
final Dayna result files were rerun with the hashes in verification.txt.

Run the existing `scripts/dev-http.py` on port 8092 for the trusted NAT peer,
serving a binary.dat containing bytes 0–255 repeated 16 times. Copy `accept.sh`
to `/tmp/accept.sh` and source it, redirecting output to `/tmp/network-results`.
Use fresh disposable output names on repeat runs; wget intentionally refuses to
overwrite. Apple/MoofPort used `matrix-` instead of `final-` output names.
For HTTP interruption, `slow-server.py` is a small test-only server, bound to the
recorded local address and restricted to the same peer. Source `slow.sh`, wait
for actual progress, send Command-period, then inspect runtime, run ipconfig,
and inspect output absence. The scripts contain test-host addresses, not product
configuration. Stop both host servers afterwards. Never host-mount an active
Snow disk image.

## Limits

ICMP replies cover Snow's NAT gateway, not routed Internet ICMP. DNS cancellation
and allocation-failure ordering retain existing host fault-test coverage; a stuck
resolver was not deliberately injected in these guests. Public DNR has no safe
hard-abort guarantee: Clarice must drain its callback before unloading code.
No new production defect was demonstrated. No general compatibility/performance
claim for other Apple/Moof versions, real Ethernet hardware, or System 7.1.
