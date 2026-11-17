from pathlib import Path
import re,json
out=Path('outputs/nano-perf');rows=[]
for name in ['before0','before1','before2','before3','after0','after1','after2','after3','cold0','cold1','edit','save']:
 p=out/(name+'.txt')
 if not p.exists():continue
 s=p.read_text();m=list(map(int,re.search(r'Nano ticks: ([\d ]+);',s)[1].split()));loader={k:int(v) for k,v in re.findall(r'(read|validate|imports|relocations|prepare|events)=(\d+)',s)}
 rows.append(dict(run=name,total=m[8]-m[0],runtime=m[2]-m[1],ncurses=m[4]-m[3],file=m[6]-m[5],first_paint=m[8]-m[7],save=m[12]-m[11],loader=loader,owned_peak=int(re.search(r'owned-peak=(\d+)',s)[1]),remaining=int(re.search(r'remaining=(\d+)',s)[1]),heap=int(re.search(r'heap=(\d+)',s)[1])))
(out/'measurements.json').write_text(json.dumps(rows,indent=2)+'\n')
for r in rows:print(r['run'],r['total'],round(r['total']/60,3),r['loader'],r['file'],r['first_paint'])
