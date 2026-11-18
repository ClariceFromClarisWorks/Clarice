"""Changed-text diff/patch cost, low-memory failure and recovery, measured in Snow."""
from pathlib import Path
import sys,json,time,re,gzip,base64
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/remote-sessions'));from client import Client
c=Client(port=23245,password=(r.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip());rows=[]
def run(cmd,expected=0,timeout=180):
 t=time.monotonic();rc,text=c.command(cmd,timeout);row=dict(command=cmd,status=rc,text=text,host_seconds=time.monotonic()-t);rows.append(row);(r/'outputs/busybox-additions/diff-cost.json').write_text(json.dumps(rows,indent=2)+'\n');assert (rc in expected if isinstance(expected,tuple) else rc==expected),(cmd,rc,text);print('PASS',cmd,flush=True);return row
try:
 for name in ['lines8k','edited8k','lines256k','edited256k']:run(f'wget -O /tmp/ba-{name} http://192.168.1.112:8099/{name}')
 for repeat in range(3):
  row=run('diff /tmp/ba-lines8k /tmp/ba-edited8k > /tmp/ba-text.diff',1);row['repeat']=repeat;row['stats']=run('status runtime')['text']
 run('cp /tmp/ba-lines8k /tmp/ba-target')
 for repeat in range(3):
  row=run('patch -i /tmp/ba-text.diff /tmp/ba-target');row['repeat']=repeat;row['stats']=run('status runtime')['text'];run('cmp /tmp/ba-target /tmp/ba-edited8k');run('patch -R -i /tmp/ba-text.diff /tmp/ba-target');run('cmp /tmp/ba-target /tmp/ba-lines8k')
 pressure=run('diff /tmp/ba-lines256k /tmp/ba-edited256k > /tmp/ba-pressure.diff',(1,2),240)
 if pressure['status']==2:assert 'out of memory' in pressure['text']
 stats=run('status runtime')['text'];assert 'remaining=0' in stats
 run('diff /tmp/ba-lines8k /tmp/ba-edited8k > /tmp/ba-text.diff',1)
 # Explicitly reject NUL-bearing target lines and malformed hunk headers.
 run("printf 'alpha\\000hidden\\nbeta\\ngamma\\n' > /tmp/ba-binary-target; cp /tmp/ba-binary-target /tmp/ba-before")
 run('patch -i /tmp/add-diff /tmp/ba-binary-target',1)
 run('cmp /tmp/ba-binary-target /tmp/ba-before')
 run("printf '--- a\\n+++ b\\n@@ -\\n' > /tmp/ba-malformed")
 run('patch -i /tmp/ba-malformed /tmp/ba-target',1)
 run('cmp /tmp/ba-target /tmp/ba-lines8k')
 # The guest compressor output must also decode with the host implementation.
 run('gzip -c /tmp/ba-binary8k > /tmp/ba-interop.gz')
 encoded=run('base64 /tmp/ba-interop.gz')['text'].split('\r\n',1)[1].split('\r\nREMOTE_END_',1)[0]
 raw=base64.b64decode(''.join(encoded.split()));assert gzip.decompress(raw)==bytes(range(256))*32
 run('cat /etc/clarice/installed.catalog')
finally:c.close()
(r/'outputs/busybox-additions/diff-cost.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS changed-text costs, bounded failure/recovery, safe rejects, host gzip interoperability')
