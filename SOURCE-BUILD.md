# Building the Clarice 0.5.1 source companion

The 0.5.1 source companion follows accepted build 2026111979.
It includes the pinned project source and its executable build scripts,
assets, tests, licences, and the verified BusyBox 1.37.0 upstream archive
in `work/busybox/`. Extract it to a new directory. Use Docker with AMD64
support, Python 3 and a host C compiler. Fetch the pinned image once:

```sh
docker pull --platform linux/amd64 ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
sh scripts/build.sh
sh scripts/build-installer.sh
```

The full build selects all 50 standard utilities, including BusyBox vi, and builds the native
recovery helper and built-in updater. It seals Core and utilities in
`work/release/sealed.json`, stages the collection, and produces the
separate updater USTAR. The native installer is the subsequent
`work/install-ux/build/Clarice.bin` output. The recovery helper is
`work/update-helper/ClariceUpdater.bin`. Use the fork-safe instructions
in [RELEASE-START.md](RELEASE-START.md) for staging decoded applications.
No Apple disk, ROM or driver is needed for this build.

To convert a staged ZIP into a classic installer archive, see
[RELEASE-PACKAGING.md](RELEASE-PACKAGING.md). The companion includes the
wrapper script. The separate host-side [sit packager](https://github.com/thecloudexpanse/sit/tree/821ae5ea6d0c76ab83ac0942e12f398aa5944250)
at revision `821ae5ea6d0c76ab83ac0942e12f398aa5944250` creates the StuffIt
container on macOS. Its binary is outside the Clarice release.

For the BusyBox applets alone, use `sh scripts/build-busybox-tools.sh`.
It unpacks the included upstream archive and regenerates the selected
configuration, help, source and libbb inputs. The companion's
`scripts/prepare-busybox-tools.py` adds comments with dated alteration
notices to generated modified sources. This is the only executable
product-source overlay. The prepared `wc.c` and `tr.c` are unchanged;
the prepared `sort.c` and `full_read.c` differ from the accepted build
inputs only by the new notices. The companion also includes byte-identical
prepared configuration, help and selected headers. These files are in
`work/busybox-tools/` for inspection. The build regenerates them.

The full build also runs `scripts/build-busybox-vi.sh` with profiling
disabled. The companion supplies the BusyBox 1.37.0 sources, the exact
prepared `work/busybox/vi.c`, the dated change notice and patch, the
configuration, runtime adapters and packaging scripts.

`SOURCE-MANIFEST.json` identifies all staged source files, the original
preparer's hash, accepted binary seal identity, and updater USTAR hash.
`BUILD-SEAL.json` is the accepted original binary manifest. The install
ZIP's `Package.json` identifies the matching companion by SHA-256. The ZIP and native installer ship the application and utilities from this seal.

[TOOLCHAIN-PROVENANCE.md](TOOLCHAIN-PROVENANCE.md) binds the exact toolchain
image to its upstream source revisions through the original build log.
The complete Retro68 and Multiversal archives are included under
`work/runtime-source/`, with hashes checked during staging. To inspect or
rebuild the toolchain, extract the Retro68 archive and put the contents of
the Multiversal archive in its `multiversal/` subdirectory. Follow upstream
`README.md` and `build-toolchain.bash`; the upstream Dockerfile also records
the build used for the accepted image. No toolchain or product rebuild was
performed for this packaging-only refresh.

Including the full toolchain source adds approximately 229 MiB to the host
source download, not to the Macintosh installer or installed application.
[LICENSING-DECISIONS.md](LICENSING-DECISIONS.md) records the effective GPL-2.0-only grant for original
Clarice contributions and the scope of the kit. Supply this exact source
companion alongside the binaries under GPLv2 section 3(a). Optional binaries
and their external upstream archives are outside the standard installer scope.
