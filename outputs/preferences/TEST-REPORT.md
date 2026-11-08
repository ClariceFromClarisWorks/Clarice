# Preferences on the Plus / System 6.0.8

8 November 2026. PASS for the requested Preferences and restart checks.

Fixture: project-local Snow, Macintosh Plus ROM, 4 MB, MultiFinder, System 6.0.8 verified by Finder/media. SysEnvirons reports 0607; see ../system608/TEST-REPORT.md. Native emulator clock, no overclock or fast-forward. Original media and ROMs unchanged.

## Changes

- Recognize Chicago (font ID zero) as an installed font when loading preferences; previously that choice caused all saved settings to be skipped.
- If a saved font is missing, retain the default font while loading the other valid preferences.
- Validate version/record length and the supported 32/64/128 retention values on load.
- Reject malformed numeric text instead of accepting a numeric prefix in the dialog and equivalent commands.
- Hide the terminal caret before opening the modal dialog.
- Clarify the checkbox label as rendering cache. This controls only Clarice, never the OS disk cache.

The existing 48-byte native HFS preference file remains in System Folder:Clarice Data:Clarice Preferences. No extra resident buffer is introduced. The cache's existing QuickDraw fallback supports fonts wider than its small glyph cache.

## Observations

- File > Preferences via its Command-comma shortcut opens the native window.
- Size `12abc` is rejected before applying changes; correcting it to `12` permits saving.
- Chicago 12, inverse colours, cache enabled (29,696 allocated bytes), 64 retained lines apply successfully.
- Quit and reopen: Chicago 12, inverse colours, cache on and 64 lines all retained in the dialog. This specifically exercises font ID zero.
- Unknown installed-font name rejected without applying changes.
- Editing all four settings and choosing Cancel leaves Chicago/12/dark/on/64 unchanged when reopening the dialog.
- Monaco 9/light/off/32 applies; `prefs` reports zero cache bytes after disabling it.
- Second quit/reopen: Monaco 9/light/off/32 all retained. Screenshot saved.
- `scrollbench quick`: PASS (zero failures), including bounded-ring checks and four stable cache release cycles. Thirty-frame timings: native 175 ticks, cached C 231, cached assembly 189; maximum frame 5/7/6 ticks. This short regression check agrees with the earlier full-suite renderer choice; it is not a new full benchmark or physical-hardware measurement.
- Extracted saved preference file: exactly 48 bytes, Monaco 9, light colours, cache off, 128 retained lines; decoded values in saved-settings.json.

## Limits

Restart and dialog checks are guest-observed. Missing-font recovery and legacy S6P2 record handling were reviewed in code, not separately exercised in the guest. No power-loss or disk-full write-failure claim is made. Scrollback capacity persists; its text contents remain session-local. Command history is a separate bounded disk store.

Final restart (including restarting the emulator): `prefs window` opens the same native dialog and confirms Monaco 9, light colours, cache off and 128 retained lines. The guest is left with that window open for review.
