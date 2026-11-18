"""Same-fixture native resize semantics, diff workload and cleanup checks."""
from pathlib import Path
import sys,json,time,re,base64,tempfile,subprocess
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/remote-sessions'));from client import Client
c=Client(port=23245,password=(r.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip());rows=[]
def run(cmd,expected=0,timeout=300):
 t=time.monotonic();rc,text=c.command(cmd,timeout);row=dict(command=cmd,status=rc,text=text,host_seconds=time.monotonic()-t);rows.append(row);(r/'outputs/realloc/compare.json').write_text(json.dumps(rows,indent=2)+'\n');assert (rc in expected if isinstance(expected,tuple) else rc==expected),(cmd,rc,text);print('PASS',cmd,flush=True);return row
def stats():
 x=run('status runtime');assert 'remaining=0' in x['text'];return x
try:
 assert '\r\nRESULT=0\r\n' in (r/'outputs/realloc/capacity-probe.txt').read_bytes().decode('latin1')
 stats()
 big=run('diffgrow /tmp/ba-lines256k /tmp/ba-edited256k > /tmp/ra-grow.diff',(1,2),600);stats()
 if big['status']==1:
  text=run('base64 /tmp/ra-grow.diff')['text'];encoded=text.split('\r\n',1)[1].split('\r\nREMOTE_END_',1)[0];patch=base64.b64decode(''.join(encoded.split()))
  (r/'outputs/realloc/large.diff').write_bytes(patch)
  with tempfile.TemporaryDirectory() as td:
   target=Path(td)/'target';target.write_bytes((r/'work/additions-snow/http/lines256k').read_bytes());pf=Path(td)/'change.diff';pf.write_bytes(patch)
   subprocess.run(['/usr/bin/patch','-s',str(target),str(pf)],check=True,capture_output=True)
   assert target.read_bytes()==(r/'work/additions-snow/http/edited256k').read_bytes()
  print('PASS host patch reproduces exact edited 256 KiB input',flush=True)
 for repeat in range(3):
  for name in ['difftrace','diffgrow']:
   run(f'{name} /tmp/ba-lines8k /tmp/ba-edited8k > /tmp/ra-{name}.diff',1);stats()
  run('cmp /tmp/ra-difftrace.diff /tmp/ra-diffgrow.diff')
 run('diffgrow /tmp/add-old /tmp/add-old');stats()
 run('diffgrow /tmp/add-old /tmp/ra-nonexistent',2);stats()
 c.t.write(b'alloctest cancel\r\n');time.sleep(2);pre=c.t.read_very_eager();assert not pre.endswith(b' > ');c.t.write(b'\x03');reply=pre+c.t.read_until(b' > ',60);assert reply.endswith(b' > ')
 status=run("printf 'CANCEL_STATUS=%s\\n' $?")['text'];assert 'CANCEL_STATUS=130\r\n' in status;stats()
 run('alloctest');stats()
finally:c.close()
print('PASS allocator semantics, diff comparison, cancellation and subsequent invocation recovery')
