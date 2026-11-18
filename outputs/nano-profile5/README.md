# Rejected initial-paint deferral experiment

18 November2026. Fixed-max Snow Plus/4MiB/System6.0.8. Same nano executable,
Core1819 baseline versus1821 candidate, deployed through clarice update.
Candidate delayed cooperative painting of a newly opened editor until its first
output or input request. No new buffers; events/update-event drawing still ran.

same-name.json contains actual warm timings. Mean totals: no file224.7→222;
existing empty271.3→279; short8KiB443.3→414; long8KiB339.7→334 guest ticks.
The apparent short-file gain includes unrelated loader/initialisation variation.
Rows fell by only one (77→76 for short), painting remained~63ticks, and some work
moved between setup/layout. No convincing general end-to-end improvement.
**Removed rather than retained on those figures.** Runtime remains1.10.

Host production-code tests covered no-output startup, output/pending-input/read
barriers, allocation/admission failure, event cancellation and close/reopen cleanup.
All passed. Experiment source/tests remain in ignored work storage. No physical
hardware acceptance is claimed. Subsequent work returns the renderer to baseline
and investigates actual buffered byte-reading overhead in the nano adapter.
