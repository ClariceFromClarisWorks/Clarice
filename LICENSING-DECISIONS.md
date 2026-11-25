# Decisions required for public redistribution

No project licence is granted by this record. The product tree has no
project-wide original-code licence. Existing upstream licences and
notices remain attached to their components. Commit authorship can
help identify contributors for an audit; it does not establish who owns
or may licence a contribution.

## Original project contributions to scope

| Path family | Clarice-authored material to include in a grant or review |
| --- | --- |
| `src/app.c`, `src/shell.c`, `src/sh.inc`, `src/kernels.S`, native filesystem and terminal files | Application, userland execution, memory/UI/terminal handling and 68000 code. Preserve separately credited imports and modifications from mrsh and other upstream code. |
| `src/commands/`, `src/module.inc`, `src/clx-*.inc` and associated headers | Native commands and CLX loader, ABI, import tables and shared services. Vendored networking and parsing libraries retain their own terms. |
| `src/offline*.inc`, `src/update/`, `src/Installer.r` and related Macintosh resources | Installer/uninstaller, Core updates and recovery, resource definitions, and authored adaptations. Keep upstream code and notices identified separately. |
| `src/ports/`, especially `busybox-tools`, runtime, terminal, path and compatibility code | Authored adapters, configuration, help, macros, glue and edits to imported code. Identify each contributor's rights and scope for the exact applets and libraries. |
| `assets/`, authored `scripts/`, `tests/`, `patches/`, `tools/` and documentation included in a deliverable | Original art, build and installation logic, tests, and documents requiring a grant. Upstream fonts, separately licensed helpers, and other vendored files retain their own terms. |

The rights holders must choose the scope and terms of this grant and
confirm contribution permissions for the files and adaptations actually
distributed. This inventory is path-based. It assigns neither ownership
nor a licence to contributors or original files.

## BusyBox CLX and Core linkage

The standard GPLv2 applets are BusyBox `wc`, `tr`, and `sort` (the
`bbwc`, `bbtr`, and `bbsort` build images). Their build links selected
libbb objects, Clarice entry and adapter objects, selected newlib objects
and libgcc. The build packs those relocatable outputs as CLX1 files.
Clarice Core loads and relocates CLX in its own application address space,
resolves imports to live Clarice service entry points, and calls the
command entry point. The standard applets use those services, for
example file access, output, cancellation and invocation memory.
Undefined-symbol inspection of the accepted applet objects confirmed
imports such as `cli_file_open`, `cli_alloc`, `cli_free`, `cli_write`,
`app_yield` and `fprintf` for all three images. Separate files,
independent packaging and lifecycle do not establish
that the programs are legally separate works.

Review the authored linked adapters and what interface and Core material
must be covered or supplied for this distribution. Confirm that any
original-code licence grants and runtime exceptions are compatible with
the BusyBox GPLv2 obligations. Optional BusyBox vi would require a
separate matching review and source distribution. GNU nano and other
optional binaries are outside this kit. See [BusyBox licensing](https://busybox.net/license.html)
and [GPLv2 sections 2–3](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

## Unresolved release gates

- Approve licensing for original Clarice and confirm rights for contributions.
- Resolve exact source provenance and licence scope for linked runtime objects.
- Determine corresponding source scope and access for the GPL applets and
  linked Clarice interfaces and adapters. Do not claim that the mechanically
  staged source set closes those points.
- Keep Apple-driver images excluded unless specific permission for the exact
  driver is independently established.
