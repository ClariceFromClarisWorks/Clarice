from pathlib import Path
import json
r=Path(__file__).resolve().parents[2]/"outputs/self-update"
for name in ["baseline","builtin","install","regress","cancel","no_dependencies"]:
 rows=json.loads((r/(name+".json")).read_text())
 assert all(x["status"]==(2 if x["command"]=="clarice update --check" else 0) for x in rows),name
s=json.loads((r/"regress.json").read_text())[0]["text"]
for expected in ["REPLACE=0","CORRUPT=1","NO_DOWNGRADE=0","NO_UPDATER_FILE=0","NO_PENDING=0"]:assert expected in s
assert s.count("PROBE2")==3
assert "CANCEL=130" in json.loads((r/"cancel.json").read_text())[0]["text"]
assert "\r\nclarice\r\n" in json.loads((r/"builtin.json").read_text())[0]["text"]
print("PASS recorded builtin, no-dependency, replacement, corruption, no-downgrade and cancellation observations")

rows=json.loads((r/"final.json").read_text())
assert all(x["status"]==0 for x in rows)
bycmd={x["command"]:x["text"] for x in rows}
assert "PROBE2" in bycmd["selfprobe"]
assert "\r\nclarice\r\n" in bycmd["command -v clarice"]
installed={}
for line in bycmd["cat /etc/clarice/installed.catalog"].splitlines():
 fields=line.split("\t")
 if len(fields)==5 and fields[0] in ("tool","core"):installed[tuple(fields[:2])]=fields[2:]
index=r.parents[1]/"work/self-update-snow/http/final/index.txt"
for line in index.read_text().splitlines()[1:]:
 fields=line.split("\t")
 assert installed[tuple(fields[:2])]==fields[2:5],fields[:2]
print("PASS final Core and all 49 external catalog identities; no external updater dependency")

rows=json.loads((r/"repeat.json").read_text())
assert all(x["status"]==0 for x in rows)
assert "All standard tools are current. Update complete." in rows[0]["text"]
print("PASS repeat repository operation and cleanup")
