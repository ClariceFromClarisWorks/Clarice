# Task 2 local build and guest checks

**Automated acceptance is now complete:** [results and evidence](guest/ACCEPTANCE.md).
The original preparation instructions below remain available for reproduction.

No publication. Builds and floppy images are local ignored artifacts. The currently
running Snow session was not altered after macOS denied keyboard automation.

For the validation-only follow-up, use the [manual acceptance checklist](MANUAL-CHECKLIST.md)
with the existing images. It replaces the two-script sequence below and adds
installed-corruption and explicit metadata checks. Do not run both workflows on
the same fixture without reviewing retained recovery files.

## Independent commands

Use the same pinned Retro68 container used by `scripts/build.sh`:

```sh
docker run --rm --pull never --platform linux/amd64 --network none \
  -v "$PWD:/src" \
  ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c \
  sh -ec 'for version in 1 2; do
    m68k-apple-macos-gcc -m68000 -Os -ffreestanding -DPROBE_VERSION=$version \
      -c /src/tests/commands/update-probe.c -o /src/work/task2/probe$version.o
  done'
python3 scripts/pack-command.py work/task2/probe1.o work/task2/probe1.clx
python3 scripts/pack-command.py work/task2/probe2.o work/task2/probe2.clx --runtime 1.0
```

Neither command invokes the terminal build. For an actual updated grep image,
repack its independently built object with `--runtime 1.0`; production updates
would compile the desired source version first. Task 2's grep fixture changes the
requirement-bearing format, not upstream source/version.

## Local artifacts

- `outputs/task2/Clarice.bin`: MacBinary application, compiled for 68000.
- `outputs/task2/Clarice.dsk`: 800 KiB application floppy image.
- `outputs/task2/Task2.dsk`: separate 800 KiB fixture floppy (volume `Task2`).
  Contains `before.sh`, `after.sh`, two independent probes, versioned grep,
  incompatible-runtime and deliberately corrupt images. No OS or ROM included.

Use a disposable copy of the Plus 4 MB / System 6.0.8 test disk. Install this
Clarice build before format-2 commands; an older terminal's startup remains unsafe.
Keep existing `nano`/`cursesdemo` files intact. Do not modify a mounted image from
host tools. Test both fresh `/bin` installation on a separate clean fixture and
restart with installed commands; host tests already cover those branches, but
there is no guest evidence yet.

Mount Task2 and run:

```sh
sh /mnt/Task2/before.sh
```

Quit and reopen Clarice, then run:

```sh
sh /mnt/Task2/after.sh
```

Expected `/tmp/task2-results`:

- check/install/replace/grep-update/recover/repair status files contain `0`.
- incompatible/corrupt replacement status files contain `1` plus diagnostic text.
- `probe1.txt` and `recovered.txt`: `independent command v1`.
- `probe2.txt` and `restarted.txt`: `independent command v2`.
- `grep-after-restart.clx` byte-matches the fixture `grep2.clx`.
- grep/repaired grep print `alpha`; sed/awk print `beta`; ls/df complete.
- Existing preferences/history remain usable; prior versions retain forks and
  Finder metadata. Do not run scripts twice without explicitly reviewing the
  retained `.old`/`.new` files. Their refusal to overwrite is intentional.

Check actual failed writes on a separately prepared full/read-only fixture volume,
and locked destination behaviour; keep the real system disk untouched. Hard-stop
experiments require disposable disk copies and must not be mistaken for proof
against arbitrary physical media failure. Review recovery names after interruption.

For loader measurements, run the **unchanged** `cursesdemo`, press Q, then capture
`status runtime`. Repeat twice, label first/subsequent launches, and record Snow
normal-speed Plus/4 MB/System 6.0.8 configuration. `load` includes command loading
through readiness to call entry; phase counters do not measure curses startup or
first keyboard-responsive rendering. Preserve those separate end-to-end goals.
