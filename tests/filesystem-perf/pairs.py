"""Interleaved normal-command measurements; fixed fixture and destination names."""
from pathlib import Path
import sys,json,re,time
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23260,password='');rows=[];p=Path('outputs/filesystem-perf/pairs.json')
try:
 for n in (10,100,1000):
  for repeat in range(3):
   for variant in (('before','after') if repeat%2==0 else ('after','before')):
    for tool in ('find','du','cp','rm'):
     args=f'/mnt/Perf/n{n}' if tool in ('find','du') else (f'-Rp --no-progress /mnt/Perf/n{n} /mnt/Perf/copy' if tool=='cp' else '-R --no-progress /mnt/Perf/copy')
     cmd=f'time /mnt/Perf/{variant}/{tool} {args} > /dev/null; result=$?; status runtime; test $result -eq 0'
     rc,text=c.command(cmd,360);m=re.search(r'Runtime: heap=(\d+) largest=(\d+) image=(\d+) ticks=(\d+) load=(\d+) owned-peak=(\d+) remaining=(\d+)',text);assert rc==0 and m and m[7]=='0',text
     r=dict(entries=n,repeat=repeat,variant=variant,command=tool,status=rc,heap=int(m[1]),largest=int(m[2]),ticks=int(m[4]),load=int(m[5]),execution=int(m[4])-int(m[5]),peak=int(m[6]),remaining=int(m[7]),text=text);rows.append(r);p.write_text(json.dumps(rows,indent=2)+'\n');print(n,repeat,variant,tool,r['ticks'],r['execution'],flush=True)
finally:c.close()
