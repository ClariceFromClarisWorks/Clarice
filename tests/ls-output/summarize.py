from pathlib import Path
import re,json,statistics
p=Path('outputs/ls-output');text=(p/'guest-results.txt').read_text()
marks=list(re.finditer(r'(?m)^(file|screen):(100|1000):(names|long):(\d+):(before|after)\n',text));rows=[]
for i,m in enumerate(marks):
 b=text[m.end():marks[i+1].start() if i+1<len(marks) else len(text)]
 def field(pattern):
  r=re.search(pattern,b);assert r,(m[0],pattern,b);return r.groups()
 seconds=float(field(r'real ([0-9.]+) s')[0]);status=int(field(r'status:(\d+)')[0])
 heap,largest,image,ticks,load,peak,remaining=map(int,field(r'Runtime: heap=(\d+) largest=(\d+) image=(\d+) ticks=(\d+) load=(\d+) owned-peak=(\d+) remaining=(\d+)'))
 assert status==0 and remaining==0
 rows.append(dict(destination=m[1],files=int(m[2]),mode=m[3],run=int(m[4]),variant=m[5],seconds=seconds,heap=heap,largest=largest,image=image,ticks=ticks,load=load,peak=peak,remaining=remaining))
assert len(rows)==36,len(rows)
comparisons=re.findall(r'compare:(\d+)',text);assert len(comparisons)==16 and set(comparisons)=={'0'}
assert 'flush:0' in text
for n in [100,1000]:
 for mode in ['names','long']:
  for v in ['before','after']:
   group=[r for r in rows if r['destination']=='file' and r['files']==n and r['mode']==mode and r['variant']==v]
   samples=[r['seconds'] for r in group if r['run']>0]
   print(n,mode,v,'first',group[0]['seconds'],'repeats',samples,'median',statistics.median(samples))
for r in rows:
 if r['destination']=='screen':print('SCREEN',r['mode'],r['variant'],r['seconds'])
(p/'measurements.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS: 36 successful guest commands; 16 exact comparisons; remaining0 every run (runtime counters exclude direct cli_alloc); flush0')
