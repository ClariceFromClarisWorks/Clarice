# Task 2 manual Plus acceptance checklist

The subsequent [automated acceptance run](guest/ACCEPTANCE.md) completed these
checks. Manual testing is no longer required to close Task 2.

Use the existing `Clarice.dsk` and `Task2.dsk` in this directory; both match their
recorded build hashes. No rebuild is needed. Use Snow at normal speed, Macintosh
Plus, 4 MB, System 6.0.8 and MultiFinder, with a **disposable copy** of the system
disk. Install/run the Clarice application from the Task 2 application disk, and
mount the fixture disk as `/mnt/Task2`. Leave nano/cursesdemo intact.

Run this checklist **instead of** before.sh/after.sh: it adds damaged-installed-file
and explicit Finder metadata checks. Start without `/bin/.clx-grep.old` or
`/bin/.clx-grep.new`; if either exists, stop and inspect it or start with a fresh
copy of the pre-test disk. Do not delete recovery files just to make a test pass.
Stop on unexpected output. Keep each `echo $?` immediately after its operation.

## 1. Independent replacement and metadata

```sh
appinstall --help
hfsattr /bin/grep
hfsattr --type TEST --creator TST2 /bin/grep
hfsattr /bin/grep
appinstall replace /mnt/Task2/grep2.clx grep
echo $?
hfsattr /bin/grep /bin/.clx-grep.old
od -t x1 -N 8 /bin/grep
echo alpha | grep alpha
```

Expect replacement status `0`, plus a retained `.old` notice. Both metadata lines
must show type `TEST`, creator `TST2` and the same flags/lock/visibility as the
pre-replacement line. Ignore the differing filenames. The first eight file bytes
must be `43 4c 58 31 00 00 00 02` (CLX1 format 2); grep prints `alpha`.
The fixture is independently repacked grep with runtime requirements, not a new
upstream grep release. The custom type/creator are intentional test markers on
this disposable disk only; command loading uses content, not these labels.

## 2. Restart survival

Quit Clarice normally and reopen **the same Task 2 build**, then run:

```sh
od -t x1 -N 8 /bin/grep
hfsattr /bin/grep /bin/.clx-grep.old
echo alpha | grep alpha
```

Expect the same format-2 bytes, metadata and `alpha`. A format-1 header means
startup replaced the independent fixture: stop and report it.

## 3. Explicit recovery

```sh
appinstall recover grep
echo $?
od -t x1 -N 8 /bin/grep
hfsattr /bin/grep /bin/.clx-grep.new
echo alpha | grep alpha
mv /bin/.clx-grep.new /tmp/task2-retained-grep.clx
echo $?
```

Expect recovery status `0`; original bundled format-1 header
`43 4c 58 31 00 00 00 01`; both metadata records retain the test markers; grep
prints `alpha`. The updated candidate remains at `.new`. The final explicit move
archives it, clearing that recovery name for the next test; expect status `0`.
If the archive target already exists, stop and choose an unused archive name.

## 4. Damaged installed file survives restart, then explicit repair

This deliberately bypasses appinstall to simulate an already damaged installation.
`appinstall replace` would correctly reject the corrupt fixture.

```sh
cp /bin/grep /tmp/task2-original-grep.clx
cp /mnt/Task2/corrupt.clx /bin/grep
appinstall check /bin/grep
echo $?
od -t x1 /bin/grep
```

Expect `CLX payload checksum mismatch`, status `126`. Keep the complete hex dump
(or a screenshot). Quit/reopen Clarice, then run:

```sh
appinstall check /bin/grep
echo $?
od -t x1 /bin/grep
appinstall repair grep
echo $?
echo alpha | grep alpha
appinstall check /bin/.clx-grep.old
echo $?
```

Before repair: same error, `126`, and identical hex dump. After repair: status
`0`, grep prints `alpha`, and checking retained `.old` still reports checksum
mismatch/`126`. Thus startup preserved the damage and explicit repair retained
that exact previous file. Keep recovery artifacts until results are reviewed.

## 5. Loader breakdown

Run three times, pressing Q to leave each demo, with no external command between
the demo and `status runtime`:

```sh
cursesdemo
status runtime
```

Record each complete Runtime/Loader line as first, repeat 1 and repeat 2. Expect
`load` and `read`, `validate`, `imports`, `relocations`, `prepare` ticks. Small
phases can legitimately be zero at 60 Hz. Phase sums should roughly account for
load; minor bookkeeping/boundary differences are possible. Record the largest
phase rather than guessing a bottleneck. Also note whether load is above 180
ticks (three seconds); this is loader time, not nano launch acceptance.

If cursesdemo is absent, use `ls /bin` immediately followed by `status runtime`
and label these **ls measurements**, not ncurses results.

## Results to return

Send the observed statuses, metadata lines, restart header, whether the damaged
hex dumps match, and all three timing records. Screenshots are sufficient.
No guest result has been obtained in this follow-up. These metadata checks cover
fields exposed by hfsattr; they do not verify extended Finder information, icon
position, or resource-fork bytes. Do not infer those checks passed.
