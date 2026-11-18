import sys,re,json
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions')
from client import Client
c=Client();rows=[]
for v in sys.argv[1:]:
 for fixture in ['buffer','empty','short','long']:
  for i in range(3):
   status,raw=c.command('cat /tmp/nb-'+fixture+'-'+v+str(i));assert status==0
   ts=list(map(int,re.search(r'Nano ticks: ([0-9 ]+);',raw)[1].split()))
   row=dict(fixture=fixture,variant=v,run=i,total=ts[8]-ts[0],load=int(re.search(r' load=(\d+)',raw)[1]),curses_init=ts[4]-ts[3],setup=ts[5]-ts[4],file_open=ts[6]-ts[5],layout=ts[7]-ts[6],first_refresh=ts[8]-ts[7],raw=raw)
   assert 'remaining=0' in raw
   rows.append(row)
c.close()
p=Path('outputs/nano-profile6/same-name.json');old=json.loads(p.read_text()) if p.exists() else [];old=[r for r in old if r['variant'] not in sys.argv[1:]];p.write_text(json.dumps(old+rows,indent=2)+'\n')
for f in ['buffer','empty','short','long']:
 a=[r['total'] for r in rows if r['fixture']==f];print(f,a,sum(a)/len(a))
