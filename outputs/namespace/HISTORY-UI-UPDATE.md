# History and bottom-bar update

Removed the permanent bottom hint/status row from native and cached renderers, reclaimed one text row, and removed its invisible click-to-cancel region. Paging alone displays a temporary More line. File > Cancel command now uses the same interrupt path as Ctrl-C, including clearing pending input at an idle prompt.

The pinned 68000 build passes. In the 4 MB Plus / MultiFinder / System 6.0.7 guest, visually verified the absence of the bar with cache off and on; restored cache off. No hardware results claimed.

Up/Down were already mapped to bounded disk-backed history (character codes 30/31); Down past the newest record restores the saved draft. Actual arrow input in Snow's Plus guest produced no recall. Source inspection of twvd/snow revision ce1b3c6baf7f0c9d01c6a8b71928156c7511e7bc, core/src/keymap/akm0110.rs, shows the Plus M0110 mapping omits universal Up 0x3E, Down 0x3D and Control 0x36/0x7D, returning None. This inspected revision is not asserted to match the installed emulator binary. It corroborates observed input failure and the documented keyboard model.

Ctrl-C remains unresolved end to end. Do not claim arrow/history keyboard verification, menu interruption verification, or real Plus keyboard compatibility from these tests. Menu history and cancellation routes are available in the application, but attempted automated menu actions did not produce a visible selection in this session. The emulator source checkout is research only; no emulator binaries, original media or global input settings were modified.
