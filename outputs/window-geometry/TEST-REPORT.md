# Window geometry — Plus / System 6.0.8

Built with the existing pinned Retro68 toolchain and executed in the 4 MB Macintosh Plus emulator under MultiFinder.

- Launch with existing S6P3 preferences: retained previous font/settings and dimensions; frame appeared two pixels from the left and two pixels below the menu bar.
- Fixed text left inset is four pixels, independent of font metrics.
- Changed width to 60 columns, used fill to change height and position, then restored width to 60 columns. Quit and relaunched Clarice: custom width and height returned, and the saved horizontal position returned. The vertical frame was adjusted one pixel to retain the two-pixel desktop inset.
- S6P4 is 52 bytes, adding signed content-origin coordinates to the previous preferences. Moves and resizes write preferences after completion, not during tracking or terminal rendering.
- Automated title-bar dragging did not move the guest window; manual drag persistence has not been verified in this test. The drag event handler saves geometry after DragWindow returns.
- These are emulator observations, not measurements on physical hardware. No claim of completed external utility migration or upstream grep port is made by this UI update.
