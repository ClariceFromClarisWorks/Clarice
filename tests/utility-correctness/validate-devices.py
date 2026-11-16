"""Validate saved actual Snow results, not a replacement for guest execution."""
from pathlib import Path
import json,re
r=Path(__file__).resolve().parents[2];d=json.loads((r/'outputs/utility-expansion/devices-guest.json').read_text());rows=d['cases']
expected=[0,0,0,0,0,0,1,0,1,1,0,0,0,0,127,0,0,0,0,0,2,0,0]
assert [x['status'] for x in rows]==expected
assert rows[0]['stdout']=='' and rows[1]['stdout']=='visible\n'
assert rows[2]['stdout']=='4294967295 0 /dev/null\n'
assert rows[5]['stdout'].split()==['00']*32
assert rows[7]['stdout']=='restored\n'
assert rows[11]['stdout']=='/mnt/Audit/tools/ls\n' and rows[13]['stdout']=='f\n'
for i in (16,17):assert rows[i]['stdout'] and not rows[i]['stderr'] and max(map(len,rows[i]['stdout'].splitlines()))<=72
v=[int(x) for x in re.findall(r'(\d+) bytes',d['free_fixed'])];assert len(v)==6
ram,extent,free,largest,purge,contig=v;assert ram==4194304 and 0<largest<=free<=purge<=extent<ram and largest<=contig<=purge
samples=re.findall(r'heap=(\d+) largest=(\d+).*remaining=(\d+)',d['repeat']);assert len(samples)==6
assert all(int(x[2])==0 for x in samples) and len({x[1] for x in samples})==1
assert max(int(x[0]) for x in samples)-min(int(x[0]) for x in samples)<=16
assert d['cancel_status']==130 and d['recovered']=='recovered\n'
print('PASS: 23 Snow cases, corrected native memory query, six stable repeat pairs, Ctrl+C=130 and recovery')
