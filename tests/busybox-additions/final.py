"""Final tar-format rejection and installed-catalog verification in isolated Snow."""
from pathlib import Path
import sys,json,time
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/remote-sessions'));from client import Client
c=Client(port=23245,password=(r.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip());rows=[]
def run(cmd,expected=0):
 t=time.monotonic();rc,text=c.command(cmd,120);rows.append(dict(command=cmd,status=rc,text=text,host_seconds=time.monotonic()-t));(r/'outputs/busybox-additions/final.json').write_text(json.dumps(rows,indent=2)+'\n');assert rc==expected,(cmd,rc,text);print('PASS',cmd,flush=True);return text
try:
 for name in ['pax','gnu']:
  run(f'wget -O /tmp/ba-{name}.tar http://192.168.1.112:8099/{name}.tar')
  run(f'tar -tf /tmp/ba-{name}.tar',1)
 run('tar -tf /tmp/ba-host.tar')
 run('tar -xOf /tmp/ba-host.tar binary8k > /tmp/ba-tar-final')
 run('cmp /tmp/ba-tar-final /tmp/ba-binary8k')
 run('tar -xvOf /tmp/ba-host.tar binary8k',1)
 installed=run('cat /etc/clarice/installed.catalog')
 expected=(r/'work/additions-snow/http/index.txt').read_text().splitlines()[1:]
 checks=[]
 latest={}
 for line in installed.splitlines():
  f=line.split('\t')
  if len(f)==5 and f[0] in ('tool','core'):latest[(f[0],f[1])]=line
 for line in expected:
  fields=line.split('\t');needle='\t'.join(fields[:5]);ok=latest.get((fields[0],fields[1]))==needle
  checks.append(dict(name=fields[1],expected=needle,matched=ok))
 (r/'outputs/busybox-additions/installed-catalog-check.json').write_text(json.dumps(checks,indent=2)+'\n')
 assert all(x['matched'] for x in checks),[x for x in checks if not x['matched']]
finally:c.close()
print('PASS final tar and installed catalog')
