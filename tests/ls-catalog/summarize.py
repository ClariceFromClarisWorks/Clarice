"""Read guest timing/status evidence without treating emulator ticks as hardware."""
from pathlib import Path
import re,json,statistics
p=Path('outputs/ls-catalog');text=(p/'guest-results.txt').read_text()
pattern=r'(?m)^(file|screen|names):(\d+):(\d+):(base|frmt|next)\n'
marks=list(re.finditer(pattern,text));rows=[]
for i,m in enumerate(marks):
 body=text[m.end():marks[i+1].start() if i+1<len(marks) else len(text)]
 def field(pattern):
  hit=re.search(pattern,body);assert hit,(m[0],pattern,body);return hit.groups()
 seconds=float(field(r'real ([0-9.]+) s')[0]);status=int(field(r'status:(\d+)')[0])
 heap,largest,image,ticks,load,peak,remaining=map(int,field(r'Runtime: heap=(\d+) largest=(\d+) image=(\d+) ticks=(\d+) load=(\d+) owned-peak=(\d+) remaining=(\d+)'))
 assert status==0 and remaining==0,(m[0],status,remaining)
 rows.append(dict(mode=m[1],files=int(m[2]),run=int(m[3]),variant=m[4],seconds=seconds,status=status,heap=heap,largest=largest,image=image,module_ticks=ticks,load_ticks=load,owned_peak=peak,remaining=remaining))
assert len(rows)==48,len(rows)
comparisons=re.findall(r'compare-(?:fmt|next):(\d+)',text);assert len(comparisons)==16 and set(comparisons)=={'0'},comparisons
summaries=[]
for mode,n in [('file',100),('file',1000),('screen',100),('screen',1000),('names',100)]:
 for v in ['base','frmt','next']:
  group=[r for r in rows if r['mode']==mode and r['files']==n and r['variant']==v and (mode!='file' or r['run']>0)]
  if not group:continue
  assert len(group)==3
  s=dict(mode=mode,files=n,variant=v,samples=[r['seconds'] for r in group],median=statistics.median(r['seconds'] for r in group))
  summaries.append(s);print(s)
(p/'measurements.json').write_text(json.dumps(dict(rows=rows,summaries=summaries,output_comparisons=len(comparisons)),indent=2)+'\n')
print('PASS',len(rows),'timed successful invocations; zero tracked survivors;',len(comparisons),'long-output comparisons')
