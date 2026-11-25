from pathlib import Path
import sys,json,re,time
sys.path.insert(0,'tests/remote-sessions');from client import Client
variant=sys.argv[1];c=Client(port=23260,password='');rows=[];out=Path('outputs/filesystem-perf')/(variant+'.json')
def run(label,s,timeout=360):
 rc,text=c.command(s,timeout);rec=dict(label=label,command=s,status=rc,text=text);rows.append(rec);out.write_text(json.dumps(rows,indent=2)+'\n');print(label,rc,re.findall(r'PROFILE[^\r\n]*',text),flush=True);assert rc==0 and all(x=='0' for x in re.findall(r'PROFILE[^\r\n]*status=([0-9]+)',text)),text
try:
 run('warmup',f'/mnt/Perf/{variant}/iprofile find /mnt/Perf/n10 > /dev/null')
 for n in (10,100,1000):
  for tool in ('find','du'):
   run(f'{tool}-{n}',f'time /mnt/Perf/{variant}/iprofile {tool} /mnt/Perf/n{n} > /dev/null; status runtime')
  run(f'cp-{n}',f'time /mnt/Perf/{variant}/fprofile cp -Rp --no-progress /mnt/Perf/n{n} /mnt/Perf/copy > /dev/null; status runtime')
  run(f'rm-{n}',f'time /mnt/Perf/{variant}/fprofile rm -R --no-progress /mnt/Perf/copy > /dev/null; status runtime')
finally:c.close()
