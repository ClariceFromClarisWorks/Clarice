Clarice 0.4.0 improves responsiveness and update safety on the Macintosh Plus / 4 MB / System 6.0.8 baseline.

- Faster text selection: changed highlight rectangles replace full-screen redraws. Guest highlight updates measured at most one 60 Hz tick, versus roughly one second before. Scrollback clearing and cached-copy alignment are corrected.
- Faster keyboard processing and explicit overflow rejection. Long/multiline paste cannot silently submit a truncated command.
- Correct HFS volume selection for `/mnt` paths, including names containing spaces.
- Existing commands survive startup unchanged. `appinstall` provides explicit validation, replacement, repair and recovery; independently distributed commands can declare runtime/capability requirements.

Includes the existing `/bin` utilities and upstream sbase/One True Awk ports. Experimental ncurses/nano adapters remain in source; a working nano editor is **not** bundled. Finite pipelines remain sequential disk spools, not concurrent Unix pipes. No claim of full POSIX compliance.

### Downloads and physical Mac installation

- **HD60_512_Clarice-0.4.0.hda**: 32 MiB HFS application volume in a full SCSI device image, with Apple partition map and Apple_Driver43 partition. Intended for BlueSCSI/ZuluSCSI. Default SCSI ID 6; rename for an unused ID if necessary. For ZuluSCSI, `HD6_Clarice-0.4.0.hda` is also suitable.
- **Clarice.bin**: resource-fork-preserving MacBinary application.
- **Clarice.dsk**: 800 KiB HFS application floppy.
- **Clarice-0.4.0.zip**: source, licences, reports and application packages.
- **SHA256SUMS.txt**: download checksums.

The HDA is **not bootable**. Keep your existing System disk, mount the Clarice disk, and **copy Clarice onto your boot disk before launching**. Direct launch from a secondary volume can report preferences-save error -43; the previous preferences file is retained. Quit the old Clarice before replacing it. Existing commands/configuration are preserved; use `appinstall --help` for explicit command updates.

Snow Plus/System 6.0.8 guest tests and focused host checks are documented in the source reports. Physical BlueSCSI/ZuluSCSI hardware remains unverified. The HDA contains no System/Finder/MultiFinder, ROM, emulator or user-data copy. Its separate Apple SCSI driver is credited in THIRD_PARTY.md.
