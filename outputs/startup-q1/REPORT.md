# Q1-007 — startup (Snow acceptance, build 2026111954)

## Changes and cause

Existing namespace startup attempted nine directory creations, then checked each
failed create; it now checks existing directories once and creates only missing
ones. Wrong-kind paths/native errors still fail. Absent legacy reports no longer
cause redundant destination lookups; existing migration handles collisions/errors.

Create the document window at its intended native frame position. Retain the
actual WDEF-bound correction for extensions/nonzero screens. Skip preference
MoveWindow when the content origin already matches. One zero-mask, zero-sleep
WaitNextEvent completes MultiFinder's foreground handoff without removing queued
keys, followed by a native Starting message. Without that yield the message was
clipped behind stale Finder pixels. The existing forced terminal draw replaces it.
Keep busy until required setup/profile completion: no premature usable prompt.
No renderer, loader, updater, networking or runtime policy change.

## Matched measurements

Snow Plus, 4 MB, System6.0.8/Finder6.1.8/MultiFinder; fixed maximum FFWD (~15x),
Monaco9, 60×20/light, same disk/config/profile. Diagnostic copies of1953/1954;
normal distributions contain no new instrumentation. All times are60Hz guest
elapsed ticks, **not CPU-only work or physical-SE timing**. Stage profiling uses
TickCount; startup spans hundreds of ticks, so single-tick quantisation is small.
Asynchronous Window Manager/disk scheduling is included. Native launch proxy
writes/closes a timestamp then uses the established native Launch convention.
It excludes Finder click latency, includes timestamp I/O and native app loading.
“First” means first launch after reboot; host disk caches were not flushed.

| Boundary |1953 first / repeat|1954 first / repeat / repeat|
|---|---:|---:|
|Native launch proxy → C entry|140 /140|141 /141 /141|
|C entry → NewWindow return|30 /30|30 /30 /30|
|C entry → positioned window / early message call|51 /51 (no message)|41 /37 /37|
|C entry → ready state|196 /193|150 /144 /144|
|C entry → ready screen drawn|198 /195|151 /145 /145|
|Proxy → ready screen drawn|338 /335|292 /286 /286|

Total proxy latency **5.63→4.87 s first, 5.58→4.77 s repeat** (14–15% faster).
Inside C, drawn-prompt latency **3.30→2.52 s, 3.25→2.42 s** (24–26%).
First meaningful text previously painted during profile execution (~162–196 first,
160–193 repeat); new Starting draw finishes at41/37ticks. Separate1x screenshots
confirm visibility after handoff; these are not frame-perfect photometric timings.

Repeat-stage costs: directory loop22→13ticks; legacy reports15→6; preference restore
30→25 (already-correct origin); font/layout9→7; profile execution33→27 (includes
paint/event costs). Total startup terminal paint remains16–17ticks. Native pre-entry
~141ticks is now about half total latency. Further major improvement would require
investigating native CODE loading/partition work, outside this focused change.
No optional work was moved past readiness or given new resident state.

## Correctness / memory

- ASan/UBSan production-loop tests: existing/fresh paths, wrong-kind paths,
  native OOM/read-only/disk-full errors, report gates, actual WDEF correction.
- Existing geometry8505 cases, HFS volume/rename, viewport and control-input tests pass.
- Normal1954 Snow: fresh Clarice Data/default profile creation; no MacTCP/DNR/Prep
  installed, optional services disabled. Builtin commands succeed, zero owned
  command allocations. Existing utilities/data restored afterwards.
- Existing legacy report moved byte-for-byte to absent destination; colliding old
  source and destination both retained byte-for-byte. No duplicate root report.
- Normal1x startup typing `echo BOOT` before readiness appears intact before Return;
  Return executes once. No displayed premature prompt.
- Monaco9/52×16/dark restored; switching to60×20/light applies and survives restart.
  Scrollbar history traversal and typing-to-live work; clipping/geometry host tests
  pass. vi edits/saves/reopens; terminal restored; Ctrl+C stops infinite loop,
  reports130 and accepts subsequent commands. Zero retained command allocations.
- Three matched diagnostic launches have identical free heap385874/largest381316;
  baseline386086/382336. Difference212bytes free/1020largest is stable (code/system
  bookkeeping/placement, not a new buffer). No new allocator calls, persistent
  bitmap, cache, scheduler or partition increase. Normal fresh heap386242,
  largest382432; after editor/command use360616/356704, remaining allocations0.
- 68000 text236204→236412 (+208), data34848→34852 (+4), BSS41444→41456 (+12,
  linker layout; no added state symbols). Core MacBinary431872→432000 (+128).

Raw matched records and host results are beside this report; probe reproduction
is in tests/startup. One disposable guest Finder became unresponsive before
launch; rebooting the fixture recovered it. No Clarice crash observed. Normal
build, production loader/import validation and offline distribution build pass.
Physical SE, System7 and third-party window extensions were not tested; the latter's
bounds are host-simulated. No hardware accessed/deployed, no GitHub push.

Product **4827238**, build **2026111954**, integrated in local main. Canonical HTTP
and AFP publication verified: all51 entries/36 distinct payloads, sealed source,
offline HDA/archive and previous1953 recovery material. See verification.json.
