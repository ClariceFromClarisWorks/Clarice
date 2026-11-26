# Clarice 0.5.1 development notes

Build **2026111979** adds BusyBox 1.37.0 `vi` to the standard utility set.
Both a fresh offline installation and a managed update install it as
`/bin/vi`. The collection now contains 50 standard utility names.

The editor remains a demand-loaded CLX utility. Its byte-oriented editing,
limited ex commands, fork-preserving HFS saves and best-effort unsaved
text recovery use the existing port. Use `vi --help` for supported options
and limitations. The corresponding-source archive accompanies the GPLv2
installer. Third-party notices are retained.

This is a local development candidate targeting a 68000 with 4 MiB RAM
and System 6.0.8. The previously reported intermittent physical-SE
coprocessor exception remains unresolved. The published 0.5.0 release
continues to be available at its existing tag.
