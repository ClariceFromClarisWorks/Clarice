# Clarice 0.4.0 release validation

11 November 2026. User authorised completion and publication after the interrupted QOL-3A session.

- [Task 2 guest acceptance](../task2/guest/ACCEPTANCE.md): prior executed installation/replacement/recovery/runtime checks; not repeated during packaging.
- [QOL-1](../qol1/TEST-REPORT.md): prior multi-volume HFS tests; not rerun during packaging.
- [QOL-2](../qol2/TEST-REPORT.md): prior input safety and latency tests; not rerun during packaging.
- [QOL-3A](../qol3a/TEST-REPORT.md): recovered final log confirms 56 pixel comparisons with zero mismatches, maximum highlight update of one tick, zero heap delta. Focused host ASan/UBSan selection tests passed. No unrelated suites rerun.
- Normal 68000 application built with all input/selection diagnostics OFF. Release MacBinary is that build; application forks round-tripped through the HDA exactly and Finder type/creator remain APPL/CLRC.
- Clean HDA created by `scripts/package-hda.py`: only descriptor, partition map and Apple_Driver43 partition retained from the formatted template. HFS and unused space start empty; zero template HFS bytes copied. [Driver/layout provenance](HDA-PROVENANCE.json), [hfsutils file listing](media-build.txt).
- A disposable copy of the HDA mounted at SCSI ID 6 alongside the existing boot disk in Snow (Plus, 4 MB, System 6.0.8/MultiFinder). Finder showed Clarice and Read Me; the application launched from that volume, read the configuration through `cat`, and handled mouse selection and scrollback. See the screenshots in this directory.
- The same release application installed on the boot volume launched cleanly, executed repeated `cat` commands, and passed HID selection followed by scrollbar movement and a new multiline selection without stale highlight or a preferences error; [installed-path screenshot](installed-selection.png).
- Direct secondary-volume launch exposed preferences-save error -43. Prior preferences were retained. Installation instructions therefore require copying the application to the boot volume. This limitation is not hidden or represented as fixed.

No physical-hardware acceptance is claimed. The HDA is application media, not a bootable OS disk. Native and cached selection are tested at Monaco 9; all font/window configurations are not exhaustively covered. No nano editor, worker architecture or subsequent roadmap implementation was added.

Naming references: [BlueSCSI usage](https://bluescsi.com/docs/Usage), [ZuluSCSI firmware documentation](https://github.com/ZuluSCSI/ZuluSCSI-firmware). Keep an existing boot disk and use an unused SCSI ID.
