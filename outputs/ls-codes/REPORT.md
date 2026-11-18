# Long listing file codes and optional legend

User authorised normal long-list type/creator codes, an optional field legend,
development tests, measured cost, commit and push to the development branch.
Baseline is dd96ea8. This changes the existing listing formatter/options only;
there is no new catalog service, parser dependency, directory scan or cache.

## Behaviour

`ls -l`, `ls -la` and `ls -lah` now show stored four-character type and creator
codes between fork sizes and creation time. The existing `--full` layout remains
unchanged and adds N/B flags and raw hexadecimal Finder flags to the normal
long layout. Nonprintable code bytes display as dots; directory codes are
`---- ----`. Codes are metadata, not inferred from filenames or extensions.

`--legend` implies `-l`. It prints explanations once, then an aligned column
heading for each operand listing. DATA and RESOURCE are the two logical fork
sizes; TYPE and CREA are type and creator. Explanations adapt to `-h` and
`--full`. It is explicitly opt-in, including when stdout is redirected or used
in a script. Without it, there are no added headers. `--` still ends option
parsing; invalid options return 2 on stderr before any catalog work. Help
accurately describes the new layout and option.

## Cost and implementation

Both fields are already returned by the existing PBGetCatInfo query. The change
runs the existing eight-byte sanitization loop for every long row and appends
ten output bytes (four type, one separator, four creator, one separator).
Plain and --full output are unchanged. A 1000-entry listing with . and .. writes
10020 additional bytes. No extra native query, heap allocation, resident Core
storage or directory pre-scan. Existing 4 KiB optional output buffer and allocation
fallback remain unchanged; wider lines can wrap earlier on narrow terminals.

Native 68000 build, including the optional legend and revised help: package
13176 → 14024 bytes (+848); loaded multicall utility image 8480 → 9196 (+716);
transient loader metadata 4648 → 4780 (+132). No added imports. The list_item
stack frame stays 748 bytes; clarice_main frame grows 1192 → 1196 (+4). Saved
register sets are unchanged. These are compiler frames, not a measured total
stack high-water mark. See packages.json and stack-frames.json.

## Snow measurements

Snow 1.5.0, Plus/68000, 4 MB, System 6.0.8, Core 2026111882,
normal video pacing. Complete shell `time` measurements include utility loading,
formatting, catalog work and output. Four runs per variant; one repeated pair
reverses order. The first run is reported separately, not assumed a cold disk.

| Case | First before/after | Repeated median before/after |
| --- | --- | --- |
| 100 entries, -lah redirected | 2.46 / 2.65 s | 2.21 / 2.26 s (+0.05 s) |
| 1000 entries, -la redirected | 8.25 / 9.30 s | 9.35 / 9.10 s (-0.25 s) |
| Small terminal -lah | 1.61 / 1.51 s | 1.65 / 1.68 s (+0.03 s) |

Small terminal fixture has four children (including a hidden name and directory),
plus . and ..: six displayed rows. `screen:5` in the raw log is its scenario
identifier, not an entry count. The 1000-entry repeat ranges overlap (baseline
8.78–9.35 s, candidate 9.08–9.65 s); do not interpret its median as a speedup.
These results show a small cost in the smaller cases and no reliable larger-case
change at this sample size, not proof of zero overhead. Physical SE measurements
remain the final performance authority.

All 24 timed commands return 0 and runtime-owned remaining allocations are zero.
Final paired terminal largest-free-block values are both 332444 bytes; heap values
are within 2 bytes. Comparisons across terminal/file phases also include shell
scrollback/session changes and are not utility peak-memory measurements.

Persisted long output adds exactly 1020/10020 bytes for 100/1000 entries plus dots;
all earlier columns match when only the new ten-byte field is removed. Plain and
--full output comparisons pass unchanged. Guest legends show actual SITx/SITx
and CLX1/CLRC metadata with aligned headings. Invalid options return 2 on stderr
with empty stdout. The human legend adds 347 bytes/six lines before the listing;
full legend adds 417 bytes/seven lines. Legend-only elapsed overhead was not
separately timed. [Summary](summary.txt), [raw results](results.txt),
[measurements](measurements.json), [human legend](legend.txt), [full legend](full.txt).

## Tests and reproduction

- 4000 cached/cold formatting cases against an independent printf oracle:
  full 32-bit sizes, human/full modes, dates, directories, virtual entries,
  flags, filenames, and explicit SITx/SITx plus nonprintable CLX code cases.
- 576 output/status comparisons retain the earlier buffering oracle. For normal
  long output only, the newly specified ten-byte field is removed before comparing
  all other bytes. Plain and --full output remain byte-identical. Includes
  allocation fallback, cancellation, failed writes, catalog errors and cleanup.
- Real catalog loop and multi-operand regressions pass. Focused legend parser
  tests cover -l/-la/-lah, implied -l, --full, human units, multiple operands,
  --, stdout/stderr, errors before listing, help and explicit redirected legend.
- Focused native 68000 compilation/packing passes with warnings as errors;
  existing production Core is reused in the guest.

Run tests/commands/listing-format.py, listing-host.py, listing-operands.py,
tests/ls-output/host.py and tests/ls-codes/legend.py with Python 3. Build focused
before/after packages with `sh tests/ls-codes/build.sh`. The prepare.py script
clones the earlier disposable file-progress media; it must not run while those
new disks are open for writing. Boot work/ls-codes/system.hda and bench.hda with
the existing Snow Plus ROM/4 MB configuration and launch Clarice. Its profile
runs guest.sh. After LS-CODES-DONE and closing Snow, extract.py exports the
results and check.py validates/measures them. This run uses the existing small
Snow-core test frontend documented in ../file-progress/resource/REPORT.md,
with video pacing and no networking. It changes no emulation or shipped code.

Physical SE and System 7/MultiFinder acceptance remain separate. There is no
hardware deployment in this task. The next hardware build should check the
long-list columns, optional legend and timing/line wrapping on the actual SE.
