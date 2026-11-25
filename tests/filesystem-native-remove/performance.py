"""Matched 1968/1969 rm traversal, unchanged 100-file Batch3C fixture."""
from pathlib import Path
import sys,json,re
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23260,password='');rows=[]
try:
 for repeat in range(3):
  for variant in (('before','after') if repeat%2==0 else ('after','before')):
   rc,t=c.command('/mnt/Perf/after/cp -Rp --no-progress /mnt/Perf/n100 /mnt/Perf/copy',180);assert rc==0,t
   rc,t=c.command(f'time /mnt/Perf/{variant}/rm -R --no-progress /mnt/Perf/copy > /dev/null; result=$?; status runtime; test $result -eq 0',180)
   m=re.search(r'Runtime: heap=(\d+) largest=(\d+) image=(\d+) ticks=(\d+) load=(\d+) owned-peak=(\d+) remaining=(\d+)',t);assert rc==0 and m and m[7]=='0',t
   row=dict(repeat=repeat,variant=variant,status=rc,ticks=int(m[4]),load=int(m[5]),execution=int(m[4])-int(m[5]),heap=int(m[1]),largest=int(m[2]),remaining=int(m[7]),text=t);rows.append(row);Path('outputs/filesystem-native-remove/performance.json').write_text(json.dumps(rows,indent=2)+'\n');print(variant,row['execution'],flush=True)
finally:c.close()
