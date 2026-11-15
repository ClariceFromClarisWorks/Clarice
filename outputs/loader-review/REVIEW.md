# Native command loading review

14 November 2026. Review only; no loader/runtime changes, application rebuild,
publication or hardware test. The installation-verified CLX work is preserved but
its pending guest acceptance is paused, not declared complete. Existing Snow is
left untouched. This review incorporates the native-command/Finder requirements.

## Recommendation

Prefer a directly linked native APPL worker as the **candidate** MultiFinder and
System 7 backend. Keep CLX working until a measured worker proves the complete
command contract. Retain in-process execution for single Finder; evaluate a
purpose-built callable resource there, rather than trying to execute an arbitrary
APPL segment inside Clarice. Share application source and portable services,
not necessarily one binary or cross-application pointers.

Native loading can remove Clarice's symbol-name lookup and custom image loader
from the worker path. It does not supply Unix streams, argument vectors, shell
exit statuses, cancellation or Clarice's filesystem namespace. Those services,
not launching an APPL, are the substantial implementation cost. A worker that
merely loads CLX internally keeps most present loader costs and adds launch/IPC
overhead; it is not the preferred performance experiment.

## Comparison

| Concern | Current CLX in Clarice | Native APPL under MultiFinder/System 7 | Callable resource under single Finder |
| --- | --- | --- | --- |
| Loading | Read image and metadata; integrity/structure checks, imports, relocation, C entry | OS launch/resource loading plus toolchain startup; statically resolve application dependencies where possible | Resource Manager reads a handle; callable entry and runtime setup still require a deliberate format/ABI |
| Memory | Clarice's 768 KiB partition, shared stack/services; temporary image and relocation workspace; demand-driven working allocations | Separate SIZE-selected partition, stack and globals; added libc/runtime and transport memory; partition admission may fail with 4 MB | Same Clarice partition; lock executable handle during use, allocate private globals/workspace and release at return |
| Arguments/cwd/environment | Existing argc/argv, shell context and runtime adapter | Versioned serialized invocation; validate/copy into worker-owned state before application entry | Versioned in-process invocation structure; retain caller state and define ownership |
| stdin/out/err | Existing descriptor adapters, redirections and finite HFS-spool pipelines | Worker-owned FILE/descriptor wrappers over bounded endpoints; redirected files/spools can be an initial binding; terminal streams require transport | Same portable interface bound to local services; no IPC required |
| Completion | Return/port_exit and invocation cleanup; shell receives status | Explicit invocation-ID/status acknowledgement; worker exit, lost-worker detection and terminal restoration; native termination alone is not a Unix exit status | Return through wrapper; release resources and restore resource file, globals and terminal state |
| Cancellation | Existing cooperative polling/runtime unwind; not preemption of arbitrary C | Clarice sends cancellation, worker polls/yields and unwinds; no assumption that forced termination is safe during saving | Existing cooperative cancellation with wrapper cleanup; cannot safely purge executing code |
| Dependencies | Private imports include libc, host globals and terminal/HFS functions | Link compatible libc/ncurses/runtime locally; OS traps native; terminal/control binding replaces private host imports | Explicit service table or linked dependencies; A5/global-data and callback conventions must be correct |
| Cost/risk | Lowest migration cost, functioning nano/safe-save; same-process faults can damage Clarice | Highest integration/testing cost; partition separation but no hardware memory protection on Plus; potentially lower loader CPU cost | Moderate-to-high ABI/toolchain work; resource container alone provides little benefit and no process isolation |

Native sublaunch is supported under System 6 MultiFinder. Ordinary single-Finder
launch replaces the caller, so it is unsuitable for retaining Clarice as a live
terminal. System 7 launch APIs/Apple Events must not be assumed on System 6.
[Apple TN126 example](https://ioi-xd.net/os9docs/pdf/launching_another_application_under_system_6.pdf).

SIZE specifies minimum/preferred partition requirements, not an automatically
growing Unix address space. Application-death notification options differ by OS;
use explicit completion rather than relying on a System 7 notification on 6.0.8.
[Apple SIZE documentation](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-87.html).

## What CODE loading does and does not replace

The native Segment Manager works with an application's jump table and A5 world;
it loads/locks segments and patches jump-table entries. Merely getting another
application's CODE resource does not establish its globals, startup conventions,
dependencies or a callable command entry. Calling its APPL startup could also
exit the host instead of returning. A standalone callable resource needs a
documented entry/service contract, initialized private state and safe return.
[Apple jump-table description](https://dev.os9.ca/techpubs/mac/runtimehtml/RTArch-118.html).

Resource handles can move or be purged when unlocked; executing code must remain
locked. Segmentation can defer seldom-used code but does not automatically reduce
the first interactive nano working set. Classic PC-relative segment constraints
also depend on compiler/linker conventions.
[Apple Segment Manager](https://dev.os9.ca/techpubs/mac/pdf/Processes/Segment_Manager.pdf).

The **pinned local Retro68 toolchain**, not just historical documentation, was
inspected. `libretrocrt.a` contains `_start -> Retro68Relocate`,
`Retro68ApplyRelocations`, constructors/destructors and optional multisegment
support with LoadSeg glue. See [symbol/disassembly evidence](toolchain.txt).
Thus native APPL does not mean zero relocation. Clarice already uses
`add_application` with `--mac-single`; the command packer separately emits CLX.
Toolchain packaging supports native applications and code-resource output, but
does not supply Clarice's command interface.
[Retro68 project](https://github.com/autc04/Retro68).

## Measured evidence and limits

These are **existing completed Snow measurements**, not new native benchmarks:
Plus/68000, 4 MB, System 6.0.8. Guest ticks are 1/60 second. Diagnostic nano build;
dispatch to first painted input wait, with actual typing subsequently confirmed.
Source: [nano startup report and raw records](../nano-startup/REPORT.md).

| Sample | Total ready | Loader | Read | Validation | Imports | Relocations | Preparation |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Cold empty | 820 ticks / 13.67 s | 555 | 63 | 331 | 17 | 22 | 122 |
| Repeat empty | 669 / 11.15 s | 432 | 63 | 329 | 17 | 22 | 1 |
| Repeat 8 KiB | 844 / 14.07 s | 433 | 64 | 330 | 17 | 22 | 0 |
| Cold 8 KiB | 1006 / 16.77 s | 555 | 63 | 331 | 17 | 22 | 122 |

Even subtracting the **entire** loader gives 4.42/3.95 seconds cold/repeat empty
and 7.52/6.85 seconds cold/repeat 8 KiB. This arithmetic is a counterfactual,
not a native prediction: a different backend changes allocation/layout and adds
its own startup/transport costs. It demonstrates that removing this measured
loader interval alone cannot produce a three-second editor.

Full validation is the largest measured loader stage (~5.5 seconds), whereas
imports plus relocation cost ~0.65 seconds. The pending installation-verified
policy must be measured before comparing its remaining cost with native loading;
using only old full-checksum figures would bias the architecture decision.

New read-only image inspection ([raw sizes](image-sizes.json)): normal nano is
198,288 image bytes plus 69,424 bytes of import/relocation metadata, with 5,380
relocations and 76 imports (267,744 bytes including header). The diagnostic image
has 198,488 + 69,572 bytes. Metadata is temporary load workspace, released before
entry. A directly linked APPL could avoid this particular CLX representation,
but its own runtime/relocation size has not been measured.

Existing normal runs report 105,172 bytes portable allocation peak for 8 KiB,
zero tracked remainder, and stable post-exit free heap 398,284/largest 393,798.
These are not whole-process peak figures. Native-worker partition, CODE-backend
peak and IPC memory are **unmeasured**, not assumed smaller. Current source uses
demand-driven allocation with a 32 KiB reserve; older architecture documents'
96 KiB-ceiling statement is historical.

No matched native-versus-CLX guest benchmark was executed in this review. There
is no existing native command wrapper or invocation transport to compare with
the real CLX command path. Timing a Finder-launched hello-world or resource read
alone would omit required command setup and completion, and cannot establish a
native command or nano speedup. The concrete experiment below is still needed;
this recommendation is architectural, not a measured native-performance claim.

## Finder safety and independent updates

Every native command should include its own minimal Toolbox startup, explanation
dialog and clean exit, with no dependency on a running Clarice. Suggested text:
“This is a Clarice command. Run it from Clarice.” No automatic launch is required.

Before calling upstream main, validate the invocation's magic/length, runtime
version/capabilities, intended command identity, fresh invocation identifier,
bounded arguments/environment/cwd and endpoint descriptions, followed by a live
controller handshake. Missing, stale, malformed or incompatible context must
not execute the command or touch redirected targets. Distinguish a valid context
that negotiates an error from a direct Finder launch. Never dereference a raw
pointer supplied in an untrusted launch record. This is misuse/staleness
protection, not a security boundary on System 6. The transport remains undecided.

Keep Task 2's absent-only installation, explicit replacement, previous-version
recovery and runtime compatibility policy for both formats. Native APPL packages
must preserve the resource fork (CODE, SIZE, dialog and compatibility resources),
data fork and Finder metadata. Current CLX validation/receipts cannot simply be
relabelled to verify APPLs: define a versioned native manifest and verification
coverage for both forks before enabling that backend. Do not silently migrate
existing independent commands or remove CLX support. Native loading supplies no
replacement for executable-integrity policy.

## Smallest decision experiment, not implemented

Build one common command body that echoes arguments, copies bounded binary input,
emits distinct stderr, returns a chosen status and polls cancellation. Package it
as actual CLX, directly linked APPL, and callable resource. Use a disposable copy
of the same Snow disk/configuration. Measure guest TickCount at dispatch, entry,
first response and completion; cold reboot and at least three repeat samples.
Include full verification and installed-fast CLX separately. Capture image size,
whole partition, peak working/transport memory and post-exit cleanup. A minimal
mailbox can establish launch/completion, but its timing must not stand in for a
future interactive transport. Test Finder launch with Clarice absent, stale
context, cancellation, errors, and installed independent updates.

Only then choose the native binding/transport and consider a nano packaging
experiment using the same genuine nano/ncurses and safe-save implementation.
Linking a tiny probe is straightforward; implementing and qualifying the command
context, streams and lifecycle is the substantial new work. No loader replacement
or worker implementation was undertaken in this review.
