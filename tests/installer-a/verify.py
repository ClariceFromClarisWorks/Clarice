"""Verify recorded disposable-image readbacks; does not invent emulator results."""
from pathlib import Path
import hashlib, json, struct

root = Path(__file__).resolve().parents[2]
work = root / "work/installer-a"
seal = json.loads((root / "work/release/sealed.json").read_text())

def forks(path):
    blob = path.read_bytes()
    data, resource = struct.unpack_from(">II", blob, 83)
    offset = 128 + (data + 127) // 128 * 128
    rsrc = blob[offset:offset + resource]
    # Same documented Resource Manager scratch convention as existing tests.
    return blob[65:73], blob[128:128 + data], rsrc[:16] + rsrc[128:]

before = (work / "1976-before-cancel.hda").read_bytes()
after = (work / "1976-after-cancel.hda").read_bytes()
assert before == after
for row in seal["commands"]:
    assert (work / "upgrade-readback" / row["name"]).read_bytes() == (
        root / "work/build" / (row["image"] + ".clx")).read_bytes()
assert forks(work / "upgrade-readback/Clarice.bin") == forks(root / "work/build/Clarice.bin")
assert forks(work / "fresh1976-readback/Clarice.bin") == forks(root / "work/build/Clarice.bin")
for row in seal["commands"]:
    assert (work / "fresh1976-readback" / row["name"]).read_bytes() == (
        root / "work/build" / (row["image"] + ".clx")).read_bytes()
assert (work / "user-before.bin").read_bytes() == (work / "user-after.bin").read_bytes()
for name in ("saved.bin", "previous.bin"):
    assert (work / "fresh-readback" / name).read_bytes() == (work / "upgrade-readback" / name).read_bytes()
assert str(seal["build"]) in (work / "upgrade-readback/installed.catalog").read_text()
assert (work / "inventory-before").read_bytes() == (work / "inventory-after").read_bytes()
result = dict(build=seal["build"], standard_utilities=len(seal["commands"]),
              cancel_disk_sha256=hashlib.sha256(before).hexdigest(),
              cancel_disk_bytes=len(before), complete_core_and_tools=True,
              user_both_forks_and_finder_metadata_unchanged=True,
              historical_apps_unchanged=True, reinstall_snapshot_unchanged=True,
              system="Snow Plus, 4 MB, System 6.0.8", system7_tested=False,
              system7_deferred_by_user=True, physical_tested=False)
(root / "outputs/installer-a/results.json").write_text(json.dumps(result, indent=2) + "\n")
print("PASS whole-disk no-write Cancel; Core/49 tools; user forks/metadata; historical backups; unchanged reinstall snapshot")
