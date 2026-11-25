import sys,json,time
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions')
from client import Client
stage=sys.argv[1];w,f=('wb','fb') if stage=='baseline' else ('wcand','fcand')
c=Client(port=23265,password='');rows=[]
try:
 for rep in range(3):
  for mode,size in [('fast',1024),('fast',8192),('fast',65536),('header',1024),('idle',1024)]:
   for cmd in (w,'wget' if stage=='baseline' else 'wnormal'):
    c.command('rm -f /tmp/4dgot')
    source=f'time {cmd} -O /tmp/4dgot http://192.168.1.112:18085/{mode}/{size}'
    rc,output=c.command(source,90);assert rc==0,output
    assert c.command(f'cmp /tmp/4dgot /tmp/expected{size}')[0]==0
    row=dict(rep=rep,command=source,output=output);rows.append(row);print(row,flush=True)
  for size in (1024,8192,65536):
   for cmd in (f,'ftp' if stage=='baseline' else 'fnormal'):
    c.command('rm -f /tmp/4dgot')
    source=f'time {cmd} -P 22368 get 192.168.1.112 {size} /tmp/4dgot'
    rc,output=c.command(source,90);assert rc==0,output
    assert c.command(f'cmp /tmp/4dgot /tmp/expected{size}')[0]==0
    row=dict(rep=rep,command=source,output=output);rows.append(row);print(row,flush=True)
finally:
 c.close();Path(f'outputs/network-4d/{stage}.json').write_text(json.dumps(rows,indent=2)+'\n')
