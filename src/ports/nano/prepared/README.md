# Accepted nano 9.2 target preparation

These are the generated GNU headers and configured target header used for the
validated 68000 nano port, preserved as build inputs so the ignored work folder
is not required to recover it. Original GNU notices remain in each file; see
../COPYING and the pinned upstream archive in scripts/prepare-nano.py.

config/config.h is the existing adapted configuration, not a new configuration
or a guarantee of general POSIX services. regex-include contains the existing
standalone GNU regex/getopt headers. generated/ restores configured upstream
headers plus src/revision.h. No object files or host session configuration.

prepare-nano.py extracts the checksum-verified archive in a fresh checkout and
copies these inputs. link-nano-probe.sh applies the existing safe-save and timing
patches. All modified upstream C files are reproduced by those two patches.
The bundled editor remains nano 9.2 tiny/non-NLS/non-UTF-8; no editor feature or
runtime changes are part of this checkpoint.
