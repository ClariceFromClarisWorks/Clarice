# Prepared upward row experiment — 2026111713

Isolated from RC2df1684. The user reported transient duplication only on the
scrollbar up arrow. Existing code moves visible pixels before composing the
exposed top row. This prototype prepares that row in the existing row bitmap,
then moves the validated overlapping pixels and presents the row consecutively.
No additional bitmap, persistent cache or allocation. Same stack-sized viewport
model; full visibility/model-match checks otherwise fall back to the old path.
QuickDraw clipping remains. Two screen operations are not VBL-atomic.

## Snow Plus / 4 MB / System 6.0.8

48 completed-image comparisons pass: full/narrow/hole clips, both themes,
selection and dirty row-buffer contents. Twelve observers verify the displayed
image is unchanged after row preparation, before the visible copy.
Three paired 24-step trials: light old165/159/157 vs new162/164/161 ticks;
dark166/162/165 vs163/164/165. Whole-operation performance is approximately
unchanged, not a speed gain. Sum of post-move drawing gaps falls light52–57 to
2–3ticks, dark59–60 to3–4; worst11–12 to1tick. Native ticks, accelerated Snow;
sub-tick gaps cannot be resolved, and visual acceptance requires hardware.

Host viewport composition and 1,560 cursor/scroll-guard cases pass. Existing
paste/selection sanitizer cases also pass. Physical result follows below.

Private build: work/direct-terminal-v9/build/Clarice.bin.
SHA256 ac8bc351bad4813b13617439b47c60788284075da94a6e05585776742d110734.
Full source overlay and changed-source hashes alongside this file preserve the
experiment; work/ is intentionally ignored. No RC promotion/push/release.

Snow Command-period stops a continuous output loop, returns130, and a following
command succeeds. A separately injected physical Control-key chord did not stop
the Plus/M0110 guest (existing automation limitation); Command-period recovered.
This is not recorded as a passing Ctrl+C hardware test.

Follow-up candidate if hardware shows row-validation overhead: reuse the existing
132-byte aligned row-equality kernel instead of a general bytewise memcmp. Not
implemented in1713; first measure this candidate on the SE.

## Camera setup (private development tooling)

User positioned the laptop facing the SE and authorised short captures. Native
AVFoundation video-only helper under ignored work/viewport/camera; no framework
installed and no microphone input. FaceTime HD,1280×720,~30.01fps. First40s paired
arrow clip saved locally; camera stopped afterwards. Readable whole-row motion,
but the CRT occupies only~300pixels horizontally. Brightness bands also occur
with stationary text, so they cannot be treated as drawing failures. The user then moved the laptop much closer and adjusted brightness; the final
framing resolves row numbers and the scrollbar (~700pixels across the CRT).
Video still does not establish pixel accuracy or rule out flashes shorter than
a camera frame.
Raw clips and room views are deliberately not tracked.

## Physical SE paired result

Stock SE68000/4MB/System6.0.8. Build1713 updated/relaunched, recovery retained.
48 completed-pixel and12 unchanged-during-preparation checks pass, failures0.
24 up steps per trial: light old188/194/195 → new180/178/179ticks
(medians194→179,7.7%); dark202/191/186 →186/185/187 (medians191→186,2.6%).
One dark pair differs by+1tick, so do not overstate a uniform speed gain.
Mean completed step ~135→124ms light/~133→129ms dark, using60ticks/s.

Post-move repair interval sums light66/72/70→3/6/3ticks;
dark74/70/74→3/4/3, with worst13–14→1tick. **This interval ends after all
exposed rows have been repaired**: old path paints top and bottom, new path
reuses the old bottom history row and paints only the prepared top. It is not
a separate measurement of exactly when the old top row alone became correct.
All preparations happen before visible movement; video/visual evidence remains
separate from these timing counters. No new bitmap or permanent cache allocation.
Diagnostic MacBinary grows290560→294016bytes, mostly probe and new helper code.

Actual held-up/down arrows were recorded in a15s private video at the closer
framing. Forty consecutive frames around an upward transition were inspected:
completed numbered rows and scrollbar remain correct; one mixed/blurred scan
frame cannot distinguish CRT exposure from the non-atomic screen copy. No
sustained duplicated line observed in that segment. User was not watching; no
human flicker-free acceptance claimed. Camera stopped.

Physical1713 continuous-output loops stopped separately with Ctrl+C and
Command-period: status130, subsequent command recovery, tracked survivors0,
freeheap326596/largest323310. Screenshots alongside this report. This does not
claim an endurance test or resolve the separate pointer anomaly.
