#!/bin/sh
# Builds the project-local Snow input patch; does not install or launch it.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
SOURCE="$ROOT/work/emulator/snow-source"
BASE=a921b108ef8bce8ba43fce121d46df0ea9d219af
PATCH="$ROOT/patches/snow-plus-input.patch"
if [ ! -d "$SOURCE" ]; then
    git clone https://github.com/twvd/snow.git "$SOURCE"
    git -C "$SOURCE" checkout --detach "$BASE"
fi
[ "$(git -C "$SOURCE" rev-parse HEAD)" = "$BASE" ] || { echo 'Unexpected Snow source revision' >&2; exit 1; }
if git -C "$SOURCE" apply --check "$PATCH" 2>/dev/null; then
    git -C "$SOURCE" apply "$PATCH"
else
    git -C "$SOURCE" apply --reverse --check "$PATCH"
fi
mkdir -p "$ROOT/outputs/emulator-input"
cd "$SOURCE"
cargo build --release --locked -p snow_frontend_egui > "$ROOT/outputs/emulator-input/build.log" 2>&1
cargo test --release --locked -p snow_core clarice_input_tests > "$ROOT/outputs/emulator-input/tests.log" 2>&1
printf 'Built %s/target/release/snowemu\n' "$SOURCE"
