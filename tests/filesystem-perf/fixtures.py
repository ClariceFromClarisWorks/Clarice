"""Disposable native fixture: 10/100/1000 files, split across up to10 subfolders."""
from pathlib import Path
import json,shutil
r=Path.cwd();w=r/'work/filesystem-perf';w.mkdir(exist_ok=True)
for name in ('system.hda','system.pram','B.hda'):
 shutil.copyfile(r/'work/filesystem-p2'/name,w/name)
c=json.loads((r/'work/filesystem-p2/test.snoww').read_text());c['pram_path']=str(w/'system.pram');c['scsi_targets'][0]={'Disk':str(w/'system.hda')};c['scsi_targets'][1]={'Disk':str(w/'B.hda')};(w/'test.snoww').write_text(json.dumps(c))
(w/'payload').write_bytes(bytes(range(256))*4)
lines=['set -eu','hformat -l Perf /src/work/filesystem-perf/B.hda','hmount /src/work/filesystem-perf/B.hda','hmkdir :before', 'hcopy -r /src/work/filesystem-perf/before-files-profile.clx :before:fprofile', 'hcopy -r /src/work/filesystem-perf/before-inspect-profile.clx :before:iprofile']
for command,image in [('cp','files'),('rm','files'),('find','inspect'),('du','inspect')]:lines.append(f'hcopy -r /src/work/filesystem-perf/before-{image}.clx :before:{command}')
for n in (10,100,1000):
 lines.append(f'hmkdir :n{n}')
 groups=min(10,n//5)
 for g in range(groups):
  lines.append(f'hmkdir :n{n}:d{g}')
  for i in range(n//groups):
   leaf=f'file {i:04d}' if i else 'slash/%'
   source='/src/work/file-progress/resource/valid.bin' if i==1 else '/src/work/filesystem-perf/payload'
   mode='m' if i==1 else 'r';target=f':n{n}:d{g}:{leaf}'
   lines.append(f'hcopy -{mode} {source} "{target}"')
lines+=['humount'];(w/'fixture.sh').write_text('\n'.join(lines)+'\n')
