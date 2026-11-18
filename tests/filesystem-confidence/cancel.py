from pathlib import Path
import sys,json,time
sys.path.insert(0,str(Path('tests/filesystem-fixes/remote').resolve()))
from client import Client
out=Path('outputs/filesystem-confidence');rows=[];c=Client(port=23283,password='')
try:
 rc,text=c.command("cd '/mnt/FS Audit A/confidence-final'; cp -p '/mnt/FS Audit A/fork-source' cancel-target",60);assert rc==0,text
 for n in range(3):
  c.t.write(b'cp large cancel-target\r\n');time.sleep(.5);c.t.get_socket().sendall(bytes([255,244]));reply=c.t.read_until(b' > ',60).decode('latin1')
  rc,text=c.command('test "$?" -eq 130',60)
  rows.append(dict(test='interrupted copy '+str(n),wire=reply,status_check=rc,text=text));(out/'cancel.json').write_text(json.dumps(rows,indent=2)+'\n');assert rc==0,(reply,text)
  rc,text=c.command("cmp '/mnt/FS Audit A/fork-source' cancel-target && status runtime",60);assert rc==0 and 'remaining=0' in text,text
  rows.append(dict(test='retained target and cleanup '+str(n),status=rc,text=text));(out/'cancel.json').write_text(json.dumps(rows,indent=2)+'\n');print('PASS interrupted copy',n,'status130, target retained, zero owned survivors',flush=True)
finally:c.close()
