from pathlib import Path
import shutil,subprocess,re
w=Path('work/file-progress/resource');prefix=':System Folder:Clarice Data:tmp:'
for n in ['system','bench']:
 target=w/(n+'.hda')
 if target.exists():
  r=subprocess.run(['lsof','-F','f',str(target)],capture_output=True)
  assert r.returncode in (0,1) and not re.search(rb'^f[^\n]*[uw]$',r.stdout,re.M),'Stop disposable Snow before resetting test media'
for n in ['system','bench']:shutil.copyfile('work/file-progress/'+n+'-tested.hda',w/(n+'.hda'))
(w/'profile').write_text('export PATH=/bin:/usr/bin\nsh /tmp/res-tests\n')
c=['hmount /src/work/file-progress/resource/system.hda',f'hcopy -m /src/work/file-progress/resource/valid.bin "{prefix}res-source"',f'hcopy -r /src/work/file-progress/resource/rescheck "{prefix}rescheck"',f'hcopy -r /src/tests/file-progress/resource/guest.sh "{prefix}res-tests"','hcopy -r /src/work/file-progress/resource/profile ":System Folder:Clarice Data:etc:profile"','humount']
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(Path.cwd())+':/src','ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c','sh','-ec','\n'.join(c)],check=True)
