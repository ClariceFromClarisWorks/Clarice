from pathlib import Path
import sys,json,time
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/remote-sessions'));from client import Client
c=Client(port=23245,password=(r.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip());rows=[]
try:
 for cmd,expected in [('difftrace /tmp/ba-lines256k /tmp/ba-edited256k > /tmp/ra-baseline.diff',2),('status runtime',0),('difftrace /tmp/ba-lines8k /tmp/ba-edited8k > /tmp/ra-small.diff',1),('status runtime',0)]:
  t=time.monotonic();rc,text=c.command(cmd,300);rows.append(dict(command=cmd,status=rc,text=text,host_seconds=time.monotonic()-t));(r/'outputs/realloc/baseline.json').write_text(json.dumps(rows,indent=2)+'\n');assert rc==expected,(rc,text);print('PASS',cmd,flush=True)
finally:c.close()
