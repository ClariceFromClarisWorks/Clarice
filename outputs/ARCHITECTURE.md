# Clarice architecture — reconciled direction

10 November 2026. This document supersedes the historical staging and fixed-budget
policy in the [earlier review](history/ARCHITECTURE-through-2026-11-09.md).
The [revised brief](../references/engineering-direction-2026-11-10.md) is authoritative.
Use [current state](CURRENT-STATE.md) for evidence, [capabilities](PORTABILITY.md)
for API gaps and [roadmap](ROADMAP.md) for order. Task 1 authorises no implementation.

## Identity and baseline

Clarice is a native Macintosh command-line environment: shell, base userland,
portable-application runtime and compatibility services. Macintosh OS retains
hardware and system-service responsibility. Baseline: Plus, 8 MHz 68000, 4 MB,
System 6.0.8; preserve single Finder, MultiFinder and SE/later 68k paths.
Normal movable terminal, installed Monaco by default, persistent preferences,
HFS-backed `Clarice Data`, `/mnt`, `/etc/clarice` and owned `/tmp` work remain.
No simulated root/users/permissions, Unix kernel or unsupported compliance claim.

## Existing implementation, not the destination

`src/module.inc` loads fresh CLX1 code/data/BSS, validates checksum/imports and
relocations, calls a C entry point, then releases the image. Ordinary utilities
have actual implementations outside shell dispatch; some are multicall images.
They still execute synchronously inside Clarice. Host globals and private
function/data imports prevent treating this as an isolated process ABI.

The current app requests minimum and preferred 768 KiB, reserves 64 KiB of stack,
and port allocations are capped at 96 KiB. These limits are facts to measure and
replace deliberately, not the new policy. Separate code, working allocations,
stack, relocation workspace, shared FILE records and display buffers in reports.

Finite shell pipelines are HFS spools. The actual ncurses probe uses on-demand
screen storage and native buffered row painting; nano has linked but lacks an
observed successful guest result. Do not discard these implementations or call
them completed applications. [COMMAND-ABI.md](COMMAND-ABI.md) specifies current CLX1.

## Intended execution architecture

Use one upstream application implementation with two runtime bindings:

- Single Finder: retained in-process loading, with invocation-owned state and
  bounded stacks/resources. No attempt to launch away from the shell.
- MultiFinder/System 7: a real Macintosh worker application with its own partition
  and startup/cleanup, retaining Clarice as terminal/controller. The same portable
  APIs bind to native local services and a terminal/control transport. Do not
  pass current host function pointers or FILE pointers across partitions.

A small reusable worker envelope loading a selected CLX payload is a candidate,
not a settled transport or launch implementation. A direct-linked APPL may be
faster; compare before choosing. System 6 MultiFinder launch/control must be
verified separately from System 7's Process Manager/Apple Events. OS memory
partitions are not modern hardware-enforced memory protection. No permanent
worker pool or resident editor is assumed. [Feasibility analysis](FEASIBILITY.md)
records the source basis and minimum experiments.

## Invocation and memory contract

Evolve toward an explicitly versioned invocation context: arguments, private
cwd/environment, standard endpoint mappings, error/cancellation state, allocation
ownership, completion and negotiated services. Keep module globals fresh while
isolating shared libc/host state. Reusable APIs should use fixed-width types,
opaque descriptors and copy-in/out buffers, not native host structs as public ABI.

Admission should consider task minimum/preferred working set, image and relocation
peak, stack, available heap and largest block, plus measured recovery/UI reserves.
Allocate on demand and fail meaningfully when the task cannot fit. MultiFinder
workers can take a justified larger partition while the idle shell remains small;
single-Finder admission remains constrained by its available application memory.
Do not equate physical 4 MB, free system memory, free app heap and contiguous space.

## Streams, terminal and portability

Preserve current spool pipelines until bounded live streams are proven. Live
execution needs resumable producer/consumer contexts, independent stacks or worker
processes, backpressure, endpoint ownership/EOF and cancellation. Yielding to
Macintosh events alone does not schedule another in-process C command. Recursive
call chains are insufficient for general mutually blocking pipelines.

Retain real upstream ncurses and truthful terminfo; optimise transport and native
dirty-row drawing rather than replace curses with an imitation. Generalise only
services exercised by ports. Audit accepted-but-unperformed terminal settings,
approximate stat fields and restricted fcntl before exposing them as stable APIs.
Locale beyond the C/byte baseline is not an immediate priority.

For each port retain upstream source/version, licences/credits, supported build
configuration, isolated patches and behaviour gaps. Cross-compilation is enough
for independent software updates; native compilation is a separate future goal.

## Acceptance and sequencing

Nano must reach fully drawn, keyboard-responsive editing within three seconds
from submission for empty and ordinary 8 KiB documents; first and repeat launches
are separate tests. This remains unverified. Existing ncurses loader timings
already exceed three seconds; profile file I/O, checksum, imports/relocations,
initialisation and first paint before selecting an optimisation.

First fix update-safe installation and define runtime compatibility, then isolate
invocations and strengthen file services before worker/live-stream work. No
subsequent step, push or release is authorised by this reconciliation.
