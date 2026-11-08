# Project-local Snow input patch

Base: Snow a921b108ef8bce8ba43fce121d46df0ea9d219af, matching the previously installed emulator revision. See provenance.json and SNOW-LICENSE.txt for identity and upstream authorship. This is a local modification, not an upstream release.

Build with scripts/build-emulator-input.sh. Rust 1.95.0 and the pinned Cargo.lock are required; the first build downloads dependencies. The source patch is patches/snow-plus-input.patch. The build script does not replace or launch the emulator.

The existing emulator binary is backed up at work/emulator/Snow-before-input-fix. Guest results are recorded in TEST-REPORT.md. System ROMs, original disks, SkiFree and other emulator installations are outside the modification scope.

Arrow packets use the 0x79 keypad prefix described by the [M0110/M0120 protocol reference](https://github.com/tmk/tmk_keyboard/wiki/Apple-M0110-Keyboard-Protocol). Control on the original non-ADB keyboard is a host convenience extension and must not be represented as a physical Plus-keyboard capability. Up/Down history and the Control+C convenience mapping have been verified in Clarice under the Plus guest. Host Control maps to guest Option on non-ADB models, including in other guest applications. Command+period is the native interrupt shortcut; Command+C remains Copy. Left/Right remain ignored on non-ADB models pending guest compatibility work.
