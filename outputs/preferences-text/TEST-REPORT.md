# Text preferences, migration and native dialog

9 November 2026. **Plus emulator execution**, not physical hardware results.
Baseline: Snow a921b10 with the existing local input patch, Macintosh Plus,
4,194,304 bytes physical RAM, System 6.0.8 and MultiFinder. No ROM or original
system disk was changed. Used the project's disposable work/system608 copy.
The development application retains the 768 KiB partition introduced by the
in-progress awk work; this preference update does not increase that partition.

## Delivered

- `/etc/clarice/clarice.conf`: editable key=value text, at most 2 KiB at load;
  fixed buffers, parsed once at startup, never executed. CR/LF/CRLF accepted.
- `/etc/clarice/clarice.history`: unchanged bounded 128-slot binary history.
- Root-level Clarice Preferences/History migrate using a 512-byte copy buffer,
  close/flush and catalog rename. Existing destinations are not overwritten.
  Failure retains the original. Resource-bearing files are not discarded by
  this private-data migration. Legacy S6P2/S6P3/S6P4 decoding remains available;
  S6P4 was the guest migration fixture.
- Saves prepare a complete new file before replacing the primary. The previous
  committed configuration remains `clarice.conf.bak`; a missing primary loads
  that backup. This is not a guarantee against media damage or unflushed power
  loss. Editing comments/unknown keys is supported on input but they are not
  retained when the dialog/CLI rewrites known settings.
- File > Preferences / Command-comma / `prefs window` exposes font, size,
  monochrome colours, columns, paging, scrollback, rendering cache/renderer and
  reversible fill-screen. The Font menu is absent. Installed fonts use native
  popup tracking, with a System 6 user-item drawing the closed control. The
  dialog was narrowed from 400 to 360 pixels following user feedback; long
  labels are clipped. All controls fit the Plus screen without tabs.
- Known old diagnostic reports move to `/tmp/legacy-*.txt`. New diagnostics,
  pipeline spools and substitution spools use `/tmp`. Spools are removed at
  operation completion/cancellation. Arbitrary user files in `/tmp` are retained;
  crash-orphan cleanup remains unimplemented.

## Checks and evidence

1. **Migration:** initial Monaco 9, light theme, screen paging, 128 lines,
   native renderer and content geometry 347×281 at (4,41) became
   `migrated.conf`. The complete history is identical before/after migration:
   131,592 bytes, SHA-256 in `migration.json`. Up recalled the pre-migration
   command, Down returned to the empty prompt. Old root preferences/history
   disappeared only after successful migration.
2. **Dialog to restart:** user manually selected Geneva from the dropdown;
   user confirmed press/drag/release works. Applied Geneva 12, dark theme and
   30 columns in the dialog. `dialog-saved.conf` records the result. Quit and
   reopened: the font, dark theme and width restored. Monaco was then restored
   at the user's request. Automated mouse dragging is unreliable in this host
   emulator setup, as in the prior scrollbar tests; manual confirmation is
   distinguished from automated evidence.
3. **CLI settings to restart:** ran `tests/preferences/guest.sh`. `cli.conf`
   records Monaco 12, dark, paging 100, 32 lines, C cache, 40 columns (300-pixel
   content width). `fill.conf` records ASM cache, fill=1 and the same normal
   restore geometry. Quit/reopened: the native dialog displayed all those
   values, including fill-screen; screen columns were 69 while filled.
   Turning fill off in the dialog restored the 300-pixel normal window.
4. **Validation:** invalid size, retention, columns and missing font all returned
   status 2 (`guest-results.txt`). Host ASan/UBSan checks in
   `tests/preferences/format.c` passed roundtrip, all fields, whitespace,
   line endings, duplicate keys, invalid/overflow values, embedded NUL and the
   file-size bound (`host-tests.txt`).
5. **Defaults and memory:** returned to Monaco 9, light, 128 lines, screen paging
   and native renderer. `prefs` reported cache off and zero cache bytes. The
   bounded history/scrollback sizes are unchanged; no performance benchmark or
   physical-hardware timing is claimed for this update.
6. **Root/temporary files:** `root-listing.txt` contains directories only.
   `echo prefs-history-ok | cat` succeeded; `tmp-listing.txt` contains no
   `clarice-pipe-*` leftovers. The new command was recalled by Up after the
   following restart, proving continued history writes at the migrated path.
7. **Backup recovery:** on the stopped disposable disk, preserved the primary
   in `restored.conf`, placed `migrated.conf` in the backup and removed the
   primary. Reopened: the original geometry, Monaco 9, light theme, screen
   paging, 128 lines and native renderer appeared in Preferences. Clicked OK
   to recreate the primary without an error. `preferences-plus.png` shows this
   final state. The emulator is left running with those settings.
8. **Build:** the complete pinned offline `scripts/build.sh` passed after fixes,
   including `-m68000`, warnings-as-errors, application/resource packaging.
   `build.log` and `build-sha256.txt` identify the output. Build success is
   separate from the guest checks above.

## Issues caught and corrected

Initial migration returned HFS bdNamErr (-37) when renaming with a full source
path. Original files were retained, verified in the guest. Resolving the volume
and parent catalog ID and passing a leaf name fixed the rename. Ordinary dialog
buttons were replaced with user items so popup tracking can begin while the
mouse is held. The full build also needed the previously unfinished serial
module's missing KillIO glue; it now uses a zeroed PBKillIOSync block. Serial
execution has not been validated here and remains deferred by the new roadmap.

The current development tree includes earlier awk/serial work. This report is
not acceptance of those tools; the extracted awk guest transcript still shows
an integer-printf failure that must be fixed before claiming that port complete.

Native popup API reference: [Apple PopUpMenuSelect documentation](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-137.html).
