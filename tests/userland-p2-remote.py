"""Loopback-only Snow acceptance; existing Telnet transport and client."""
import sys,time,json
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23259,password='');rows=[]
try:
 rc,out=c.command('cat /tmp/p2-result',60);rows.append(dict(test='local-script',status=rc,output=out));print(out,flush=True)
 for value in ['HOME=</home> tilde=</home>','override=</tmp>','restored=</home>','pipeline-alive','sub=<> status=1','assignment-status=1','at=<a>','at=<b>','at=<c>','at=<d>','empty-ifs=<prea>','empty-ifs=<b>','empty-ifs=<post>','/bin/cat','path-ok','P2-LOCAL-DONE']:
  assert value in out,value
 assert out.count('device-status=0')==5 and out.count('zero-status=0')==5 and out.count('remaining=0')==5
 for value in [256,511,-1]:
  c.t.write(("sh -c 'exit "+str(value)+"'\r\n").encode());reply=c.t.read_until(b' > ',20);assert reply.endswith(b' > '),reply
  rc,out=c.command('echo connected');assert rc==0 and 'connected\r\n' in out
  rows.append(dict(test='child-status-'+str(value),output=reply.decode('latin1')+out))
 c.t.write(b'read blocked\r\n');time.sleep(1);c.t.write(b'\x03');reply=c.t.read_until(b' > ',20)
 rc,out=c.command('echo $?');assert '\r\n130\r\n' in out and 'read failed' not in reply.decode('latin1'),(reply,out)
 rows.append(dict(test='blocked-read-cancel',output=reply.decode('latin1')+out))
 rc,out=c.command('printf "remote-home=%s\\n" "$HOME"; status runtime');assert rc==0 and 'remote-home=/home' in out and 'remaining=0' in out;rows.append(dict(test='recovery',output=out))
finally:c.close()
Path('outputs/userland-p2/guest.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS local script, native devices, remote high-status survival, blocked read cancellation and prompt recovery')
