# BusyBox additions for Clarice

These independent, optional CLX packages use genuine BusyBox 1.37.0 source.
They do not change the standard catalog or the approved wc/tr/sort replacements.

Build with `sh scripts/build-busybox-additions.sh`, then validate with
`python3 tests/busybox-additions/packages.py`. Outputs are under
`work/busybox-additions/`. The source archive is pinned by
`prepare-busybox-source.py`; upstream copyright headers remain in staged files.
The upstream licence and AUTHORS are retained in `licenses/busybox`.

`prepare-busybox-additions.py` records all upstream source transformations.
The shared BusyBox bindings and invocation runtime are reused; no core service,
loader ceiling, authentication policy, Unix process model or permission stub is
added. Each image requires runtime 1.10. Invocation allocations are released.

The staged pinned newlib option parser has a narrow lone-dash correction.
The original vendor file is preserved. `tests/busybox-additions/getopt.py` checks
this fix with host ASan/UBSan; the older standard BusyBox build is unchanged.

## Current boundaries

- **diff**: genuine file comparison and unified-diff engine, byte-oriented.
  Directory options are rejected. stdin is spooled through existing invocation
  temporary files because upstream's algorithm seeks. Metadata-only equality is
  disabled: partial Macintosh stat data must never imply identical contents.
- **base64**: upstream codec and command, with Clarice FILE reads. Upstream's
  permissive decoder ignores nonalphabet characters; this is not strict input
  validation. Read/write errors still fail.
- **xxd**: upstream command and shared dump engine, including reverse/plain/C
  output. Input is an owned FILE, not libc freopen on a process descriptor.
  Autoskip is unavailable upstream and rejected here. Output is stdout.
- **patch**: upstream hunk matching with one explicit existing target. Native
  safe-save staging preserves resource fork and Finder metadata. Commit occurs
  only after complete input, successful matching and successful output close.
  The host aborts abandoned replacement tokens on failure/cancellation. No
  multi-file, creation/deletion, permission emulation or missing-newline patches.
  Scanned target lines and patch input reject embedded NULs; scanned target
  lines require a final newline. Dry-run uses a staging file and therefore requires writable storage.
- **gzip/gunzip**: genuine upstream compression/inflate engines. Explicit -c
  file input or stdin/stdout; source files are never removed. Gunzip -t checks
  integrity. The compressor uses BusyBox's SMALL_MEM configuration and genuine
  level selection (1–3 map to 4). No automatic HFS filename replacement.
- **tar**: upstream command and POSIX header reader, listing and stdout
  extraction. No filesystem extraction or creation yet. Exact member matching;
  unsupported extended headers fail. Verbose stdout extraction is rejected so
  filenames cannot contaminate binary output. Octal values must fit the platform's
  unsigned long, and member size its signed file-offset range. No ownership,
  Unix permission, link or device creation. Archives are data-fork interchange,
  not Macintosh backups.

`od` already covers ordinary binary inspection; xxd was selected over adding a
second hexdump command because reverse conversion and C-array output add useful
workflows. No default package promotion is implied by these experimental ports.
