# Toolchain provenance for build 2026111978

Target: `m68k-apple-macos`, 68000. Product commit `f571f7c`.
Accepted build scripts pin the Linux/AMD64 toolchain image
`ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c`.
GHCR returned that manifest digest and config digest
`sha256:db531379aedca156b4ebb49516f3a1237cd4d205e889530898efa6c8d1908123`.
The image reports creation at 2026-10-31 12:23:10 UTC. Its labels identify the
Retro68 repository and Ubuntu 22.04. No source revision is in those labels.

## Directly verified

The pinned image reports GCC **16.1.0**, including a single-threaded,
m68000-target compiler configured from `/Retro68/gcc`. Its target's
`_newlib_version.h` reports **4.6.0**. Startup selects Multiversal
interfaces. The release uses the archives below directly or as linker
inputs. A static archive hash identifies the installed archive, without
implying that every member is included in a binary.

| Archive in the pinned image | SHA-256 |
| --- | --- |
| `libretrocrt.a` | `b17d76dac0455c6d38485c289542a0970fd6eca9b342517fb35fd1915c89e75d` |
| `libgcc.a` | `b1eda07c23cc635d71dc6d0b4bf06dbeadea5201423a854fa3064fba0a7aceed` |
| `libc.a` | `0dbd6b65de0b2332cbf365a1da9e420b71a862cd6172d970bd74dd1cf9e140d5` |
| `libm.a` | `9f6de65f7d3dd448c2d317b0449ddbb4fc9360f59f305fde5da2775c9c0256b6` |
| `libInterface.a` (same bytes under target and Multiversal paths) | `0e2a1543400e44b6a91d6fda188c42a3980470594be9a5c06f042bd3d17aace8` |

## Corroborated source candidate

The upstream Linux workflow ran 11:41:47–12:25:41 UTC on the image's
creation date and pushed the mutable `latest` tag. Its checkout was
`2aa5c87e88771c6057f52bf830af9d440a5e4e36`. Five installed setup scripts
match that Git revision byte-for-byte (`docker-entrypoint.sh`,
`install-universal-interfaces.sh`, `interfaces-and-libraries.sh`,
`prepare-headers.sh`, `prepare-rincludes.sh`). That revision's Multiversal
gitlink is `580f3309649ad4c788c048be3dff2416d7ba49f3`. Its
`custom/NavServicesAvailable.c`, `custom/cfmwrapper.c`, and `custom/glue.c`
also match the image byte-for-byte. [Retro68 revision](https://github.com/autc04/Retro68/commit/2aa5c87e88771c6057f52bf830af9d440a5e4e36) · [Upstream Linux job](https://github.com/autc04/Retro68/actions/runs/41197912675/job/124527722273) · [Multiversal revision](https://github.com/autc04/multiversal/tree/580f3309649ad4c788c048be3dff2416d7ba49f3).

For locating the candidate sources, the Retro68 `libretro` Git tree is
`81a6521697ebf3e6819b1a039cc480ae7cf66f4b`. The combined `gcc` tree is
`8b96f59a7264f6582b9cd7c9fa43cc053189250d`, including the
`gcc/libgcc` tree `578e81fd316ed19537096264e8309bd30f16c985`
and the `gcc/newlib` tree `a40b3a0b0afc0e7c103e98e4ac7e7b84a325fac4`.

The image metadata and accessible workflow data do not bind those trees
to the shipped digest. The job log endpoint refused unauthenticated
access (403); the digest attestation endpoint returned 404. Matching
scripts, C files, timestamps and version strings corroborate a candidate
but are insufficient to identify the exact source checkout for all linked
runtime code. No candidate runtime checkout is described as corresponding
source in the companion. Obtain digest-bound upstream build provenance or
independently verify the linked runtime source before release.

## Terms to carry forward

- Retro68 runtime sources retain their own notices (upstream GPLv3 or
  later with GCC Runtime Library Exception 3.1 where marked). See
  [Retro68 COPYING](https://github.com/autc04/Retro68/blob/2aa5c87e88771c6057f52bf830af9d440a5e4e36/COPYING)
  and [its runtime exception](https://github.com/autc04/Retro68/blob/2aa5c87e88771c6057f52bf830af9d440a5e4e36/COPYING.RUNTIME).
- GCC libgcc files carry GPLv3-or-later terms and the runtime exception
  where stated by the relevant file notice. The exception has eligibility
  conditions and applies to marked files. It neither licences Clarice
  nor changes BusyBox's GPLv2. [GCC Runtime Library Exception 3.1](https://www.gnu.org/licenses/gcc-exception-3.1.html).
- newlib and libm have per-component terms. Preserve the bundled
  `COPYING.NEWLIB` inventory and confirm the actual linked source/member
  notices against the exact source when verified.
- Multiversal's upstream copying terms are MIT. Preserve the copyright
  and notice for its headers/generated glue. The installed toolchain
  binary and Apple interfaces or driver images are outside the package.
  [Multiversal copying terms](https://github.com/autc04/multiversal/blob/580f3309649ad4c788c048be3dff2416d7ba49f3/COPYING).
