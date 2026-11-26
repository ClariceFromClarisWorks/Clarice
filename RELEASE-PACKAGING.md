# Clarice release packaging

For 0.5.1, use the accepted 2026111979 seal at product commit d44e891.
Run `tools/release-kit.py --version 0.5.1 --final` with the sealed build,
accepted offline readback and matching updater USTAR. Pass the resulting ZIP
to `tools/classic-installer.py --final`, then independently extract and
verify all three applications, the 50 utilities and the nested notices.
Verify the SHA-256 manifest and matching source bindings. The examples
below document the historical 0.5.0 handoff.

## Required internal Macintosh handoff

Never hand off an HDA as an installer. HDA files are emulator/test fixtures;
internal users must be able to download and install entirely within System 6.
Convert the verified staging ZIP with the existing upstream StuffIt packager:

```sh
python3 tools/classic-installer.py \
  work/release-0.5.0-1978-gpl/Clarice-0.5.0-2026111978-Installer.zip \
  work/se-installer-1978-gpl
```

This packages native Core, Install Clarice and Recover Clarice applications,
Release, all 49 standard utilities and the notices as a StuffIt 1.5.1-compatible
archive wrapped in MacBinary. No application recompilation or new build ID is
needed for this packaging-only conversion. Independently extract and compare
every file, application fork and Finder type/creator before copying the same
immutable `.sit.bin` to canonical HTTP and AFP. Verify both downloads before
handing over the URL. Preserve older recovery material; do not offer HDA links.

Current internal kit: `http://192.168.1.112:8089/full/Clarice-1978-Installer.sit.bin`
and `Macintosh Exchange/Clarice/Clarice-1978-Installer.sit.bin`.

## Maintainer/source staging

Use the integrated product build 2026111978, product commit f571f7c,
and its accepted local seal and native offline readback. The staging
tool deliberately leaves the original product inputs, USTAR, private
HDA, checksums and development distribution alone.

```sh
python3 scripts/release.py check
python3 tools/release-kit.py --product f571f7c \
  --offline-readback work/installer-c/offline-readback \
  --updater-tar work/installer-c/publication/Clarice-1978.tar \
  --output work/release-0.5.0-1978-gpl
(cd work/release-0.5.0-1978-gpl && shasum -a 256 -c SHA256SUMS.txt)
```

The offline-readback directory contains the extracted MacBinary Core,
installer and recovery helper, the Release manifest and 49 native
utility data files from the accepted local installation kit.
The staging tool compares the digital files with the seal, offline
readback and existing USTAR where relevant. It compares both MacBinary
forks and Finder identity, normalizing only reserved resource-header
bytes for native-HFS readback. It rewrites the embedded Finder names in
the installer and helper MacBinary headers, with a new header CRC,
while leaving the wrapped fork bytes unchanged.

The ZIP is an outer transfer package for manual Finder setup. It
carries no physical-device HDA or Apple driver. Its Utilities and
Release remain native binary data files, and its Licenses tree keeps
nested notices. The source companion preserves the pinned product inputs, with a
comments-only preparer overlay for regenerated BusyBox files. It includes
build and installation instructions, notices and the verified upstream
archive. The package manifest records the companion SHA-256. See
[SOURCE-BUILD.md](SOURCE-BUILD.md) and [TOOLCHAIN-PROVENANCE.md](TOOLCHAIN-PROVENANCE.md).

The approved GitHub release uses the verified GPL-2.0-only ZIP, native
StuffIt and matching source archives. Use the public release notes for the
final System 6 acceptance and residual physical reliability status. The
verified package bytes retain the earlier staging-era documentation.

## Verification for this draft

- The existing sealed Core, its bundled updater, the native installer and
  helper readback, and the unchanged updater USTAR matched. Installer C's
  accepted System 6 Snow readback contains the same 49 standard utilities.
- The completed ZIP's manifest and archive were compared member by member.
  A fresh, driver-free disposable HFS volume decoded the three MacBinary
  files and read back Release, all 49 utilities and all notice files in the original 22-notice draft.
  This was a host packaging check, without a new guest installation.
- The previous host package, collection, manifest, journal and Installer C
  tests passed against the unchanged 1978 binaries. The updated companion
  separately validates its per-file manifest and the SHA-256 reference
  from the installer ZIP.
- The original private HDA and updater USTAR retain their recorded
  SHA-256 checksums. No public disk image was staged. See the local
  output directory's `SHA256SUMS.txt` for the new draft artifacts.

## GPLv2 source and notice refresh

The GPL-2.0-only project grant replaces the unissued MIT draft. Preserve
all third-party notice bytes. Stage to the separate `1978-gpl` directories
above, keeping earlier packages as historical evidence. The source companion
includes the root LICENSE, scoped grant, accepted BusyBox generated inputs,
Core/interfaces and pinned runtime sources. See LICENSING-DECISIONS.md.

Deliver the exact matching source companion with the binaries on the same
medium, or offer equivalent download access at the same location under GPLv2
section 3(a). The native kit retains the source filename and SHA-256 in its
Package.json. The approved publication destination for this
set is the GitHub v0.5.0 release.
