"""Patch's actual HFS fork/metadata, rollback and cancellation checks in Snow."""
from pathlib import Path
import sys,json,time,re
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/remote-sessions'));from client import Client
c=Client(port=23245,password=(r.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip());rows=[]
def run(cmd,expected=0,timeout=90):
 t=time.monotonic();rc,text=c.command(cmd,timeout);rows.append(dict(command=cmd,status=rc,text=text,host_seconds=time.monotonic()-t));(r/'outputs/busybox-additions/safety.json').write_text(json.dumps(rows,indent=2)+'\n');assert rc==expected,(cmd,rc,text);print('PASS',cmd,flush=True);return text
try:
 run('cp native:GlyphFull:Clarice /tmp/ba-forks')
 run("printf 'alpha\\nbeta\\ngamma\\n' > /tmp/ba-forks; hfsattr --type TEXT --creator TEST /tmp/ba-forks")
 run('cp /tmp/ba-forks /tmp/ba-resource-baseline')
 before=run('forksum /tmp/ba-forks');fingerprint=re.search(r'resource (\d+) ([0-9a-f]+)',before).groups();assert int(fingerprint[0])>0
 run('patch -i /tmp/add-diff /tmp/ba-forks')
 run('cmp /tmp/ba-forks /tmp/add-new')
 run('forksum /tmp/ba-resource-baseline /tmp/ba-forks')
 after=run('forksum /tmp/ba-forks');assert re.search(r'resource (\d+) ([0-9a-f]+)',after).groups()==fingerprint
 info=run('file /tmp/ba-forks');assert "type 'TEXT', creator 'TEST'" in info
 run('patch -R -i /tmp/add-diff /tmp/ba-forks')
 run('cmp /tmp/ba-forks /tmp/add-old')
 # Keep a resource-bearing disposable target, enlarge only its data fork.
 run('cat '+ ' '.join(['/tmp/ba-text8k']*32)+' > /tmp/ba-forks')
 run('cp /tmp/ba-forks /tmp/ba-forks-before')
 run("printf '--- a\\n+++ b\\n@@ -1,1 +1,1 @@\\n-NO_MATCH_IN_TARGET\\n+replacement\\n' > /tmp/ba-cancel.patch")
 c.t.write(b'patch -i /tmp/ba-cancel.patch /tmp/ba-forks\r\n');time.sleep(2);before_cancel=c.t.read_very_eager();assert not before_cancel.endswith(b' > '),before_cancel;c.t.write(b'\x03');reply=before_cancel+c.t.read_until(b' > ',60);assert reply.endswith(b' > ')
 text=run("printf 'CANCEL_STATUS=%s\\n' $?");assert 'CANCEL_STATUS=130\r\n' in text
 stats=run('status runtime');assert 'remaining=0' in stats
 run('cmp /tmp/ba-forks /tmp/ba-forks-before')
 run('forksum /tmp/ba-resource-baseline /tmp/ba-forks')
 after=run('forksum /tmp/ba-forks');assert re.search(r'resource (\d+) ([0-9a-f]+)',after).groups()==fingerprint
 # A subsequent successful save proves no abandoned replacement token remains.
 run("printf 'alpha\\nbeta\\ngamma\\n' > /tmp/ba-forks")
 run('patch -i /tmp/add-diff /tmp/ba-forks')
 run('cmp /tmp/ba-forks /tmp/add-new')
 stats=run('status runtime');assert 'remaining=0' in stats
finally:c.close()
print('PASS resource contents, Finder metadata, cancellation rollback and next-save recovery')
