"""Authenticated read-only checks in this task's isolated Snow guest."""
from pathlib import Path
import sys,json,time
root=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(root/'tests/remote-sessions'))
from client import Client
phase=sys.argv[1]
c=Client(port=23246,password=(root.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip())
rows=[]
try:
 commands=['command -v clarice','clarice --help','status runtime','clarice --help','status runtime']
 if phase!='baseline':commands+=['help clarice','clarice update --check','status runtime']
 for cmd in commands:
  t=time.monotonic();rc,text=c.command(cmd,60);rows.append(dict(command=cmd,status=rc,text=text,host_seconds=time.monotonic()-t))
  assert rc==(2 if cmd=='clarice update --check' else 0),(cmd,rc,text)
finally:c.close()
(root/f'outputs/self-update/{phase}.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS',phase,len(rows))
