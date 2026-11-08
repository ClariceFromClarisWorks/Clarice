# Clarice identity update

The current product name is Clarice. New UI, build artifacts and first-run data use Clarice / Clarice Data. Historical reports and original briefs retain their names. Previous development data is preserved and is not automatically migrated.

The original monochrome icon depicts a command window, output lines, a chevron and a solid block cursor. It is drawn separately at 32×32 and 16×16 pixels. `scripts/make-icon.py` reproduces the resource artwork and preview without third-party artwork. The application creator is CLRC; existing S6UX shell-file metadata is retained for format compatibility. Packaging sets the Finder hasBundle flag and updates the MacBinary header checksum, then copies that application onto its HFS distribution disk.

The pinned offline 68000 build passes. Host shell fixtures pass 31/31; cooperative cancellation fixtures pass 3/3, covering a loop, nested shell and pipeline. Those host tests do not establish guest keyboard delivery or POSIX conformance.

Snow's Plus model emulates the M0110 keyboard without a Control key (https://docs.snowemu.com/manual/input). A host Ctrl-C consequently arrives as ordinary C. The application still supports Control modifiers where supplied. Option-C now uses the physical C key code before the OS's accented-character translation; Command-period uses the same interrupt path. At an idle prompt the interrupt records the cancelled input and ^C, clears editing/search state, and returns status 130. During execution it requests cooperative cancellation and prints ^C when control returns.

The available Plus guest is System 6.0.7 with MultiFinder and 4 MB, not 6.0.8 or real hardware. Prior namespace transcript results predate this identity update. Broader final namespace/performance acceptance remains pending.

Guest observations: Finder displays the custom 32-pixel icon, and Clarice launches with its renamed title/banner and a fresh root prompt. Automated Option-C and Command-period presses arrived as ordinary characters; their guest keyboard delivery remains unverified. The user resumed interacting with the emulator, so further automated keyboard actions stopped. Do not treat the shortcut implementation as a verified resolution of the reported keyboard problem.
