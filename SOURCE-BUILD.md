# Building the Clarice 0.5.0 source companion

The source companion follows accepted build 2026111978, product f571f7c.
It includes the pinned project source and its executable build scripts,
assets, tests, licences, and the verified BusyBox 1.37.0 upstream archive
in `work/busybox/`. Extract it to a new directory. Use Docker with AMD64
support, Python 3 and a host C compiler. Fetch the pinned image once:

```sh
docker pull --platform linux/amd64 ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
sh scripts/build.sh
sh scripts/build-installer.sh
```

The full build selects all 49 standard utilities and builds the native
recovery helper and built-in updater. It seals Core and utilities in
`work/release/sealed.json`, stages the collection, and produces the
separate updater USTAR. The native installer is the subsequent
`work/install-ux/build/Clarice.bin` output. The recovery helper is
`work/update-helper/ClariceUpdater.bin`. Use the fork-safe instructions
in [RELEASE-START.md](RELEASE-START.md) for staging decoded applications.
No Apple disk, ROM or driver is needed for this build.

For the BusyBox applets alone, use `sh scripts/build-busybox-tools.sh`.
It unpacks the included upstream archive and regenerates the selected
configuration, help, source and libbb inputs. The companion's
`scripts/prepare-busybox-tools.py` adds comments with dated alteration
notices to generated modified sources. This is the only executable
product-source overlay. The prepared `wc.c` and `tr.c` are unchanged;
the prepared `sort.c` and `full_read.c` differ from the accepted build
inputs only by the new notices. The staged forms are in
`work/busybox-tools/` for inspection; the build regenerates them.

`SOURCE-MANIFEST.json` identifies all staged source files, the original
preparer's hash, accepted binary seal identity, and updater USTAR hash.
`BUILD-SEAL.json` is the accepted original binary manifest. The install
ZIP's `Package.json` identifies the matching companion by SHA-256. The
ZIP still ships accepted binary bytes from the original seal, without
rebuilding the application or applets for this notice correction.

[TOOLCHAIN-PROVENANCE.md](TOOLCHAIN-PROVENANCE.md) distinguishes the
exact compiler/runtime objects from corroborated source candidates.
The current source companion does not close those source-lineage or
rights questions. [LICENSING-DECISIONS.md](LICENSING-DECISIONS.md)
records the remaining approval boundaries. Optional binaries and their
external upstream archives are outside the standard installer scope.
