# Staging the Clarice 0.5.0 draft

Use the integrated product build 2026111978, product commit f571f7c,
and its accepted local seal and native offline readback. The staging
tool deliberately leaves the original product inputs, USTAR, private
HDA, checksums and development distribution alone.

```sh
python3 scripts/release.py check
python3 tools/release-kit.py --product f571f7c \
  --offline-readback work/installer-c/offline-readback \
  --updater-tar work/installer-c/publication/Clarice-1978.tar \
  --output work/release-0.5.0-1978-complete-source
(cd work/release-0.5.0-1978-complete-source && shasum -a 256 -c SHA256SUMS.txt)
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

The licensing questions in [THIRD_PARTY.md](THIRD_PARTY.md) block public
redistribution clearance. The ZIP, source archive and SHA-256 list are
local draft artifacts. System 7 and physical validation of build 1978
remain separate acceptance work.

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

## Source and notice refresh

The refreshed installer carries 24 notices. The companion contains
1,192 files, including its manifest. Independent readback checked their
hashes, the Git product bytes and executable modes, the dated
comments-only BusyBox overlay, and the prepared source against the
accepted build inputs. All 53 binary and release-manifest ZIP members
matched the earlier installer ZIP byte-for-byte. The accepted updater
USTAR still matches the seal. The host package, collection, manifest
and journal tests passed again. No product binary was rebuilt.

The refreshed work directory is separate from the earlier local
installer draft, private driver-containing HDA and recovery artifacts.
