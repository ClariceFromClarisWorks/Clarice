"""Summarise matched observations without treating failures as speed wins."""
from pathlib import Path
import json,statistics,collections
r=Path(__file__).resolve().parents[2];o=r/'outputs/busybox-tools';rows=json.loads((o/'measure.json').read_text());groups=collections.defaultdict(dict)
for row in rows:
 if 'implementation' not in row:continue
 assert row['status']==0 and row['remaining']==0
 groups[(row['workload'],row['size'])].setdefault(row['implementation'],[]).append(row)
summary=[]
for (name,size),implementations in groups.items():
 record=dict(workload=name,size=size)
 for impl in ['existing','busybox']:
  a=sorted(implementations[impl],key=lambda x:x['run']);assert [x['run'] for x in a]==[0,1,2],(name,size,impl)
  record[impl]=dict(seconds=[x['guest_seconds'] for x in a],median=statistics.median(x['guest_seconds'] for x in a),repeat_median=statistics.median(x['guest_seconds'] for x in a[1:]),peak=max(x['peak'] for x in a),image=a[0]['image'],loads=[x['load'] for x in a],heap=[x['heap'] for x in a])
 record['busybox_reduction_percent']=round(100*(1-record['busybox']['median']/record['existing']['median']),1)
 summary.append(record)
assert len(summary)==47,len(summary)
(o/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print('| Workload | Existing seconds | BusyBox seconds | Change | Working peak existing / BB |')
print('|---|---:|---:|---:|---:|')
for a in summary:
 if a['size'] not in ['large','path']:continue
 e,b=a['existing'],a['busybox'];print(f"| {a['workload']} | {e['median']:.2f} | {b['median']:.2f} | {a['busybox_reduction_percent']:+.1f}% shorter | {e['peak']} / {b['peak']} B |")
