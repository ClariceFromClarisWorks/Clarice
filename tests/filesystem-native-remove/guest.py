"""Normal 68000 Snow acceptance; disposable paths on the cloned Batch3C fixture."""
from pathlib import Path
import sys,json,time,re
sys.path.insert(0,'tests/remote-sessions');from client import Client
out=Path('outputs/filesystem-native-remove');rows=[];c=Client(port=23260,password='')
native='native:Sys6Unix:System Folder:Clarice Data:tmp:batch3d:'
def run(label,cmd,expected=0):
 rc,t=c.command(cmd,180);rows.append(dict(test=label,command=cmd,status=rc,expected=expected,text=t));(out/'guest.json').write_text(json.dumps(rows,indent=2)+'\n');print(label,rc,flush=True);assert rc==expected,t
try:
 run('setup','mkdir -p /tmp/batch3d/keep; cp -p /tmp/batch3b/source /tmp/batch3d/keep/sentinel; cd /tmp/batch3d; cp -p keep/sentinel reference')
 run('literal native trailing slash seed',"cp -p reference keep%2F; fsprobe same reference '"+native+"keep/'")
 run('remove exact native file',"rm -R '"+native+"keep/' && test ! -e keep%2F && test -d keep && fsprobe same reference keep/sentinel")
 for encoded,raw in [('mid%2Fname','mid/name'),('%2Flead','/lead'),('keep%2F%2F','keep//'),('part%2F.','part/.')]:
  run('native slash '+raw,"cp -p reference '"+encoded+"' && rm -R '"+native+raw+"' && test ! -e '"+encoded+"' && fsprobe same reference keep/sentinel")
 run('native slash directory',"mkdir folder%2F; cp -p reference folder%2F/child; rm -R '"+native+"folder/' && test ! -e folder%2F && fsprobe same reference keep/sentinel")
 run('symlink seed','ln -s keep dirlink; ln -s absent dangling')
 for operand in ['dirlink/','dirlink///','dirlink/.///','dangling/',native+'dirlink:',native+'dangling:']:
  run('reject '+operand,"rm -R '"+operand+"'",1)
  run('unchanged sibling and links','test "$(readlink dirlink)" = keep && test "$(readlink dangling)" = absent && fsprobe same reference keep/sentinel')
 run('force guard','rm -Rf dirlink/',1)
 run('remove links only','rm -R dirlink dangling && test "$(find . -type l)" = "" && fsprobe same reference keep/sentinel')
 for operand in ('ordinary///',native+'ordinary:'):
  run('ordinary directory '+operand,"mkdir -p ordinary/sub; cp -p reference ordinary/sub/file; rm -R '"+operand+"' && test ! -e ordinary && fsprobe same reference keep/sentinel")
 for operand in ('missing',native+'missing/','',native+'keep/.///'):
  run('missing literal '+operand,"rm -R '"+operand+"'",1)
  run('error recovery and untouched sentinel','fsprobe same reference keep/sentinel')
 run('no operand','rm -R',2)
 run('cancel seed','cp -R --no-progress /mnt/Perf/n100 /mnt/Perf/cancel-3d')
 c.t.write(b'rm -R --no-progress /mnt/Perf/cancel-3d\r\n');time.sleep(.15);start=time.monotonic();c.t.get_socket().sendall(bytes([255,244]));reply=c.t.read_until(b'\r\nbatch3d > ',30);assert reply.endswith(b'\r\nbatch3d > '),reply
 rows.append(dict(test='cancel response',host_ms=(time.monotonic()-start)*1000,text=reply.decode('latin1')))
 run('cancel status and recovery','test "$?" -eq 130 && status runtime && fsprobe same reference keep/sentinel')
 run('partial tree cleanup','rm -Rf --no-progress /mnt/Perf/cancel-3d; test ! -e /mnt/Perf/cancel-3d')
 for n in range(3):
  run('repeated cleanup '+str(n),"i=0; while test $i -lt 10; do cp -p reference repeat%2F || break; rm '"+native+"repeat/' || break; i=$((i+1)); done; test $i -eq 10 && fsprobe same reference keep/sentinel && status runtime")
 run('final preserved state','test -d keep && fsprobe same reference keep/sentinel && status runtime')
finally:c.close()
