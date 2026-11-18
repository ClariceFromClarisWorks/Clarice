# Physical SE acceptance and performance — 18 November 2026

Physical serial/Kermit and diagnostic nano/overall measurements completed.
Final normal Core1841 and eleven updated modules installed through clarice update -y.
Physical serial/filesystem checks passed; nano reopened the saved file on1841.
Normal1828 nano edit/save/search/reopen/cancel passed before that integration. Ordinary Telnet
acceptance remains blocked by the physical passive-open issue; proposed peer
filtering change awaits explicit security review, not assumed approval.

Branch codex/se-performance-20261118, isolated Clarice-SE-Performance worktree.
Based on7e3e036 plus an exact copy of Clarice-Main's working diff and untracked
files. Both original checkouts preserved. Runtime1.10; normal nano2026111827.
No push, published release, production optimisation or security-policy change.

| Scope | Evidence / boundary |
| --- | --- |
| Real-SE updater |1550→diagnostic1823 automatic relaunch and repository tool update passed; normal1828 and combined1841 handoffs below |
| Serial | Auth, commands, Ctrl+C130, repeated interrupted transfers, cleanup, reconnect passed |
| Physical cable | User unplug/replug plus explicit service reset and fresh login passed; not automatic detection |
| C-Kermit | Independent peer,4352B exact paced physical serial round trip passed |
| Telnet | Normal partial-wildcard listen refuses TCP; fixed-source-port diagnostic connects; normal acceptance blocked |
| Nano |20 matched diagnostic launches, exit/cleanup and identical images; full three-second goal fails |
| General performance | Four local samples/case, exact copy checks and separately corrected complete-pipeline timing |
| Exclusions | New cold-boot or System7 acceptance, long endurance, destructive power loss, native fork extraction and resource-fork failure injection |

## Initial access observations (resolved)

SE 192.168.1.252 answered two pings. Existing port5969 listener reports no SE.
Recovery listener on port5970 was absent; started using existing private token,
with a separate local socket. Still no authenticated helper connection observed.
Initially no USB serial device appeared in /dev/cu.*. Webcam succeeds (1280x720, ~30 fps,
video only) and shows Finder with SEControl Fast visible. Telnet port23 did not
connect. Requested opening the helper and attaching serial; cannot synthesize
physical keyboard access from a webcam. Private capture stays in ignored work/.

Historical PENDING-HARDWARE-TESTS contains superseded bullets. Use current dated
reports: earlier 1550 acceptance and SEControl reconnect already passed; this
session prioritises the new updater, remote sessions/Kermit and nano changes.
PowerBook/System7 and destructive power-loss testing are outside this session.

## Deployment evidence

Graphical connection restored on existing port5969 after user opened helper.
USB modem1401 appeared; 57600 probe returned no data (service not yet running).
Finder launched canonical System6.0.8:Clarice, Core1550, CPU68000/4MB.
Legacy clarice update -y upgraded and automatically relaunched Core1823.
Download274KiB reported13.70KiB/s; staging1508ticks, verification676ticks.
No error28 this run. First repository attempt failed safely with requested
download-limit/error-36: independently installed old clarice updater remains
legacy despite Core upgrade. Fetching current standalone updater for explicit
execution, then normal repository installation; original updater preserved.
The selected repository Core includes nano profiling instrumentation; normal
1823 Core hash differs and is retained separately. Do not present diagnostic
startup costs as normal-production measurements.

## Completed remote/transfer checkpoints

Repository update completed: six changed tools, matching standard tools retained;
normal nano1827, profile1826, seriald/telnetd1814, updater/Kermit1823. Original
independent recovery files retained. GUI and HTTP remain usable.

Physical serial: authenticated variables, Ctrl+C130, logout/reconnect, three
interrupted partial receives with no destination/temp remnants pass. Free/largest
heap319552/317302 on all three cycles, remaining0. User physically unplugged USB
for~5s and reconnected; explicit local service reset followed by fresh
authentication23.177s and successful PHYSICAL_RECONNECTED command, status0,
remaining0. This is recovery with explicit service reset, not automatic cable
loss detection. Raw evidence: serial.json, cable-recovery.json.

Independent C-Kermit9.0.302 passed the 4352-byte binary/MacBinary-container
round trip using a bounded private PTY bridge paced2ms/byte to the physical USB
serial adapter. Returned bytes and SHA256 match exactly.52.858s includes login,
commands and both transfers, not raw throughput. Native resource-fork extraction
is not implied. Unpaced transfer failed; CDC setup and pacing are required in
this harness. Evidence: kermit-serial-paced.json. Host fixture originally truncated
IAC DO BINARY's zero option byte; corrected counted feed, full remote host suite
passes under sanitizers. Separate TELNET-ISSUES.md tracks native pending-open
versus refused TCP connections and shared slow authentication.

## Overall performance observations

Diagnostic Core1823, physical SE, remote services off, SEControl active. First
samples are first in this session, not cold-boot measurements. Complete-command
`time` includes loading; fixture preparation and exact-copy verification are
separate. Four samples each; raw overall.json and parsed overall-summary.json.

| Case | First seconds | Subsequent seconds |
| --- | ---: | --- |
| Tiny builtin printf | <0.02 | <0.02, <0.02, <0.02 |
| External wc on23B | 1.13 | 1.16, 1.26, 1.23 |
| wc -c on64KiB | 19.71 | 18.16, 19.23, 18.56 |
| Copy64KiB | 3.20 | 3.63, 4.30, 4.46 |
| ls -l /bin to null | 1.85 | 2.41, 1.86, 1.70 |

All four exact copy comparisons pass. Script-context free heap314768–314778B,
no progressive tracked survivors. Builtin time rounds to zero; its CLX counters
refer to the preceding external invocation and are excluded from interpretation.
The initial pipeline timing covered only cat; do not treat it as whole-pipeline
latency. A corrected `time sh -c ...` run was executed separately.

wc64KiB makes129 instrumented reads totaling57–65ticks, versus1080–1173 invocation
ticks and32–34 loader ticks. Source inspection finds genuine upstream sbase wc
always decodes runes and performs Unicode whitespace classification, including
byte-count-only mode. This is a measured investigation candidate, not a new
optimisation or proof that every remaining tick belongs to that loop. Prefer a
small faithful byte-only path or established upstream approach over a new I/O
framework; preserve streams, errors and cancellation. No production change here.

Whole-pipeline follow-up (`cat | grep | wc`, including shell and spool work):
3.86s first,3.73/3.70/4.13s subsequent; all status0 (pipeline.json).

## Physical nano comparison

Before/after variants are identified by exact hashes in nano-variants.json; both
were installed as nano-profile, with prior recovery files moved aside safely.
The current buffered variant was restored after the baseline. Remote services
were off throughout timed runs. All20 editor exits status0, remaining0; ten paired
screenshots byte-for-byte identical. First baseline screen was visually inspected;
later image-preview failure limited review to exact-image comparisons and local
Apple Vision text recognition. Webcam movies and original screenshots retained
in ignored work/se-performance. No invented video latency measurements.

| Workload | Before first / repeats (ticks) | Buffered first / repeats (ticks) |
| --- | --- | --- |
| No file | 259 /264,294,311 | 260 /254,259,254 |
| Existing empty | 342 /not sampled | 294 /not sampled |
| Ordinary8KiB | 558 /499,508,499 | 542 /578,474,479 |
| Two-long-line8KiB | 424 /not sampled | 490 /not sampled |

60ticks/s. First means first in the variant session, not cold boot. Mark0 is
AFTER command echo repaint; mark8 is after initial draw before input wait. Thus
these are startup lower bounds for the full submission-to-responsive criterion,
not complete wall-clock certification. Non-modifying Right and Ctrl+X after the
readiness wait demonstrate eventual input handling. Three-second goal remains
unmet even within this shorter interval: buffered empty4.23–4.33s and ordinary
7.90–9.63s. Do not attribute empty-case differences to a buffered read change.

Ordinary warm mean502→510.33ticks (8.37→8.51s), median499→479 (8.32→7.98s).
File-open/buffer phase147→134/128/130ticks; long-file181→156. However buffered
short1 and long0 have curses/setup spikes79+90/91ticks versus typical31–44+36–37.
These dominate complete-operation variance; no consistent physical overall gain
is accepted. Next focused measurement should separate allocation/initialisation
variance from file preparation/layout, retaining complete-operation outcomes.
Typical remaining costs: loader86–90ticks, short-file layout58–63, initial
refresh102–106. No renderer or allocation-policy changes were made here.

Owned peaks unchanged:74572B no-file,75112 empty,101736 ordinary,95662 long.
Native free heap340512–362456B, largest337422–359380B; layout changes between
cases and no progressive decline. These are post-invocation counters, not peak
whole-application RAM. Raw nano.json, nano-summary.json and image hashes retained.

## Normal Core1828 handback checkpoint

Private normal Core278784B, SHA25640daba4e1aab21e2232619b53b23c7e4fd33e8dc0aebf1a25fb14c9a116cae4b.
Existing updater verified both forks, staged1509ticks and verified678ticks, then
automatically relaunched with build1828 and update-complete greeting. Recovery
retained; no error28 reproduced. This is a Core-only private build reusing
independent packages, not a sealed full distribution. Default update.conf unchanged.

Normal nano1827 saved/reopened exact `SE EDIT ORIGINAL\n`, preserved TEXT/ttxt,
returned0/remaining0. Search ORIGINAL moved to column9 (location display).
Command-period during startup returned130/remaining0 and restored the shell.
Evidence normal-acceptance.json plus captured screens. Resource-fork fault
injection not repeated. Image review after the preview failure used saved-image
OCR and exact serial contents/status checks, not claimed direct visual inspection.

## Concurrent development integration1841

While the physical run was active, Dev integrated47eb033 with the prior nano,
serial and Kermit work, source1841/runtime1.10. Copied that combined source into
this isolated branch, preserving pre1841 task source under ignored recovery and
retaining only opt-in TCP diagnostics. Original checkouts untouched. Combined
source and copied test/report hashes: integration1841.json. Historical timing
results above remain1823 diagnostic, not silently attributed to1841.

The upstream integration report records combined host regression and 68000 build
coverage. This branch repeated focused service/auth/serial and filesystem/cwd
host regressions successfully; normal native Core/files/services build and
physical updater follow-up succeeded: all eleven changed modules fetched, Core1841
automatically relaunched and reported update complete. Focused hardware checks
follow. No peer-policy change.

Combined packages: normal Core279552B (+768 versus1828), files13268B (+272 versus
prior12996B; shared across nine command names), telnetd40784B (+1504) and
seriald30060B (+1124). These include the combined existing development work,
not an isolated filesystem-fix cost. Remote module hashes exactly match the
upstream integration's validated binaries. packages1841.json retains all hashes.

Physical1841 follow-up passes: locked source returns1 while source NEW and target
OLD remain intact; fixture unlocked afterwards. Empty mkdir returns1. Renaming
an ancestor updates remote cwd to /tmp/sf1841/c/b. Remote nano returns2 with the
local-input diagnostic; telnetd help succeeds. Serial cursor insertion produces
ABC, and prior saved nano contents/TEXT+ttxt metadata remain intact. Remaining0;
post-check native heap342318B/largest341948B with serial enabled. This single
sample is not a new endurance measurement. Core staging1409ticks, verification
646ticks; previous Core and tool recovery files retained. Ordinary Telnet still
refuses TCP on1841. Full data: integration1841-hardware.json and telnet1841.json.

All performance comparisons remain explicitly diagnostic1823. No fresh combined
Snow run or full1841 performance campaign is claimed. Earlier integration host
and fix-branch Snow evidence plus the focused1841 physical checks are distinct.

Final1841 nano reopen screenshot is byte-identical to the verified1828 reopen
screen with SE EDIT ORIGINAL. Original private update source config unchanged;
explicit tested source is http://192.168.1.112:8086/repository/normal.txt while the
private development server is running. No public publication or push.

## Remaining decisions and recommended next step

Normal Telnet acceptance is blocked by T1. The proposed wildcard native listen
plus immediate allowed-peer rejection before greeting/auth was subsequently
approved by the user. Candidate1843 is implemented and host/Snow tested; see
[approved fix report](../telnet-peer-fix/README.md). Next, restore SE access and
test both allowed and disallowed clients on the physical machine. For performance, investigate nano initialisation variance and the measured
wc byte-only unnecessary work before adding infrastructure or assembly.

No cold-boot, full end-to-end video latency, whole-application peak-memory,
System7 or long-duration acceptance is claimed. The nano goal remains unmet;
the reports distinguish first/repeat, instrumentation, physical and Snow evidence.

Final console confirms Telnet and serial disabled, normal nano remaining0,
post-exit heap364586B/largest361290B, and a local prompt. These final values are
from saved console/OCR; invocation ticks include test waiting and are not startup
timing. SEControl5969 remains available. Unused recovery listener5970 stopped;
private HTTP8086 retained for explicit-source development iterations. Handback
state and final screenshot hash: handback.json.

## Integration into the development tree

The performance harness, evidence and opt-in Telnet diagnostics are integrated
into Clarice-Main on codex/coherent-deployment. Existing uncommitted nano,
serial/Kermit, filesystem and release-recovery work remains intact. Current-state
notes retain both the physical-SE results and newer filesystem integration entries.
The normal service behavior and peer policy are unchanged; diagnostics remain
compile-time opt-in. No rebuild or hardware redeployment was needed for this merge.

Focused session/authentication/service/serial host regressions, Python compilation,
shell syntax and diff whitespace checks pass in the destination tree. Integration
is part of the existing uncommitted development work; no push or release occurred.
Recovery location and preservation audit are in development-integration.json.

Raw webcam captures, screenshots, private credentials and generated private HTTP
packages remain in the original isolated checkout at
`/Users/williamprestwich/Documents/ChatGPT/Clarice-SE-Performance/work/se-performance`.
Earlier work/se-performance references in this report refer to that capture root.
Credentials were not copied into the development tree. Tests requiring them must
be configured explicitly before another hardware run; no hardware tests run as
part of this integration.
