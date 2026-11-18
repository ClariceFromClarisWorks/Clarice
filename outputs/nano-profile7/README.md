# Nano line-buffer growth — isolated Snow experiment, 18 November 2026

Worktree `Clarice-Nano`, branch `codex/nano-startup-20261118`; based on 7e3e036
plus a copied working-source snapshot of the preceding serial/nano work. Original
checkout, other agents' Snow disks/repository, physical SE and release candidate
were not modified. No push. Dedicated Snow disks were copied from stopped readback
fixtures, not from the other running emulator's mounted media.

## Retained change

Pinned nano 9.2 `read_file` grows its temporary line buffer by 120 bytes. Clarice's
invocation allocator implements realloc by allocating/copying, so long lines
produce quadratic copying. Use the geometric-growth strategy already present in
nano's bundled GNU gnulib `lib/malloc/dynarray_emplace_enlarge.c` (1.5x), with the
original +120 request as fallback when the larger allocation fails. Guard overflow;
retain nano's normal allocation-failure path. Initial 120-byte allocation,
upstream parsing, file formats, rendering, ncurses and safe-save code are unchanged.

The adaptation is isolated in `scripts/patch-nano-growth.py`, invoked by the
existing optional nano build. Upstream source/licences remain intact. No resident
cache, extra runtime imports, Core change or permanent allocation.

## Matched results

Plus/4 MiB/System 6.0.8, Core 2026111823/runtime 1.10, maximum/uncapped Snow mode.
Concurrent host work causes acceleration to vary; figures below are guest ticks,
not host elapsed time or physical-SE estimates. Three warm runs per variant,
same command name, fixtures, Core and window. No new cold-start claim.

| Input | Before ticks | After ticks | Mean change | Owned peak before → after |
|---|---|---|---|---|
| 8 KiB / 128 short lines | 402,401,400 | 400,401,402 | 401→401 | 101736→101736 B |
| 8 KiB / two long lines | 324,323,323 | 312,310,310 | 323.3→310.7 (3.9%) | 95662→95472 B |
| 16 KiB / one line, embedded NUL | 453,455,455 | 360,348,348 | 454.3→352 (22.5%) | 116066→120416 B |

16 KiB read-loop ticks: 185/187/187→82/81/80. An earlier 16 KiB baseline overlapped
an update (missing first result, incomplete second); excluded and retained as
`sixteen-baseline-attempt.json`. The clean repeated baseline is
`sixteen-baseline.json`. `acceptance.json` contains candidate 16 KiB results;
`same-name.json` contains the 8 KiB runs and explicitly null timing for failed
64 KiB launches. `images.json` identifies both diagnostic executables.

For a 4095-byte line, growth operations fall 34→8 and requested cumulative copying
71400→7885 bytes. A 65535-byte line's theoretical growth copying falls
17919720→140759 bytes, but this is NOT a successful guest workload: both versions
exhaust the current application partition before opening it. Candidate returns
status 1 with zero retained allocations and subsequent commands work. Installed
4 MiB RAM is not all available to the current application's partition.

Diagnostic package 269420→269528 (+108 B), loaded image 199176→199260 (+84 B).
Normal package 268900→269016 (+116 B). No idle memory increase. The 16 KiB extra
4350 B is transient; short-file peak is unchanged. Every collected run reports
remaining=0; free heap remains around 364,000 bytes without progressive loss.

## Validation

- ASan/UBSan host test executes the actual generated growth block with forced
  moving allocations, exact-size fallback, failure preserving original contents,
  32-bit size overflow and 64 KiB binary-pattern lines. Patch idempotence and
  unexpected-upstream-source rejection also pass (`tests/ports/nano-growth.py`).
- Snow candidate: 16 KiB embedded-NUL file saved byte-for-byte (`cmp`), TEXT/ttxt
  metadata preserved, save cleanup remaining=0; Command-period cancellation 130
  and genuine partition exhaustion 1 both recover with no retained allocations.
- Normal optional nano compiled and installed through `clarice update`, using a
  separate private test repository and new manifest versions. Normal build 2026111853 passes exact 16 KiB embedded-NUL save, TEXT/ttxt
  metadata, zero retained allocations and subsequent shell commands; see
  `normal-acceptance.json`. Diagnostic candidate final revision is 2026111855.
- Prior safe-replacement fork/failure evidence remains applicable; this changes
  only read-buffer capacity. No renderer change or new physical acceptance.

Core bootstrap needed a Core-only repository pass because the stopped fixture
had runtime 1.6 and rejected runtime-1.10 nano before the Core update. This was
isolated fixture preparation, not an updater change. Both recovery copies and
completed installation state were retained.

The three-second target remains unmet. This removes avoidable long-line copying;
ordinary-file startup still spends roughly 100 ticks in loading, ~80 in initial
refresh, and the remainder in file construction, setup/layout and native allocation.
Physical comparison and broader application-partition sizing remain separate work.

## Development checkpoint validation

Pre-commit rerun: nano growth and buffered-getc ASan/UBSan tests, native-path
metadata-reuse test, and remote session/authentication/TCP/serial host suite pass.
The TCP host fixture needed a binary-length fix: strlen had dropped the zero-byte
BINARY option from its test packet. It now explicitly feeds all three bytes; the
production implementation was unchanged. Prior Snow evidence was not rerun for
this test-only correction. Unfinished updater-recovery experiments and generated
build logs remain outside this checkpoint.
