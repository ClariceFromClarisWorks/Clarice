# Focused utility audit reproduction

Use the existing Snow tools and an OFFLINE clone of the Plus/4 MB image. Never
mount a running guest image through hfsutils. Keep the physical SE untouched.

Generate inputs with `python3 tests/utility-audit/fixtures.py work/utility-audit`.
Create a disposable driver-bearing HDA using `scripts/package-hda.py`, then
hformat its HFS partition as Audit. Place inputs and these .sh fixtures at its
root; create empty/ and ten/ (ten empty files). Add it as a second SCSI target.
Stage the independently built awk.clx and sort.clx at Audit's root. Existing
scripts/build-awk.sh and scripts/build-pipelines.sh build these ports. Use the
current runtime object for pipeline linking; don't accidentally reuse an old
96 KiB/16 KiB-limited runtime. Source generation is in prepare-*.py.

Run `sh /mnt/Audit/bench.sh > /mnt/Audit/bench.txt` on the profiling application
and `sh /mnt/Audit/elastic.sh > /mnt/Audit/elastic.txt`. Benchmark uses actual
installed ls, not a replacement test body. Normal build 1538 checks read.sh.
The shared fixture reports 6.0.7; do not relabel as verified 6.0.8.

Before quitting/terminating Snow for offline inspection, compile/run flush.c as
an external test CLX to call native FlushVol on both named test volumes. The first
scratch image developed an evidence-file failure: elastic.txt was readable to its
end in the guest, but after restart hfsutils stopped at 6144 bytes and native cp
returned -127. Do not treat a partial export as acceptance. The original image is
preserved under work/utility-audit/audit-preserved.hda; a clean-image repeat is
recorded in the audit report. No production filesystem fix is inferred from this.
Export files to outputs/utility-audit/guest, then run validate.py. It checks exact
bytes, expected capacity failures, recovery, owned allocations and loader samples.

For interactive cancellation, run each candidate then send configured Control-C
(Option-C in this fixture) after its loader has finished:
  /mnt/Audit/awk.clx 1 /mnt/Audit/oom > /mnt/Audit/cancel-a.out
  /mnt/Audit/sort.clx /mnt/Audit/long > /mnt/Audit/cancel.out
Capture status immediately, then status runtime; both must report 130 and zero
owned survivors. Run a small command afterward and flush before offline export.

Host checks: tests/ports/elastic-text.py after the existing awk/pipeline host
build scripts; tests/shell/read-overflow.py after building tests/shell/host.c.
Host CLI shim aborts on retained owned allocations; optional CLARICE_TEST_STATS
prints peak/remaining. Existing awk and pipelines-more checks cover regression,
spill write failures and cancellation. ASan/UBSan are enabled by those builders.
