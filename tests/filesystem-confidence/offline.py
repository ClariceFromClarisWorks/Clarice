"""Only run after successful native FlushVol, application quit and termination of this test's Snow PID."""
from pathlib import Path
import subprocess,json,hashlib,os
r=Path.cwd();w=r/'work/filesystem-confidence/guest';out=r/'outputs/filesystem-confidence/forks';out.mkdir(exist_ok=True)
pid=int((w/'pid').read_text())
try:os.kill(pid,0)
except ProcessLookupError:pass
else:raise SystemExit('Refusing to inspect disk images while the owned guest is running')
s='hmount /src/work/filesystem-confidence/guest/A.hda\n'
for name,path in [('source',':fork-source'),('overwrite',':confidence-final:fork-overwrite'),('cancel-target',':confidence-final:cancel-target'),('large',':confidence-final:large'),('largecopy',':confidence-final:largecopy')]:s+=f'hcopy -m "{path}" /src/outputs/filesystem-confidence/forks/{name}.bin\n'
s+='hcopy -r :flush /src/outputs/filesystem-confidence/forks/flush.clx\nhls -la :confidence-final\nhumount\nhmount /src/work/filesystem-confidence/guest/B.hda\n'
for name,path in [('moved',':confidence-final:forks'),('full-target',':confidence-final:full-target'),('recovered',':confidence-final:recovered')]:s+=f'hcopy -m "{path}" /src/outputs/filesystem-confidence/forks/{name}.bin\n'
s+='hls -la :confidence-final\nhumount\n'
im='ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c'
p=subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(r)+':/src',im,'sh','-ec',s],capture_output=True,text=True);(out/'export.log').write_text(p.stdout+p.stderr);p.check_returncode();assert 'Clv tmp' not in p.stdout

assert hashlib.sha256((out/'flush.clx').read_bytes()).hexdigest()==json.loads((r/'outputs/filesystem-confidence/flush-fixture.json').read_text())['sha256']

def read(name):
 b=(out/(name+'.bin')).read_bytes();n=int.from_bytes(b[83:87],'big');q=int.from_bytes(b[87:91],'big');off=128+((n+127)//128)*128;assert len(b)>=off+q
 return dict(data_bytes=n,resource_bytes=q,data_sha256=hashlib.sha256(b[128:128+n]).hexdigest(),resource_sha256=hashlib.sha256(b[off:off+q]).hexdigest(),type_creator=b[65:73].decode('latin1'),timestamps=b[91:99].hex())
rows={n:read(n) for n in ['source','overwrite','moved','full-target','cancel-target','large','largecopy','recovered']}
for n in ['overwrite','moved','full-target','cancel-target']:assert rows[n]==rows['source'],n
for n in ['largecopy','recovered']:
 for k in ['data_bytes','resource_bytes','data_sha256','resource_sha256','type_creator']:assert rows[n][k]==rows['large'][k],(n,k)
assert rows['large']['data_bytes']==2097163
(out/'comparison.json').write_text(json.dumps(rows,indent=2)+'\n');print('PASS persisted fork bytes, metadata, timestamps, full-disk target retention, large local/cross-volume copies; no temporary siblings')
