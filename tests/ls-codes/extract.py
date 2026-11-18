from pathlib import Path
import subprocess,re
w=Path('work/ls-codes');out=Path('outputs/ls-codes')
r=subprocess.run(['lsof','-F','f',str(w/'system.hda')],capture_output=True)
assert r.returncode in (0,1) and not re.search(rb'^f[^\n]*[uw]$',r.stdout,re.M),'Stop disposable Snow before extraction'
p=':System Folder:Clarice Data:tmp:'
names=['results','out-100-before','out-100-after','out-1000-before','out-1000-after','legend','full','bad','error']
c=['hmount /src/work/ls-codes/system.hda']+[f'hcopy -r "{p}lc-{n}" /src/outputs/ls-codes/{n}.txt' for n in names]+['humount']
r=subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(Path.cwd())+':/src','ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c','sh','-ec','\n'.join(c)],capture_output=True,text=True)
(out/'extraction.txt').write_text(r.stdout+r.stderr);r.check_returncode()
