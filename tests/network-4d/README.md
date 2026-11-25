# Batch 4D focused probes

`sh tests/network-4d/run-host.sh` runs the existing HTTP/FTP/Telnet fault suites
plus buffered-parser event bounds, idle sleeping, cancellation and no-sleep
available-data checks under ASan/UBSan. Native asynchronous ownership and remote
service regression runners remain `tests/network-4b/run-host.sh` and
`tests/network-4c/run-host.sh`.

`build-probes.sh baseline|candidate` compiles disposable instrumented versions of
the actual foreground clients and native TCP adapter. Build baseline before
changing source. The same native objects and upstream parser/protocol engines are
used. It requires the existing Retro68 toolchain and network/Telnet build objects.
`profile-guest.c` counts foreground status/receive/send/yield calls, elapsed guest
ticks, and native system free memory around TCPCreate and final cleanup. It appends
results to `/tmp/4dmetrics`; probes are never distribution utilities. Per-yield
TickCount instrumentation adds cost, so compare ordinary client `time` measurements
separately. Neither guest elapsed time nor time inside a yield is pure CPU time.

`peers.py` serves generated binary fixtures in `work/network-4d/server` via the
existing pyftpdlib environment and HTTP. Use only disposable data. Generate files
named 1024,8192,65536,262144 containing repeated bytes0..255. HTTP routes
`/fast/SIZE`, `/header/SIZE`, `/idle/SIZE`, `/slow/SIZE`, `/stall/SIZE` respectively
serve immediately, add a4KiB header, delay headers750ms, pace2KiB chunks50ms, or
delay40s for the client's30s timeout. Host peers run independently of guest time:
use Snow1x for matched network timings, not maximum acceleration. Record host
latency separately; no extrapolation to physical hardware. Prior Telnet fixtures
supply negotiation/echo/cancellation and a delayed orderly-close workload.

`measure-snow.py baseline|candidate` records three matched HTTP/FTP repetitions
and byte comparisons through the existing localhost-only Snow Telnet fixture
(port23265). It expects profiling aliases wb/fb or wcand/fcand, normal wget/ftp
or wnormal/fnormal, and `/tmp/expectedSIZE` fixtures. Install only in a disposable
emulator image. Results go to `outputs/network-4d`; no physical host is contacted.
