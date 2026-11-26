#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c
"$ROOT/scripts/build-installer.sh"
python3 "$ROOT/scripts/package-utilities.py"
# Reuse the existing native-device packager; retain only its Apple driver/header.
TEMPLATE=${1:-"$ROOT/outputs/release-0.4.0/HD60_512_Clarice-0.4.0.hda"}
python3 "$ROOT/scripts/package-hda.py" "$TEMPLATE" "$ROOT/work/distribution/Clarice.hda" > "$ROOT/work/distribution/HDA-PROVENANCE.json"
docker run --rm --pull never --platform linux/amd64 --network none -v "$ROOT:/src" "$IMAGE" sh -ec '
hformat -l Clarice /src/work/distribution/Clarice.hda
hmount /src/work/distribution/Clarice.hda
hcopy -m /src/work/distribution/kit/Clarice.bin :Clarice
hcopy -m /src/work/install-ux/build/Clarice.bin ":Install Clarice"
hcopy -m /src/work/update-helper/ClariceUpdater.bin ":Recover Clarice"
hcopy -r /src/work/distribution/kit/Release :Release
hmkdir :Utilities
for f in /src/work/distribution/kit/Utilities/*; do hcopy -r "$f" ":Utilities:$(basename "$f")"; done
hcopy -r "/src/work/distribution/kit/Read Me.txt" ":Read Me"
hattrib -t TEXT -c ttxt ":Read Me"
hcopy -r "/src/work/distribution/kit/Third Party.txt" ":Third Party"
hmkdir :Licenses
licences=/src/work/distribution/kit/Licenses
find "$licences" -mindepth 1 -type d | LC_ALL=C sort | while IFS= read -r dir; do
 native=$(printf "%s" "${dir#"$licences"/}" | tr / :)
 hmkdir ":Licenses:$native"
done
find "$licences" -type f | while IFS= read -r f; do
 native=$(printf "%s" "${f#"$licences"/}" | tr / :)
 hcopy -r "$f" ":Licenses:$native"
done
humount
'
