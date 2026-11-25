"""Optional localhost-only Snow paste acceptance; run after starting the disposable fixture.
Native keyboard/editor checks are recorded separately in the acceptance report.
"""
import sys,time,json,re,subprocess
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions');from client import Client
rows=[];out=Path('work/network-4c/snow-edit.json')
def record(name,**fields):
 rows.append(dict(test=name,**fields));out.write_text(json.dumps(rows,indent=2)+'\n');print(name,fields,flush=True)
def command(c,s):
 rc,text=c.command(s,60);assert rc==0,(s,rc,text);record(s,status=rc,text=text);return text
c=Client(port=23265,password='');command(c,'remote status');command(c,'netheap2');command(c,'status runtime')
for cycle in range(5):
 c.t.read_very_eager();tail=b't'*700;paste=b'p'*300
 c.t.write(b"printf '%s\\n' '"+tail)
 time.sleep(.15);c.t.write(b'\x1b[H'+b'\x1b[C'*len(b"printf '%s\\n' '"))
 time.sleep(.1);t=time.monotonic();c.t.write(paste+b'\x08'+b'p'+b"\x1b[F'\r\n")
 answer=c.t.read_until(b' > ',30)
 assert b'\r\n'+paste+tail+b'\r\n' in answer,repr(answer[-1300:])
 record('1000-byte mid-line paste/backspace/wrapped command',cycle=cycle,host_seconds=time.monotonic()-t,wire_bytes=len(answer))
 c.t.read_very_eager()
command(c,'echo RECOVERED; status runtime');command(c,'netheap2');c.close()
