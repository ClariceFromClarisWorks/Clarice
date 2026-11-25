# Clarice 0.5.0 attribution and redistribution

Distribution draft for build 2026111978, product commit f571f7c.
The installed inventory is defined by `src/standard-commands.inc` and
the release manifest. Retained research source and optional tools have
separate terms.

## In the installer kit

| Component | Attribution and terms |
| --- | --- |
| BusyBox 1.37.0 | BusyBox contributors. Standard **wc, tr and sort** and their selected libbb code use GPL-2.0-only. Upstream archive SHA-256 `3311dff32e746499f4df0d5df04d7eb396382d7e108bb9250e7b519b837043a4`. See `Licenses/busybox/LICENSE`, `AUTHORS`, `src/ports/busybox-tools` and `scripts/prepare-busybox-tools.py`. |
| sbase | suckless developers and per-file authors, MIT/ISC and retained file notices. Provides grep, sed, head, tail, cut, uniq, tee, basename, dirname, cmp, cksum, sleep, printenv, mktemp and dd, plus the explicit wc-sbase, tr-sbase and sort-sbase alternatives. Revision `c546c3a5724c81cee9a11d816a38ccdf17472129`. Preparation and Macintosh adapters are in `scripts/` and `src/ports/`. The bundled ISC strtonum notice credits Ted Unangst and Todd Miller; additional per-file notices are preserved in the kit. |
| musl regex and fnmatch | Rich Felker and contributors, with Ville Laurikari's TRE-derived regex; retained notices for each snapshot. Regex supports the sbase ports; the byte-oriented fnmatch adaptation serves native file inspection/traversal. See the two musl copyright files and `src/vendor/USERLAND-SOURCES.json`. |
| One True Awk | Lucent Technologies and authors listed in `src/vendor/awk`. Revision `72e9b56caddb4a6f8d1405315a5183c26b8650a4`. Preparation and Macintosh runtime are in `scripts/prepare-awk.py` and `src/ports/awk-runtime.c`. |
| mrsh | emersion and contributors, MIT, revision `4c81598721bc5eeb28f9faa818b3102d0471b7f6`. Parser and AST are adapted; shell execution is Clarice's Macintosh implementation. |
| E-Kermit 1.8 | Frank da Cruz and the Trustees of Columbia University, revised three-clause BSD. Original notices and sources are in `src/ports/ekermit/upstream`. |
| Macinclaude MacTCP adapter | Bart Decrem, MIT. Provenance and changes in `src/commands/vendor/mactcp`. |
| libtelnet | Sean Middleditch and contributors, public-domain dedication and authors in `src/commands/vendor/libtelnet`. Used with the native transport and terminal adapter. |
| picohttpparser | Tokuhiro Matsuno, Kazuho Oku and contributors, MIT option, revision `dd573d8fa9264c646a400e96ce9f07dea6f156e4`. HTTP parsing in wget and the updater. |
| newlib getopt | Gregory Pietsch, public-domain notice preserved in the original source and the kit. Used by BusyBox ports. |
| Macintosh toolchain | Retro68 by Wolfgang Thaller and contributors, GPL-3.0-or-later with GCC Runtime Library Exception 3.1; Multiversal MIT; GCC runtime exception; individual newlib notices. The pinned image digest is in `scripts/build.sh`. The newlib file is the upstream notice inventory. Exact linked-runtime source revisions are unverified. TOOLCHAIN-PROVENANCE.md records archive hashes and corroborated candidates. |

Notice filenames in the kit are under `Licenses/`. Source headers and
upstream documentation retain per-file notices. The project uses the
Macintosh's installed fonts. The historical JetBrains Mono-derived
bitmap source is kept with the SIL OFL notice; no font is installed
by the kit.

## Optional editors and development tools

- The optional repository **vi** is the BusyBox 1.37.0 editor by Sterling
  Huxley and contributors, with corresponding BusyBox GPL-2.0-only
  obligations. It is not a standard installer utility. Its existing
  optional repository build is 2026111951. See `src/ports/busybox` and
  `scripts/prepare-busybox-vi.py`. Berkeley/Heirloom ex/vi and xvi are
  comparison ports; their notices remain under `Licenses/vi` and
  `Licenses/xvi`. This product includes software developed by the
  University of California, Berkeley and its contributors where the
  Berkeley prototype is distributed. The prototype retains the
  Caldera attribution and copying terms in its own notice.
- GNU nano 9.2 is an optional source port, GPL-3.0-or-later, with
  GNU helper files under their own retained terms including LGPL.
  The accepted generated configuration is under
  `src/ports/nano/prepared`. See `scripts/prepare-nano.py` and
  `outputs/nano/ADAPTATIONS.md`. Ncurses 6.5 is an experimental
  source dependency with its notice in `src/vendor/ncurses/COPYING`.
  No nano or ncurses binary is in the standard kit.
- microtar by rxi (MIT) and the sbase public-domain SHA-256 reference
  are retained in source. Production repository payload checks use CRC-32.
  The SHA-256 source is used by optional remote authentication.
- SEControl and the Snow emulator are separate development tools.
  SEControl's MiniVNC-derived portions are GPL-3.0-or-later and
  Macinclaude portions retain MIT terms. Snow is MIT. Their binaries
  and Apple ROM or System media are outside the kit.

## Original Clarice code and corresponding source

The product tree has no project-wide licence grant for original Clarice
code. The listed third-party permissions apply to their respective
components. There is no blanket permission to redistribute Clarice by
virtue of a public source repository. Obtain the rights holders'
explicit grant and confirm compatibility for Clarice adapters linked
into GPL-covered utilities. Clarify the linked-runtime exception and
CLX integration boundaries for this distribution before release.

The local source companion **@SOURCE_ARCHIVE@** stages the pinned
product source, licence inventory, adaptations, build instructions and
installation scripts, and hash-verified BusyBox 1.37.0 upstream source.
The generated BusyBox port files carry dated modification notices where
adapted. Its source manifest and the binary kit share the accepted seal
identity. See SOURCE-BUILD.md and LICENSING-DECISIONS.md in the companion.
Provide complete corresponding source alongside any binary distribution
of the BusyBox applets under GPLv2 section 3, including the granted
terms for required linked adapters. A pinned archive, fetch URL, or
toolchain image digest is not a substitute for resolving any missing
corresponding source. Optional GPL editors need their own matching
complete source and build materials when distributed. Confirm the
rights grant and remaining runtime/source provenance before publication.

BusyBox licensing: https://busybox.net/license.html
GPLv2: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
GPLv3: https://www.gnu.org/licenses/gpl-3.0.html

## Apple driver and private media

The historical private physical-device HDA contains an Apple_Driver43
partition from a local template. The available provenance shows its
origin and checksum, not a redistribution grant. No permission specific
to that driver was verified. Do not redistribute that HDA. The local ZIP
uses fork-preserving MacBinary files and contains no Apple driver,
ROM, System, Finder or emulator image. Existing private HDA and
recovery material remain available for their existing use. Apple lists
licences according to the terms applicable to each product:
https://www.apple.com/legal/sla/
