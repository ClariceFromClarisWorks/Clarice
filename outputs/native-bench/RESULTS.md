# Minimal native launch comparison — 16 November 2026

Test-only sources: `tests/native-bench`. Native worker and CLX driver execute the
same bounded argument/output/status body. Worker uses native CODE resources and
System 6 MultiFinder LC sublaunch; returns normally through ExitToShell. A fresh
186-byte HFS request carries one argument and bounded stdout/stderr/status.
No Clarice production process API, IPC framework or executable format change.

Three paired Snow runs, installed-fast driver, same Plus 4 MB fixture:

| Route | Total invocation ticks | Seconds |
|---|---|---|
| CLX common body | 25, 27, 27 | 0.42, 0.45, 0.45 |
| Native APPL through test driver | 126, 127, 126 | 2.10, 2.12, 2.10 |

Native driver loading contributes 25–26 ticks; worker entry is reached 55–56
additional ticks after driver entry; worker initialisation/body takes 4–5 ticks.
Driver entry through result forwarding takes 100 ticks. Thus native process and
file exchange add about 1.67 s in this probe, independent of the shared CLX driver.
This is not a claim that native loading itself takes 1.67 s. Completion includes
actual launch, worker execution, return and stdout/stderr/status forwarding;
module timing excludes final prompt painting. Screenshots show the returned prompt.
Initial unknown-path runs are excluded from the matched installed-fast series.

All runs produced `alpha beta`, expected stderr and status 7. Post-invocation heap
421176 bytes, owned remaining zero throughout paired samples. Native SIZE is
160 KiB; worker FreeMem reports 4664 bytes after Retro68 startup, not the total
memory available through libc or a meaningful application peak. No large-app
memory scalability conclusion is justified by this tiny worker.

The first native attempt mistakenly treated a positive Launch return as an error.
Apple's System 6 sublaunch note specifies negative values as errors; correcting
that interpretation allowed the actual returning launches above. The prior
request was exported and explicitly removed before repeating the fixture.

Limits: file-based exchange has its own HFS cost; no streaming terminal input,
remote cancellation protocol or full environment is implemented. Local waiting is
bounded and cancellable, but worker cancellation is not tested. Do not compare
this small body directly with nano. Physical APPL measurements are recorded below; direct Finder
warning acceptance remains pending. The fixture greets as System 6.0.7; distinguish
that reported version from historical descriptions of this Snow fixture as 6.0.8.

Recommendation: keep optimised CLX for now, especially small commands and single
Finder. Native processes remain plausible for separate application partitions and
lifecycle isolation, not a demonstrated speed improvement. Do not build a process
system merely to improve this benchmark.

Reference: Apple's System 6 LC sublaunch block and return convention,
https://ioi-xd.net/os9docs/pdf/launching_another_application_under_system_6.pdf .

## Physical SE matched repeats

Build 1534, stock 68000 / 4 MB, same worker and common body as Snow. The screen
reports System 6.0.7, although the configured baseline has been described as 6.0.8;
record the observed version rather than claiming independent OS-version acceptance.

| Route | Total invocation ticks | Seconds |
|---|---|---|
| Installed-fast CLX | 34, 34, 33 | 0.57, 0.57, 0.55 |
| Native APPL through same driver | 157, 155, 155 | 2.62, 2.58, 2.58 |

Native driver load: 32/34/33 ticks. Driver entry through returned output: 124/121/121
ticks; native worker entry 69/65/66 and worker initialisation/body 10/9/9 ticks.
Post-invocation heap 420992 in every pair; owned remaining zero. Worker SIZE remains
160 KiB and FreeMem is 4652, subject to the libc-reservation caveat above.
Screenshots se-{0,1,2}-nat.png show both routes, expected output/status and prompt.
The native route is about 4.6 times the complete CLX invocation cost for this tiny
workload; its extra roughly two seconds includes file exchange and scheduling.
This is not evidence against all native applications, nor a forecast for nano.

Test worker deployment used an isolated embedded package/extractor command, first
validated in Snow (successful native execution; duplicate destination refused).
Only /tmp/NBWorker was created; Clarice itself was not replaced. No native process
architecture or general unpacker was added to production.
