"""Validate exported actual-guest help and focused new-command results."""
from pathlib import Path
import json,subprocess
r=Path(__file__).resolve().parents[2];base=r/'outputs/utility-expansion';rows=json.loads((base/'help-inventory-after.json').read_text());g=json.loads((base/'guest-results.json').read_text())
for x in rows:
 i=x['index'];out=g[f'o{i}'].encode('latin1');err=g[f'e{i}'].encode('latin1');status=int(g[f's{i}']);assert status==0 and out and not err,x
 assert max(map(len,out.splitlines()))<=(80 if x['name']=='nano' else 72),x
expect=[0,1,1,0,2,2,0,0,0,0,2,1,2,2,2,2]
for i,rc in enumerate(expect):assert int(g[f't{i}.status'])==rc,i
for i in (4,5,10,11,12,13,14,15):assert not g[f't{i}.out'].encode('latin1') and g[f't{i}.err'].encode('latin1'),i
assert g['t2.out'].encode('latin1')==b'1201 260 132\n'
data=bytes(range(256))*256
ref=subprocess.run(['/usr/bin/cksum'],input=data,capture_output=True,check=True).stdout
assert g['t7.out'].encode('latin1')==ref and g['t8.out'].encode('latin1')==ref
assert b'remaining=0' in g['after'].encode('latin1')
print(f'PASS: {len(rows)} actual guest help results; 16 utility cases; reference CRC/pipeline; repeated-run cleanup')
