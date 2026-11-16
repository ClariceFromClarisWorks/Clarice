from pathlib import Path
import json,re
r=Path(__file__).resolve().parents[2];d=r/'outputs/packaging';v=json.loads((d/'variants.json').read_text());full=v['full'];core=v['core']
assert [x for x in full['resources'] if x['type']!='CLX1']==core['resources']
assert full['clx_payload']==780568 and core['clx_payload']==0
rows=[]
for line in (d/'guest.txt').read_text().splitlines():rows.append(dict((k,int(n)) for k,n in re.findall(r'(\w+)=(-?\d+)',line)))
assert len(rows)==10
assert all(x['loaded']==0 and x['census_cost']==0 for x in rows)
assert all(x['before']==x['after'] for x in rows[1:])
assert all(x['install']==112 for x in rows[1:4])
assert all(x['install']==0 for x in rows[4:])
assert all(x['clx_count']==28 for x in rows[:7]) and all(x['clx_count']==0 for x in rows[7:])
assert full['code_payload']==core['code_payload']
assert all(x['before']==rows[1]['before']+344 for x in rows[7:])
print('PASS: ten recorded startup samples; identical non-CLX resources; no retained utility payload; 112-tick scan and 344-byte map delta')
