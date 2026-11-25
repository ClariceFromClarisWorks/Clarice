# Clarice 0.5.0 draft release notes

Build **2026111978**, product commit **f571f7c**. These notes describe a
locally staged release kit. Publication and redistribution clearance are
separate release gates.

- Native 68000 Macintosh shell and HFS tools for an existing System 6/7
  installation. Use System 6.0.8 and 4 MiB RAM as the tested baseline.
  The Core requests a 768 KiB application partition. System 7 and physical
  acceptance of this exact build remain pending. Earlier physical Macintosh
  SE testing used build 2026111965.
- The graphical installer creates or upgrades the selected Core and Data
  locations on one HFS volume. The offline kit includes Core, Recover
  Clarice and all 49 standard utility names. Optional vi and nano are
  separate, with their own prerequisites and limitations.
- Width-aware help, terminal editing, history, scrollback and Preferences
  support the normal 66-column window. HFS files keep Macintosh metadata
  and forks where supported. Shell pipelines are finite HFS spools. Unix
  process/job control and concurrent pipelines are outside the shell's scope.
- The standard collection includes ICMP, HTTP, passive FTP, Telnet,
  serial and Kermit clients. Network tools require a working Macintosh
  TCP/IP setup. HTTP, FTP and Telnet are plain-text protocols. Optional
  remote server components require separate installation and configuration.
- Quit the running Core before an offline upgrade. Keep the installer
  folder together and preserve Clarice Data. Use matching-Core media for
  explicit repairs. Run the snapshot's Recover Clarice from Recovery
  Previous for rollback. The updater's USTAR is a separate machine input.
- Prior hardware evidence recorded slow default-regex grep, 64 KiB sort
  memory failures and unresolved older system exceptions. Large workloads
  remain constrained by available contiguous heap. Installer C passed
  System 6 Snow and host checks. These results do not qualify physical
  performance or reliability of build 1978.

Installation steps: Start Here.txt. Attribution, corresponding source
and release gates: Third Party.txt.
