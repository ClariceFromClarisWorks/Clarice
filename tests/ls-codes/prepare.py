from pathlib import Path
import shutil,subprocess,re
w=Path('work/ls-codes');prefix=':System Folder:Clarice Data:tmp:'
for n in ['system','bench']:
 target=w/(n+'.hda')
 if target.exists():
  r=subprocess.run(['lsof','-F','f',str(target)],capture_output=True)
  assert r.returncode in (0,1) and not re.search(rb'^f[^\n]*[uw]$',r.stdout,re.M),'Stop disposable Snow before resetting media'
 shutil.copyfile('work/file-progress/'+n+'-tested.hda',target)
(w/'profile').write_text('export PATH=/bin:/usr/bin\nsh /tmp/lc-tests\n')
(w/'sample').write_bytes(b'sample\n')
c=['hmount /src/work/ls-codes/system.hda']
for v in ['before','after']:
 c += [f'hmkdir "{prefix}lc-{v}"',f'hcopy -r /src/work/ls-codes/build/{v}/ls "{prefix}lc-{v}:ls"']
c += [f'hmkdir "{prefix}lc-samples"',f'hmkdir "{prefix}lc-samples:folder"']
for n,t,creator in [('archive','SITx','SITx'),('command','CLX1','CLRC'),('.hidden','TEXT','ttxt')]:
 c += [f'hcopy -r /src/work/ls-codes/sample "{prefix}lc-samples:{n}"',f'hattrib -t {t} -c {creator} "{prefix}lc-samples:{n}"']
c += [f'hcopy -r /src/tests/ls-codes/guest.sh "{prefix}lc-tests"','hcopy -r /src/work/ls-codes/profile ":System Folder:Clarice Data:etc:profile"','humount']
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(Path.cwd())+':/src','ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c','sh','-ec','\n'.join(c)],check=True)
