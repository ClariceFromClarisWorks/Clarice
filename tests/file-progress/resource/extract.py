"""Run only after the disposable Snow process has exited."""
from pathlib import Path
import subprocess,re
w=Path('work/file-progress/resource');out=Path('outputs/file-progress/resource');out.mkdir(exist_ok=True,parents=True)
for n in ['system','bench']:
 r=subprocess.run(['lsof','-F','f',str(w/(n+'.hda'))],capture_output=True)
 assert r.returncode in (0,1) and not re.search(rb'^f[^\n]*[uw]$',r.stdout,re.M),'disk is still open'
p=':System Folder:Clarice Data:tmp:';c=['hmount /src/work/file-progress/resource/system.hda',f'hcopy -r "{p}res-results" /src/outputs/file-progress/resource/guest.txt']
for n in ['source','back-before','back-after']:c.append(f'hcopy -m "{p}res-{n}" /src/work/file-progress/resource/{n}.bin')
c+=['humount','hmount /src/work/file-progress/resource/bench.hda']
for n in ['before','after']:c.append(f'hcopy -m :res-{n} /src/work/file-progress/resource/{n}.bin')
c+=['humount']
r=subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(Path.cwd())+':/src','ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c','sh','-ec','\n'.join(c)],capture_output=True,text=True)
(out/'extraction.txt').write_text(r.stdout+r.stderr);r.check_returncode()
