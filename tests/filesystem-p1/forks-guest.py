"""Real Resource Manager fixture from tests/file-progress/resource/fixture.py."""
from pathlib import Path
import sys,json,time
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23259,password='');rows=[]
def run(s,expected=0):
 rc,text=c.command(s,90);rows.append(dict(command=s,status=rc,text=text));print(rc,s,flush=True);assert rc==expected,text
try:
 run('cd /tmp/batch3a/native; cp -p source "percent%25slash%2Fname"; cp -p source moved; mv moved renamed; cmp source renamed')
 run('ln -s target dirlink')
 run('rm -R dirlink/',1)
 run('cmp source target/sub/sentinel; test "$(readlink dirlink)" = target')
 run('cp source missing/parent/file',1)
 run('dd if=source of=large bs=1024 seek=2048 count=1')
 c.t.write(b'cp large cancel-target\r\n');time.sleep(.2);c.t.get_socket().sendall(bytes([255,244]));reply=c.t.read_until(b' > ',60);assert reply.endswith(b' > ')
 run('test "$?" -eq 130 && cmp source cancel-target && status runtime')
 run('rm large; status runtime')
finally:c.close();Path('outputs/filesystem-p1/forks-guest.json').write_text(json.dumps(rows,indent=2)+'\n')
