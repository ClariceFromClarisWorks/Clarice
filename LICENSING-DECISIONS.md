# Clarice 0.5.0 licensing decisions

## Adopted decision — 26 November 2026

The user explicitly authorised **GPL-2.0-only** for original Clarice code
and other original project material where they have authority. This replaces
the unissued MIT/Expat draft; it is an effective grant within that scope,
not a pending choice of licence. The grant is in
[the project notice](licenses/Clarice-GPL-2.0-only.txt), with the complete
GPLv2 terms in [LICENSE](LICENSE).

Use the project credit **Clarice** without repeating the user's personal
name. This credit is not an invented copyright owner or a rights assignment.
No required existing copyright notice is removed. Git authorship and
historical provenance are preserved; neither establishes ownership alone.

## Grant scope and third-party boundaries

This inventory covers accepted product `f571f7c`. The grant applies to the
owner's original contributions in these areas, including mixed files only
to the extent of their authority. It does not assert ownership of imports.
Unverified third-party permissions remain open rather than being covered by
a blanket project licence.

| Scope | Original contributions covered by the grant |
| --- | --- |
| Shell, terminal and native runtime | `src/sh.inc` expressly identifies itself as the original Clarice execution adapter. The candidate scope also includes the authored portions of `src/app.c`, `src/shell.c`, `src/shell-*.inc`, `src/kernels.S`, `src/namespace.inc`, `src/streams.inc`, and other project terminal, filesystem and UI code. The mrsh parser and AST keep their existing MIT attribution. |
| Native utilities, networking and CLX | Authored portions of `src/commands/`, `src/module.inc`, `src/clx-*.inc`, and the service and session interfaces. Vendored libraries, copied declarations or implementations, and upstream-derived portions require their own treatment. |
| Installer, updater, recovery and art | Authored portions of `src/Installer.r`, `src/installer-ui.inc`, `src/offline-*.inc`, `src/self-update.inc`, and non-vendor files in `src/update/`. `src/Icons.r` expressly identifies the original Clarice icon artwork and its generator, `scripts/make-icon.py`. |
| Build scripts, adapters and documentation | Authored build, packaging, test and documentation material in the companion. Review `scripts/prepare-*.py`, `src/ports/` and port patches at the contribution level, especially any generated code and snippets copied from upstream. |

The following third-party material retains its existing terms. The grant
covers only separable original contributions within the owner's authority:

- Upstream trees and adaptations in `src/vendor/`, `src/commands/vendor/`,
  `src/commands/remote/vendor/`, `src/update/vendor/` and the port
  subtrees. This includes BusyBox and sbase edits, musl regex and fnmatch,
  mrsh parser changes, One True Awk, E-Kermit, and the BusyBox vi port and optional nano.
  Keep upstream rights and notices intact. Review any separately licensable
  Clarice contributions in mixed files before naming them in the grant.
- The JetBrains Mono-derived bitmap material in `assets/font/`, runtime
  source archives, external test material and upstream tool patches or
  helpers. Keep their original notices. Confirm provenance and permission
  for any remaining externally sourced contribution before redistribution.

No blanket per-file SPDX rewrite is made: it would obscure mixed-file
provenance. The project-level scope notice applies to original contributions;
upstream headers and licences remain intact.

## BusyBox CLX and Core

Standard `wc`, `tr`, `sort` and `vi` are distributed as BusyBox GPL-2.0-only
CLX builds. Each is a relocatable link of the selected
BusyBox applet/libbb code, Clarice entry and port objects, library objects
and libgcc as selected by the linker. Clarice Core loads and relocates CLX
in its own address space and calls its entry point. Undefined-symbol
inspection of the accepted applets found imports such as `cli_file_open`,
`cli_alloc`, `cli_free`, `cli_write`, `app_yield` and `fprintf`. These
are live in-process service calls for files, allocation, output and
cancellation. Separate files and independent installation establish how
the kit is packaged. The original Core and adapter contributions are now offered under GPLv2
only as well; no separate-file exemption is relied upon.

The release source companion includes the whole accepted Core source,
interfaces and adapters together with pinned
upstream sources, exact generated build inputs and build/install scripts.
The selected delivery method is GPLv2 section 3(a): accompany binaries with
this matching source on the same distribution medium; for downloads provide
equivalent access to the exact companion in the same place. Package.json
binds its name and SHA-256. Do not distribute the installer or updater alone
without that source access. No unfulfilled written-source-offer is substituted.

## Release scope and distribution

- The focused release review resolved the contribution and linked-runtime
  checks for the accepted standard kit. The original grant stays within
  the project owner's authority. Retain the recorded runtime permissions
  and exceptions in [TOOLCHAIN-PROVENANCE.md](TOOLCHAIN-PROVENANCE.md).
- Optional GNU nano is GPL-3.0-or-later. GPL-2.0-only Core cannot simply be
  assumed compatible with an in-process GPLv3 editor. No nano binary is in
  this kit. The standard vi kit retains the upstream vi.c grant, BusyBox
  licensing terms, port configuration and modified source.
- Apple-driver images remain excluded without an exact redistribution grant.
- The owner authorised publication of the verified 1978 set on GitHub.
  Use the matching installer, corresponding source and checksums together.
  The earlier private distribution and prior review archives remain separate.
  Mechanical packaging verification does not independently reproduce the
  accepted binaries.

References: [BusyBox licensing](https://busybox.net/license.html),
[GPLv2 terms](LICENSE). The earlier MIT draft and review artifacts are
historical only and are not part of the new installer/source companion.
