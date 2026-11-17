# Practical portable scripts — build 2026111544

## Changes and provenance

A simple-command constructor was skipped after a syntax error, but its shared
cleanup path then set the OOM flag anyway. Cleanup now preserves the existing
error; NULL-child propagation also preserves a diagnosed failure. Real constructor,
array and error-message allocation failures still set sticky OOM. No global masking
of allocation errors was introduced.

Three complete, unchanged upstream scripts are used with ordinary and default or
space-containing-path arguments:

- musl tools/musl-gcc.specs.sh: positional assignment and expanded here-document
  generation. [Upstream blob](https://git.musl-libc.org/cgit/musl/tree/tools/musl-gcc.specs.sh)
  3049257446e113b0fc745f8efeb3e2b101e07798. This tests generated text, not a working
  GCC/Linux toolchain on the Macintosh.
- ncurses 6.5 include/MKncurses_def.sh: default expansion, quoted utility selection,
  input redirection and genuine awk generation, using upstream ncurses_defs.
- ncurses 6.5 include/MKparametrized.sh: positional defaults, cat/awk pipeline and
  table generation, using the full upstream Caps file (414 output entries).

Fixtures retain upstream copyright/licensing; SOURCES.json records hashes. ncurses
source comes from the existing ncurses-6.5 archive already used for Clarice's port.
Scripts were not rewritten or stripped to pass. Their source is in
[tests/shell/practical](../../tests/shell/practical).

The default ncurses parameter-generator path exposed `set - Caps` rejection. It
continued and misleadingly generated a zero-entry table with status zero. Supporting
ordinary `set args` and the `set - args` delimiter alongside `set --` fixes the real
workflow. Unsupported shell options still fail explicitly; no option system added.

Selection boundaries: ncurses MKtermsort.sh/MKkey_defs.sh require traps and Unix
process identifiers, and are not accepted here. MKhashsize.sh uses implementation-
defined echo backslash handling; it was not selected as an unambiguous portable
oracle. No claim is made about arbitrary ncurses/musl build systems.

## Environment audit

awk's 18-entry/164-byte import limits silently omitted exported variables. Its
snapshot now grows a temporary entry buffer and pointer array with the existing
invocation allocator; overflow/allocation failure exits nonzero and runtime teardown
reclaims partial imports. No idle allocation/cache, execution-model change or new
runtime ABI. An updated standalone awk.clx is tested independently of startup
installation; no claim that existing installed utilities update themselves.

Host tests: 30 exports, each 4096 bytes, are all visible (peak 286323 B, remaining=0).
Allocation budgets 64/512/4096/16384 B fail nonzero and clean up. Cancellation during
1000-entry import returns 130 before executing the awk program, remaining=0. Small
three-entry input peak 28813 B includes the genuine upstream interpreter and stdio.

The **unchanged editor** adapter was compiled in isolation with ASan/UBSan: it
imports only 18 of 30 entries; entries of 1500 bytes are all silently omitted;
setenv rejects a 1500-byte value with ENOMEM. Its 24 slots, 18 imports and 1024-byte
entry size are artificial, not HFS/68000 requirements. They remain an explicit
functional gap. TERM=clarice is an intentional terminal-capability override.
The separate 255-byte canonical terminal input line is also an adapter bound;
nano's raw-input path does not use it. Native HFS component/path constraints are
separate genuine platform constraints. No editor binary was rebuilt or accepted
in this batch; its environment importer is the next focused correction.

## Verification

Host evidence: six upstream-script variants exactly match dash status/stdout;
54 previous POSIX cases, 39 focused, 41 elastic cases and the existing basic shell
suite remain passing. Parser sweep and scope-failure reports are retained here.
Seven malformed-syntax cases now report syntax errors, return 2 and recover.
A dedicated ASan/UBSan sweep covers 128 injected allocations across those same
malformed inputs: genuine exhaustion still reports OOM, cleanup is complete and
subsequent parsing succeeds.

Guest results: all six complete upstream variants match dash byte-for-byte, all
seven syntax failures return 2 with correct diagnostics and recover, 30 x 512-byte
exports survive all three native awk runs. Actual Ctrl+C returns 130, skips BAD and
restores stdout/stderr. See guest.json. Native owned peak is 67723 B, remaining=0.

**Whole-application heap stability did not pass.** Three initial readings were
381500,381404,381302 B. Ten repeats also declined. A matched old/new awk control
with the same 30 exports/program lost exactly 384 B over four intervals for both
binaries (~96 B/invocation), with zero tracked command allocations. Old awk omitted
all long exports; new awk retained all. This isolates retention as pre-existing,
not introduced by the change, but its source and long-run bound are unresolved.
Do not call the native heap stable or claim all runtime memory is reclaimed.
The repeat fixture generated the same-length values at runtime to avoid repeatedly
parsing a 15 KiB literal script; source-parsing performance remains a known gap.
See memory-comparison.json. This issue and the unchanged editor import limits are
the next focused work, not grounds for an unmeasured allocator rewrite. Physical SE/System 6.0.8 acceptance remains pending; the SE stayed
off. Snow's disposable fixture reports System 6.0.7, Plus/4 MB, fixed uncapped
FFWD approximately 15–16x. Synthetic command entry may briefly use normal speed.

Reproduce with tests/shell/practical.py, practical-diagnostics.py,
tests/ports/environment-audit.py and tests/parser/diagnostic.c (same ASan/UBSan
flags and fault.o as tests/parser/build.sh). Build the normal application and
scripts/build-awk.sh. practical-prepare.py generates staging commands for the
existing disposable images; never stage while Snow is running. After PRACTICAL_READY,
exercise Ctrl+C and the existing elastic/end.sh restoration/unmount fixture.
practical-guest.py --export generates hfsutils commands; practical-guest.py compares
exported results. All external scripts/data are disposable; no real-SE operation.
