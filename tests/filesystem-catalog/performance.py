"""Identical ordinary catalog fixture; separate normal Core1969/1970 launches."""
from pathlib import Path
import sys,json,re
sys.path.insert(0,'tests/remote-sessions');from client import Client
variant=sys.argv[1];c=Client(port=23260,password='');rows=[]
try:
 for repeat in range(3):
  for name,cmd in [('cp','cp -Rp --no-progress /mnt/Perf/n100 /mnt/Perf/copy'),('rm','rm -R --no-progress /mnt/Perf/copy'),('find','find /mnt/Perf/n1000'),('du','du /mnt/Perf/n1000')]:
   rc,t=c.command(f'time {cmd} > /dev/null; result=$?; status runtime; test $result -eq 0',240)
   m=re.search(r'Runtime: heap=(\d+) largest=(\d+) image=(\d+) ticks=(\d+) load=(\d+) owned-peak=(\d+) remaining=(\d+)',t);assert rc==0 and m and m[7]=='0',t
   row=dict(repeat=repeat,variant=variant,command=name,ticks=int(m[4]),load=int(m[5]),execution=int(m[4])-int(m[5]),heap=int(m[1]),largest=int(m[2]),peak=int(m[6]),remaining=int(m[7]),text=t);rows.append(row);Path(f'outputs/filesystem-catalog/performance-{variant}.json').write_text(json.dumps(rows,indent=2)+'\n');print(name,row['execution'],flush=True)
finally:c.close()
