# Plus / 4 MB / System 6.0.8 verification

8 November 2026. Emulator execution, not physical hardware measurements.

## Environment and version correction

Snow 1.5.0-a921b10 with the project input patch, Macintosh Plus, 4,194,304 bytes RAM, MultiFinder, normal speed, no overclock or fast-forward. Clarice binary unchanged from the input verification. A separate disposable disk lives at work/system608/system.hda; its workspace is work/system608/plus.snoww. Launch with `python3 scripts/emulator.py start608` after stopping the owned emulator. The ordinary `start` and `install` commands still target the earlier project disk.

System and Finder were copied from the user's local 800K System 6.0.8 System Tools installation disk, with MultiFinder from Utilities 1. Finder Get Info confirms System Software 6.0.8 (system-version.png); the System vers resources 1 and 2 both identify 6.0.8, and Finder identifies 6.1.8. The running SysEnvirons query nevertheless returns 0607. Thus that value alone does not establish that this guest is 6.0.7. A contemporary developer [Gestalt investigation](https://groups.google.com/g/comp.sys.mac.programmer/c/c-2TeyhOzyc) also records the 6.0.8/0607 discrepancy, but our version identification rests on the local installation media and file evidence.

The earlier extracted work/emulator/System.bin also has 6.0.8 version resources. Earlier reports labelling the guest 6.0.7 from its environment query should be read with this correction; their recorded raw values and measurements are preserved. Clarice's status command still shows the raw API value, not a corrected release label.

## Interactive checks

- Up recalled echo second608, then echo first608. Two Down presses restored the unfinished draft608.
- Ctrl+C cleared the draft; a repeated press returned a fresh prompt.
- An endless echo loop scrolled continuously with no command prompt while busy. Ctrl+C stopped it; echo $? returned 130, and the following echo bad did not run.
- Command+period stopped a quiet infinite loop, returned status 130, and skipped the following echo bad.

## Limits

No physical Plus, SE, or System 7 result is implied. Screenshot observations and framebuffer equality tests cannot establish that every transient frame is flicker-free; exhaustive temporal flicker acceptance remains open. OS disk cache and system extensions were not changed. Host Control remains the documented Option alias on non-ADB guest models.

## Scrolling results

Full scrollbench: PASS, zero failures; 90 frames per run, three runs per renderer, 80 columns by 22 output rows. Native ticks: 507/506/507; cached C: 694/693/693; cached assembly: 567/567/567. Maximum measured frame times respectively 5, 7 and 6 guest ticks (60 ticks/sec). Native remains fastest in this workload, approximately 10.7 forced updates/sec. This is not keyboard latency or a physical-hardware frame-rate guarantee.

The suite covered 32/64/128-line ring wrap and view expiry, compositor and row-copy reference comparisons, incremental cached views versus fresh rendering, and four cache allocation/release cycles. Cache heap delta was 29,704 bytes with stable release cycles. Native framebuffer burst comparison separately passed 20 burst sizes with zero pixel mismatches.

Scrollbar Up moved one line and a track click moved a page into numbered retained output. Typing returned to the live output. Thumb dragging did not move under automation and is not marked passed. Shift+Up produced a slash rather than a scrollback action; this is an open modified-arrow mapping issue, distinct from the passing plain Up/Down history checks. These limitations prevent an unqualified all-scrolling-controls-pass claim.

Cached framebuffer comparison also passed 20 burst sizes with zero pixel mismatches. The 1,000-entry ls -la paused after file0020; Space advanced to file0041; Ctrl+C removed the More prompt and returned status 130. The installed Clarice data and resource forks exactly match the packaged binary. Reports were extracted only after a clean Clarice exit and emulator stop. No application code was changed for this test.

Follow-up: Shift-arrow mapping and buffered scrolling are covered in [the scrolling correction report](../scroll-fix/TEST-REPORT.md). The failures recorded above describe the earlier build.
