from pathlib import Path
import sys,json,time
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/remote-sessions'));from client import Client
phase=sys.argv[1];c=Client(port=23246,password=(r.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip());rows=[]
try:
 for cmd in sys.argv[2:]:
  t=time.monotonic();rc,text=c.command(cmd,90);rows.append(dict(command=cmd,status=rc,text=text,host_seconds=time.monotonic()-t))
finally:c.close()
(r/f'outputs/self-update/{phase}.json').write_text(json.dumps(rows,indent=2)+'\n');print(json.dumps(rows,indent=2))
