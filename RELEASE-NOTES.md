# Clarice 0.5.0

Build **2026111978** · Product commit **f571f7c**

Validated target: **Macintosh SE, stock 8 MHz 68000, 4 MB RAM, System 6.0.8**. System 7 and PowerBook acceptance are deferred.

- Native shell, HFS tools, width-aware help, Preferences and 49 standard utilities. Shell pipelines use sequential disk spools. The installer includes Core and Recover Clarice. Optional vi and nano are outside this release kit.
- On the Macintosh, expand `Clarice-1978-Installer.sit.bin` with StuffIt Expander, keep the extracted folder together, then open **Install Clarice**. Quit the running Core before upgrading. For Repair, use matching-build media. For rollback, launch **Recover Clarice** from the installed Data folder's **Recovery Previous** snapshot.
- Large text jobs can be very slow on an 8 MHz SE and may exhaust the available contiguous heap. The earlier hardware baseline recorded particularly slow regex grep and 64 KiB sort memory failures. Scrolling and redraw can lag during sustained terminal output.
- The physical SE has intermittently reported **“Coprocessor not installed”**, including on build 2026111978. The cause remains unknown. The release carries this reliability risk.
- Original Clarice contributions within the project owner's authority use GPL-2.0-only. Third-party terms and attribution remain in the kit. The complete corresponding-source archive and SHA-256 manifest are available alongside both installer formats.

The installer archives preserve the verified 1978 payloads and their embedded staging-era notes. This page reflects the final release status.
